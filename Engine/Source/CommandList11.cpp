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

#include "ColorBuffer11.h"
#include "CommandListManager11.h"
#include "DepthBuffer11.h"
#include "PipelineState11.h"
#include "Rectangle.h"
#include "Viewport.h"


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


CommandList& CommandList::Begin()
{
	CommandList* newCommandList = CommandList::AllocateCommandList();
	return *newCommandList;
}


uint64_t CommandList::CloseAndExecute(bool waitForCompletion)
{
	ID3D11CommandList* commandList;
	m_context->FinishCommandList(TRUE, &commandList);

	// ExecuteCommandList calls Release on the commandList pointer
	m_owner->ExecuteCommandList(commandList);

	FreeCommandList(this);
	return 0;
}


void CommandList::Initialize(CommandListManager& manager)
{
	m_owner = &manager;
	m_owner->CreateNewDeferredContext(&m_context);
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

		m_context->VSSetShader(PSO.m_vertexShader.Get(), nullptr, 0);
		m_context->HSSetShader(PSO.m_hullShader.Get(), nullptr, 0);
		m_context->DSSetShader(PSO.m_domainShader.Get(), nullptr, 0);
		m_context->GSSetShader(PSO.m_geometryShader.Get(), nullptr, 0);
		m_context->PSSetShader(PSO.m_pixelShader.Get(), nullptr, 0);
	}
}