// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "CommandList11.h"

#include "ColorBuffer.h"
#include "CommandListManager11.h"
#include "ConstantBuffer11.h"
#include "DepthBuffer.h"
#include "DeviceManager11.h"
#include "GpuBuffer11.h"
#include "IndexBuffer11.h"
#include "PipelineState11.h"
#include "Rectangle.h"
#include "RenderEnums11.h"
#include "RenderUtils.h"
#include "VertexBuffer11.h"
#include "Viewport.h"

#include <locale>
#include <codecvt>

using namespace Kodiak;
using namespace Microsoft::WRL;
using namespace std;


vector<unique_ptr<CommandList>>	CommandList::s_commandListPool;
mutex CommandList::s_commandListAllocationMutex;
std::queue<CommandList*> CommandList::s_availableCommandLists;


CommandList::CommandList()
{
	m_currentBlendFactor = DirectX::Colors::White;
}


CommandList::~CommandList()
{
	if (nullptr != m_context)
	{
		m_context->Release();
	}
}


void CommandList::DestroyAllCommandLists()
{
	s_commandListPool.clear();
}


void CommandList::CopyCounter(GpuBuffer& dest, size_t destOffset, StructuredBuffer& src)
{
	m_context->CopyStructureCount(dest.GetBuffer(), static_cast<UINT>(destOffset), src.GetUAV());
}


void CommandList::InitializeTextureArraySlice(GpuResource& dest, UINT sliceIndex, GpuResource& src)
{
	auto& commandList = CommandList::Begin();

	auto destResource = dest.GetResource();
	auto srcResource = src.GetResource();

	D3D11_RESOURCE_DIMENSION destType, srcType;
	destResource->GetType(&destType);
	srcResource->GetType(&srcType);

	// TODO: Only handling 2d textures for now
	assert(destType == D3D11_RESOURCE_DIMENSION_TEXTURE2D);
	assert(destType == srcType);

	ID3D11Texture2D* destTexture = nullptr;
	ID3D11Texture2D* srcTexture = nullptr;

	ThrowIfFailed(destResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&destTexture)));
	ThrowIfFailed(srcResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&srcTexture)));

	D3D11_TEXTURE2D_DESC destDesc, srcDesc;
	destTexture->GetDesc(&destDesc);
	srcTexture->GetDesc(&srcDesc);

	assert(sliceIndex < destDesc.ArraySize &&
		srcDesc.ArraySize == 1 &&
		destDesc.Width == srcDesc.Width &&
		destDesc.Height == srcDesc.Height &&
		destDesc.MipLevels <= srcDesc.MipLevels
	);

	UINT subresourceIndex = sliceIndex * destDesc.MipLevels;

	for (UINT i = 0; i < destDesc.MipLevels; ++i)
	{
		commandList.m_context->CopySubresourceRegion(
			destResource,
			subresourceIndex + i,
			0,
			0,
			0,
			srcResource,
			i,
			nullptr);
	}

	commandList.CloseAndExecute(true);
}


void CommandList::WriteBuffer(GpuResource& dest, size_t destOffset, const void* data, size_t numBytes)
{
	ComPtr<ID3D11Buffer> staging;

	D3D11_BUFFER_DESC desc{};
	
	desc.ByteWidth = static_cast<UINT>(numBytes);
	desc.BindFlags = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = data;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	DeviceManager::GetInstance().GetDevice()->CreateBuffer(&desc, &initData, &staging);

	m_context->CopySubresourceRegion(dest.GetResource(), 0, static_cast<UINT>(destOffset), 0, 0, staging.Get(), 0, nullptr);
}


void CommandList::FillBuffer(GpuResource& dest, size_t destOffset, DWParam value, size_t numBytes)
{
	ComPtr<ID3D11Buffer> staging;

	D3D11_BUFFER_DESC desc{};
	
	desc.ByteWidth = 4;
	desc.BindFlags = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = &value;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	DeviceManager::GetInstance().GetDevice()->CreateBuffer(&desc, &initData, &staging);

	m_context->CopySubresourceRegion(dest.GetResource(), 0, static_cast<UINT>(destOffset), 0, 0, staging.Get(), 0, nullptr);
}


void CommandList::ResetCounter(StructuredBuffer& buf, uint32_t value)
{
	buf.SetCounterInitialValue(value);
}


CommandList& CommandList::Begin()
{
	CommandList* newCommandList = CommandList::AllocateCommandList();
	return *newCommandList;
}


uint64_t CommandList::CloseAndExecute(bool waitForCompletion)
{
	assert(m_pixMarkerCount == 0);
	ID3D11CommandList* commandList = nullptr;
	ThrowIfFailed(m_context->FinishCommandList(TRUE, &commandList));

	// ExecuteCommandList calls Release on the commandList pointer
	m_owner->ExecuteCommandList(commandList);

	FreeCommandList(this);
	return 0;
}


void CommandList::Initialize(CommandListManager& manager)
{
	m_owner = &manager;
	m_owner->CreateNewDeferredContext(&m_context);
	ThrowIfFailed(m_context->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&m_context1)));
	ThrowIfFailed(m_context->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), reinterpret_cast<void**>(&m_annotation)));
}


void CommandList::PIXBeginEvent(const string& label)
{
#if defined(RELEASE)
	(label)
#else
	wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
	wstring wide = converter.from_bytes(label);

	m_annotation->BeginEvent(wide.c_str());
	++m_pixMarkerCount;
#endif
}


void CommandList::PIXEndEvent()
{
#if !defined(RELEASE)
	m_annotation->EndEvent();
	--m_pixMarkerCount;
#endif
}


void CommandList::PIXSetMarker(const string& label)
{
#if defined(RELEASE)
	(label)
#else
	wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
	wstring wide = converter.from_bytes(label);

	m_annotation->SetMarker(wide.c_str());
#endif
}


CommandList* CommandList::AllocateCommandList()
{
	lock_guard<mutex> lockGuard(s_commandListAllocationMutex);

	CommandList* commandList = nullptr;
	if (s_availableCommandLists.empty())
	{
		commandList = new CommandList;
		s_commandListPool.emplace_back(commandList);
		commandList->Initialize(CommandListManager::GetInstance());
	}
	else
	{
		commandList = s_availableCommandLists.front();
		s_availableCommandLists.pop();
		commandList->Reset();
	}

	assert(nullptr != commandList);

	return commandList;
}


void CommandList::FreeCommandList(CommandList* commandList)
{
	lock_guard<mutex> lockGuard(s_commandListAllocationMutex);

	s_availableCommandLists.push(commandList);
}


void CommandList::Reset()
{
	m_currentStencilRef = 0;
	m_currentBlendFactor = DirectX::Colors::White;
	m_currentBlendState = nullptr;
	m_currentDepthStencilState = nullptr;
	m_currentGraphicsPSO = nullptr;
}


void GraphicsCommandList::ClearUAV(ColorBuffer& target)
{
	auto uav = target.GetUAV();
	if (uav)
	{
		m_context->ClearUnorderedAccessViewFloat(uav, target.GetClearColor());
	}
}


void GraphicsCommandList::ClearUAV(ColorBuffer& target, const DirectX::XMVECTORF32& clearColor)
{
	auto uav = target.GetUAV();
	if (uav)
	{
		m_context->ClearUnorderedAccessViewFloat(uav, clearColor);
	}
}


void GraphicsCommandList::ClearUAV(ColorBuffer& target, const DirectX::XMVECTORU32& clearValue)
{
	auto uav = target.GetUAV();
	if (uav)
	{
		m_context->ClearUnorderedAccessViewUint(uav, clearValue.u);
	}
}


void GraphicsCommandList::ClearColor(ColorBuffer& target)
{
	m_context->ClearRenderTargetView(target.GetRTV(), target.GetClearColor());
}


void GraphicsCommandList::ClearColor(ColorBuffer& target, const DirectX::XMVECTORF32& clearColor)
{
	m_context->ClearRenderTargetView(target.GetRTV(), clearColor);
}


void GraphicsCommandList::ClearDepth(DepthBuffer& target)
{
	m_context->ClearDepthStencilView(target.GetDSV(), D3D11_CLEAR_DEPTH, target.GetClearDepth(), 0);
}


void GraphicsCommandList::ClearDepth(DepthBuffer& target, float clearDepth)
{
	m_context->ClearDepthStencilView(target.GetDSV(), D3D11_CLEAR_DEPTH, clearDepth, 0);
}


void GraphicsCommandList::ClearStencil(DepthBuffer& target)
{
	m_context->ClearDepthStencilView(target.GetDSV(), D3D11_CLEAR_STENCIL, 0.0f, target.GetClearStencil());
}


void GraphicsCommandList::ClearStencil(DepthBuffer& target, uint32_t clearStencil)
{
	m_context->ClearDepthStencilView(target.GetDSV(), D3D11_CLEAR_STENCIL, 0.0f, clearStencil);
}


void GraphicsCommandList::ClearDepthAndStencil(DepthBuffer& target)
{
	m_context->ClearDepthStencilView(
		target.GetDSV(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		target.GetClearDepth(),
		target.GetClearStencil());
}


void GraphicsCommandList::ClearDepthAndStencil(DepthBuffer& target, float clearDepth, uint32_t clearStencil)
{
	m_context->ClearDepthStencilView(
		target.GetDSV(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		clearDepth,
		clearStencil);
}


void GraphicsCommandList::SetRenderTargets(uint32_t numRTVs, ColorBuffer* rtvs, DepthBuffer* dsv, bool readOnlyDepth)
{
	ID3D11RenderTargetView* d3dRTVs[8];
	for (uint32_t i = 0; i < numRTVs; ++i)
	{
		d3dRTVs[i] = rtvs[i].GetRTV();
	}

	if (dsv)
	{
		if (readOnlyDepth)
		{
			m_context->OMSetRenderTargets(numRTVs, d3dRTVs, dsv->GetDSVReadOnly());
		}
		else
		{
			m_context->OMSetRenderTargets(numRTVs, d3dRTVs, dsv->GetDSV());
		}
	}
	else
	{
		m_context->OMSetRenderTargets(numRTVs, d3dRTVs, nullptr);
	}
}


void GraphicsCommandList::UnbindRenderTargets()
{
	static ID3D11RenderTargetView* d3dRTVs[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	m_context->OMSetRenderTargets(8, d3dRTVs, nullptr);
}


void GraphicsCommandList::SetViewport(const Viewport& vp)
{
	D3D11_VIEWPORT d3dVP = { vp.topLeftX, vp.topLeftY, vp.width, vp.height, vp.minDepth, vp.maxDepth };
	m_context->RSSetViewports(1, &d3dVP);
}


void GraphicsCommandList::SetViewport(float x, float y, float w, float h, float minDepth, float maxDepth)
{
	D3D11_VIEWPORT d3dVP = { x, y, w, h, minDepth, maxDepth };
	m_context->RSSetViewports(1, &d3dVP);
}


void GraphicsCommandList::SetScissor(const Kodiak::Rectangle& rect)
{
	assert(rect.left < rect.right && rect.top < rect.bottom);
	D3D11_RECT d3dRect = { (LONG)rect.left, (LONG)rect.top, (LONG)rect.right, (LONG)rect.bottom };
	m_context->RSSetScissorRects(1, &d3dRect);
}


void GraphicsCommandList::SetScissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom)
{
	assert(left < right && top < bottom);
	D3D11_RECT d3dRect = { (LONG)left, (LONG)top, (LONG)right, (LONG)bottom };
	m_context->RSSetScissorRects(1, &d3dRect);
}


void GraphicsCommandList::SetViewportAndScissor(const Viewport& vp, const Kodiak::Rectangle& rect)
{
	assert(rect.left < rect.right && rect.top < rect.bottom);
	D3D11_VIEWPORT d3dVP = { vp.topLeftX, vp.topLeftY, vp.width, vp.height, vp.minDepth, vp.maxDepth };
	D3D11_RECT d3dRect = { (LONG)rect.left, (LONG)rect.top, (LONG)rect.right, (LONG)rect.bottom };
	m_context->RSSetViewports(1, &d3dVP);
	m_context->RSSetScissorRects(1, &d3dRect);
}


void GraphicsCommandList::SetViewportAndScissor(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
	D3D11_VIEWPORT d3dVP = { (FLOAT)x, (FLOAT)y, (FLOAT)w, (FLOAT)h, 0.0f, 1.0f };
	D3D11_RECT d3dRect = { (LONG)x, (LONG)y, (LONG)(x + w), (LONG)(y + h) };
	m_context->RSSetViewports(1, &d3dVP);
	m_context->RSSetScissorRects(1, &d3dRect);
}


void GraphicsCommandList::SetStencilRef(uint32_t stencilRef)
{
	if (m_currentStencilRef != stencilRef)
	{
		m_currentStencilRef = stencilRef;
		m_context->OMSetDepthStencilState(m_currentDepthStencilState.Get(), m_currentStencilRef);
	}
}


void GraphicsCommandList::SetBlendFactor(const DirectX::XMVECTORF32& blendFactor)
{
	if (m_currentBlendFactor != blendFactor)
	{
		m_currentBlendFactor = blendFactor;
		m_context->OMSetBlendState(m_currentBlendState.Get(), m_currentBlendFactor, 0xffffffff);
	}
}


void GraphicsCommandList::SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY topology)
{
	m_context->IASetPrimitiveTopology(topology);
}


void GraphicsCommandList::SetPipelineState(GraphicsPSO& PSO)
{
	if (&PSO != m_currentGraphicsPSO)
	{
		m_currentGraphicsPSO = &PSO;

		m_currentBlendState = PSO.m_blendState;
		m_currentDepthStencilState = PSO.m_depthStencilState;

		m_context->OMSetBlendState(m_currentBlendState.Get(), m_currentBlendFactor, 0xffffffff);
		m_context->OMSetDepthStencilState(m_currentDepthStencilState.Get(), m_currentStencilRef);
		m_context->RSSetState(PSO.m_rasterizerState);

		m_context->IASetInputLayout(PSO.m_inputLayout.Get());

		m_context->VSSetShader(PSO.m_vertexShader.Get(), nullptr, 0);
		m_context->HSSetShader(PSO.m_hullShader.Get(), nullptr, 0);
		m_context->DSSetShader(PSO.m_domainShader.Get(), nullptr, 0);
		m_context->GSSetShader(PSO.m_geometryShader.Get(), nullptr, 0);
		m_context->PSSetShader(PSO.m_pixelShader.Get(), nullptr, 0);
	}
}


byte* GraphicsCommandList::MapConstants(const ConstantBuffer& cbuffer)
{
	D3D11_MAPPED_SUBRESOURCE subresource;
	ThrowIfFailed(m_context->Map(cbuffer.constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource));

	return reinterpret_cast<byte*>(subresource.pData);
}


void GraphicsCommandList::UnmapConstants(const ConstantBuffer& cbuffer)
{
	m_context->Unmap(cbuffer.constantBuffer.Get(), 0);
}


void GraphicsCommandList::SetIndexBuffer(const IndexBuffer& ibuffer, uint32_t offset)
{
	m_context->IASetIndexBuffer(ibuffer.indexBuffer.Get(), ibuffer.format, offset);
}


void GraphicsCommandList::SetVertexBuffers(uint32_t numVBs, uint32_t startSlot, const VertexBuffer* vertexBuffers, uint32_t* offsets)
{
	ID3D11Buffer* d3dBuffers[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	UINT strides[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	UINT _offsets[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];

	for (uint32_t i = 0; i < numVBs; ++i)
	{
		d3dBuffers[i] = vertexBuffers[i].vertexBuffer.Get();
		strides[i] = vertexBuffers[i].stride;
		_offsets[i] = offsets != nullptr ? offsets[i] : 0;
	}

	m_context->IASetVertexBuffers(startSlot, numVBs, d3dBuffers, strides, _offsets);
}


void GraphicsCommandList::DrawIndirect(GpuBuffer& argumentBuffer, size_t argumentBufferOffset)
{
	m_context->DrawInstancedIndirect(argumentBuffer.GetBuffer(), static_cast<UINT>(argumentBufferOffset));
}


void GraphicsCommandList::SetVertexShaderConstants(uint32_t slot, const ConstantBuffer& cbuffer)
{
	ID3D11Buffer* d3dBuffer = cbuffer.constantBuffer.Get();
	m_context->VSSetConstantBuffers(slot, 1, &d3dBuffer);
}


void GraphicsCommandList::SetVertexShaderConstants(uint32_t startSlot, uint32_t numBuffers, ID3D11Buffer* const * cbuffers, 
	const uint32_t* firstConstant, const uint32_t* numConstants)
{
	assert(m_context1);
	m_context1->VSSetConstantBuffers1(startSlot, numBuffers, cbuffers, firstConstant, numConstants);
}


void GraphicsCommandList::SetHullShaderConstants(uint32_t slot, const ConstantBuffer& cbuffer)
{
	ID3D11Buffer* d3dBuffer = cbuffer.constantBuffer.Get();
	m_context->HSSetConstantBuffers(slot, 1, &d3dBuffer);
}


void GraphicsCommandList::SetHullShaderConstants(uint32_t startSlot, uint32_t numBuffers, ID3D11Buffer* const * cbuffers,
	const uint32_t* firstConstant, const uint32_t* numConstants)
{
	assert(m_context1);
	m_context1->HSSetConstantBuffers1(startSlot, numBuffers, cbuffers, firstConstant, numConstants);
}


void GraphicsCommandList::SetDomainShaderConstants(uint32_t slot, const ConstantBuffer& cbuffer)
{
	ID3D11Buffer* d3dBuffer = cbuffer.constantBuffer.Get();
	m_context->DSSetConstantBuffers(slot, 1, &d3dBuffer);
}


void GraphicsCommandList::SetDomainShaderConstants(uint32_t startSlot, uint32_t numBuffers, ID3D11Buffer* const * cbuffers,
	const uint32_t* firstConstant, const uint32_t* numConstants)
{
	assert(m_context1);
	m_context1->DSSetConstantBuffers1(startSlot, numBuffers, cbuffers, firstConstant, numConstants);
}


void GraphicsCommandList::SetGeometryShaderConstants(uint32_t slot, const ConstantBuffer& cbuffer)
{
	ID3D11Buffer* d3dBuffer = cbuffer.constantBuffer.Get();
	m_context->GSSetConstantBuffers(slot, 1, &d3dBuffer);
}


void GraphicsCommandList::SetGeometryShaderConstants(uint32_t startSlot, uint32_t numBuffers, ID3D11Buffer* const * cbuffers,
	const uint32_t* firstConstant, const uint32_t* numConstants)
{
	assert(m_context1);
	m_context1->GSSetConstantBuffers1(startSlot, numBuffers, cbuffers, firstConstant, numConstants);
}


void GraphicsCommandList::SetPixelShaderConstants(uint32_t slot, const ConstantBuffer& cbuffer)
{
	ID3D11Buffer* d3dBuffer = cbuffer.constantBuffer.Get();
	m_context->PSSetConstantBuffers(slot, 1, &d3dBuffer);
}


void GraphicsCommandList::SetPixelShaderConstants(uint32_t startSlot, uint32_t numBuffers, ID3D11Buffer* const * cbuffers,
	const uint32_t* firstConstant, const uint32_t* numConstants)
{
	assert(m_context1);
	m_context1->PSSetConstantBuffers1(startSlot, numBuffers, cbuffers, firstConstant, numConstants);
}


void ComputeCommandList::ClearUAV(ColorBuffer& target)
{
	auto uav = target.GetUAV();
	if (uav)
	{
		m_context->ClearUnorderedAccessViewFloat(uav, target.GetClearColor());
	}
}


void ComputeCommandList::ClearUAV(ColorBuffer& target, const DirectX::XMVECTORF32& clearColor)
{
	auto uav = target.GetUAV();
	if (uav)
	{
		m_context->ClearUnorderedAccessViewFloat(uav, clearColor);
	}
}


void ComputeCommandList::ClearUAV(ColorBuffer& target, const DirectX::XMVECTORU32& clearValue)
{
	auto uav = target.GetUAV();
	if (uav)
	{
		m_context->ClearUnorderedAccessViewUint(uav, clearValue.u);
	}
}


void ComputeCommandList::ClearUAV(GpuBuffer& target)
{
	auto uav = target.GetUAV();
	if (uav)
	{
		UINT values[4] = { 0, 0, 0, 0 };
		m_context->ClearUnorderedAccessViewUint(uav, values);
	}
}


void ComputeCommandList::SetPipelineState(ComputePSO& pso)
{
	m_context->CSSetShader(pso.m_computeShader.Get(), nullptr, 0);
}


void ComputeCommandList::DispatchIndirect(GpuBuffer& argumentBuffer, size_t argumentBufferOffset)
{

	m_context->DispatchIndirect(argumentBuffer.GetBuffer(), static_cast<UINT>(argumentBufferOffset));
}


byte* ComputeCommandList::MapConstants(const ConstantBuffer& cbuffer)
{
	D3D11_MAPPED_SUBRESOURCE subresource;
	ThrowIfFailed(m_context->Map(cbuffer.constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource));

	return reinterpret_cast<byte*>(subresource.pData);
}


void ComputeCommandList::UnmapConstants(const ConstantBuffer& cbuffer)
{
	m_context->Unmap(cbuffer.constantBuffer.Get(), 0);
}


void ComputeCommandList::SetShaderConstants(uint32_t slot, const ConstantBuffer& cbuffer)
{
	ID3D11Buffer* d3dBuffer = cbuffer.constantBuffer.Get();
	m_context->CSSetConstantBuffers(slot, 1, &d3dBuffer);
}


void ComputeCommandList::SetShaderConstants(uint32_t startSlot, uint32_t numBuffers, ID3D11Buffer* const * cbuffers,
	const uint32_t* firstConstant, const uint32_t* numConstants)
{
	assert(m_context1);
	m_context1->CSSetConstantBuffers1(startSlot, numBuffers, cbuffers, firstConstant, numConstants);
}