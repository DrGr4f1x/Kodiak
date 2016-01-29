// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from PipelineState.cpp in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#include "Stdafx.h"

#include "PipelineState12.h"

#include "DeviceManager12.h"
#include "DXGIUtility.h"
#include "Format.h"
#include "InputLayout12.h"
#include "RenderEnums12.h"
#include "RenderUtils.h"
#include "RootSignature12.h"
#include "Shader12.h"

#include <map>

using namespace Kodiak;
using namespace std;
using namespace Microsoft::WRL;

namespace
{

	map<size_t, ComPtr<ID3D12PipelineState>> s_graphicsPSOHashMap;
	map<size_t, ComPtr<ID3D12PipelineState>> s_computePSOHashMap;

	struct CD3D12_SHADER_BYTECODE : public D3D12_SHADER_BYTECODE
	{
		CD3D12_SHADER_BYTECODE() = default;
		explicit CD3D12_SHADER_BYTECODE(const D3D12_SHADER_BYTECODE& o) :
			D3D12_SHADER_BYTECODE(o)
		{}
		explicit CD3D12_SHADER_BYTECODE(const void* _pShaderBytecode, SIZE_T _BytecodeLength)
		{
			pShaderBytecode = _pShaderBytecode;
			BytecodeLength = _BytecodeLength;
		}
		~CD3D12_SHADER_BYTECODE() {}
		operator const D3D12_SHADER_BYTECODE&() const { return *this; }
	};

} // anonymous namespace


RenderTargetBlendDesc::RenderTargetBlendDesc()
	: blendEnable(false)
	, logicOpEnable(false)
	, srcBlend(Blend::One)
	, dstBlend(Blend::Zero)
	, blendOp(BlendOp::Add)
	, srcBlendAlpha(Blend::One)
	, dstBlendAlpha(Blend::Zero)
	, blendOpAlpha(BlendOp::Add)
	, logicOp(LogicOp::Noop)
	, writeMask(ColorWrite::All)
{}


RenderTargetBlendDesc::RenderTargetBlendDesc(Blend srcBlend, Blend dstBlend)
	: blendEnable((srcBlend != Blend::One) || (dstBlend != Blend::Zero))
	, logicOpEnable(false)
	, srcBlend(srcBlend)
	, dstBlend(dstBlend)
	, blendOp(BlendOp::Add)
	, srcBlendAlpha(srcBlend)
	, dstBlendAlpha(dstBlend)
	, blendOpAlpha(BlendOp::Add)
	, logicOp(LogicOp::Noop)
	, writeMask(ColorWrite::All)
{}


BlendStateDesc::BlendStateDesc()
	: alphaToCoverageEnable(false)
	, independentBlendEnable(false)
{}


BlendStateDesc::BlendStateDesc(Blend srcBlend, Blend dstBlend)
	: alphaToCoverageEnable(false)
	, independentBlendEnable(false)
{
	renderTargetBlend[0] = RenderTargetBlendDesc(srcBlend, dstBlend);
}


RasterizerStateDesc::RasterizerStateDesc()
	: cullMode(CullMode::Back)
	, fillMode(FillMode::Solid)
	, frontCounterClockwise(false)
	, depthBias(0)
	, slopeScaledDepthBias(0.0f)
	, depthBiasClamp(0.0f)
	, depthClipEnable(true)
	, multisampleEnable(false)
	, antialiasedLineEnable(false)
	, forcedSampleCount(0)
	, conservativeRasterizationEnable(false)
{}


RasterizerStateDesc::RasterizerStateDesc(CullMode cullMode, FillMode fillMode)
	: cullMode(cullMode)
	, fillMode(fillMode)
	, frontCounterClockwise(false)
	, depthBias(0)
	, slopeScaledDepthBias(0.0f)
	, depthBiasClamp(0.0f)
	, depthClipEnable(true)
	, multisampleEnable(false)
	, antialiasedLineEnable(false)
	, forcedSampleCount(0)
	, conservativeRasterizationEnable(false)
{}


StencilOpDesc::StencilOpDesc()
	: stencilFailOp(StencilOp::Keep)
	, stencilDepthFailOp(StencilOp::Keep)
	, stencilPassOp(StencilOp::Keep)
	, stencilFunc(ComparisonFunc::Always)
{}


DepthStencilStateDesc::DepthStencilStateDesc()
	: depthEnable(true)
	, depthWriteMask(DepthWrite::All)
	, depthFunc(ComparisonFunc::Less)
	, stencilEnable(false)
	, stencilReadMask(D3D12_DEFAULT_STENCIL_READ_MASK)
	, stencilWriteMask(D3D12_DEFAULT_STENCIL_WRITE_MASK)
	, frontFace()
	, backFace()
{}


DepthStencilStateDesc::DepthStencilStateDesc(bool enable, bool writeEnable)
	: depthEnable(enable)
	, depthWriteMask(writeEnable ? DepthWrite::All : DepthWrite::Zero)
	, depthFunc(ComparisonFunc::Less)
	, stencilEnable(false)
	, stencilReadMask(D3D12_DEFAULT_STENCIL_READ_MASK)
	, stencilWriteMask(D3D12_DEFAULT_STENCIL_WRITE_MASK)
	, frontFace()
	, backFace()
{}


void PSO::DestroyAll()
{
	s_graphicsPSOHashMap.clear();
	s_computePSOHashMap.clear();
}


GraphicsPSO::GraphicsPSO()
{
	ZeroMemory(&m_psoDesc, sizeof(m_psoDesc));
	m_psoDesc.NodeMask = 1;
	m_psoDesc.SampleMask = 0xFFFFFFFFu;
	m_psoDesc.SampleDesc.Count = 1;
	m_psoDesc.InputLayout.NumElements = 0;
}


void GraphicsPSO::SetBlendState(const BlendStateDesc& blendDesc)
{
	m_psoDesc.BlendState.AlphaToCoverageEnable = blendDesc.alphaToCoverageEnable ? TRUE : FALSE;
	m_psoDesc.BlendState.IndependentBlendEnable = blendDesc.independentBlendEnable ? TRUE : FALSE;

	for (uint32_t i = 0; i < 8; ++i)
	{
		auto& rtDesc = m_psoDesc.BlendState.RenderTarget[i];

		rtDesc.BlendEnable = blendDesc.renderTargetBlend[i].blendEnable ? TRUE : FALSE;
		rtDesc.LogicOpEnable = blendDesc.renderTargetBlend[i].logicOpEnable ? TRUE : FALSE;
		rtDesc.SrcBlend = static_cast<D3D12_BLEND>(blendDesc.renderTargetBlend[i].srcBlend);
		rtDesc.DestBlend = static_cast<D3D12_BLEND>(blendDesc.renderTargetBlend[i].dstBlend);
		rtDesc.BlendOp = static_cast<D3D12_BLEND_OP>(blendDesc.renderTargetBlend[i].blendOp);
		rtDesc.SrcBlendAlpha = static_cast<D3D12_BLEND>(blendDesc.renderTargetBlend[i].srcBlendAlpha);
		rtDesc.DestBlendAlpha = static_cast<D3D12_BLEND>(blendDesc.renderTargetBlend[i].dstBlendAlpha);
		rtDesc.BlendOpAlpha = static_cast<D3D12_BLEND_OP>(blendDesc.renderTargetBlend[i].blendOpAlpha);
		rtDesc.LogicOp = static_cast<D3D12_LOGIC_OP>(blendDesc.renderTargetBlend[i].logicOp);
		rtDesc.RenderTargetWriteMask = static_cast<UINT8>(blendDesc.renderTargetBlend[i].writeMask);
	}
}


void GraphicsPSO::SetRasterizerState(const RasterizerStateDesc& rasterizerDesc)
{
	m_psoDesc.RasterizerState.FillMode = static_cast<D3D12_FILL_MODE>(rasterizerDesc.fillMode);
	m_psoDesc.RasterizerState.CullMode = static_cast<D3D12_CULL_MODE>(rasterizerDesc.cullMode);
	m_psoDesc.RasterizerState.FrontCounterClockwise = rasterizerDesc.frontCounterClockwise ? TRUE : FALSE;
	m_psoDesc.RasterizerState.DepthBias = rasterizerDesc.depthBias;
	m_psoDesc.RasterizerState.DepthBiasClamp = rasterizerDesc.depthBiasClamp;
	m_psoDesc.RasterizerState.SlopeScaledDepthBias = rasterizerDesc.slopeScaledDepthBias;
	m_psoDesc.RasterizerState.DepthClipEnable = rasterizerDesc.depthClipEnable ? TRUE : FALSE;
	m_psoDesc.RasterizerState.MultisampleEnable = rasterizerDesc.multisampleEnable ? TRUE : FALSE;
	m_psoDesc.RasterizerState.AntialiasedLineEnable = rasterizerDesc.antialiasedLineEnable ? TRUE : FALSE;
	m_psoDesc.RasterizerState.ForcedSampleCount = rasterizerDesc.forcedSampleCount;
	m_psoDesc.RasterizerState.ConservativeRaster =
		static_cast<D3D12_CONSERVATIVE_RASTERIZATION_MODE>(rasterizerDesc.conservativeRasterizationEnable ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF);
}


void GraphicsPSO::SetDepthStencilState(const DepthStencilStateDesc& depthStencilDesc)
{
	m_psoDesc.DepthStencilState.DepthEnable = depthStencilDesc.depthEnable ? TRUE : FALSE;
	m_psoDesc.DepthStencilState.DepthWriteMask = static_cast<D3D12_DEPTH_WRITE_MASK>(depthStencilDesc.depthWriteMask);
	m_psoDesc.DepthStencilState.DepthFunc = static_cast<D3D12_COMPARISON_FUNC>(depthStencilDesc.depthFunc);
	m_psoDesc.DepthStencilState.StencilEnable = depthStencilDesc.stencilEnable ? TRUE : FALSE;
	m_psoDesc.DepthStencilState.StencilReadMask = depthStencilDesc.stencilReadMask;
	m_psoDesc.DepthStencilState.StencilWriteMask = depthStencilDesc.stencilWriteMask;
	m_psoDesc.DepthStencilState.FrontFace.StencilFailOp = static_cast<D3D12_STENCIL_OP>(depthStencilDesc.frontFace.stencilFailOp);
	m_psoDesc.DepthStencilState.FrontFace.StencilDepthFailOp = static_cast<D3D12_STENCIL_OP>(depthStencilDesc.frontFace.stencilDepthFailOp);
	m_psoDesc.DepthStencilState.FrontFace.StencilPassOp = static_cast<D3D12_STENCIL_OP>(depthStencilDesc.frontFace.stencilPassOp);
	m_psoDesc.DepthStencilState.FrontFace.StencilFunc = static_cast<D3D12_COMPARISON_FUNC>(depthStencilDesc.frontFace.stencilFunc);
	m_psoDesc.DepthStencilState.BackFace.StencilFailOp = static_cast<D3D12_STENCIL_OP>(depthStencilDesc.backFace.stencilFailOp);
	m_psoDesc.DepthStencilState.BackFace.StencilDepthFailOp = static_cast<D3D12_STENCIL_OP>(depthStencilDesc.backFace.stencilDepthFailOp);
	m_psoDesc.DepthStencilState.BackFace.StencilPassOp = static_cast<D3D12_STENCIL_OP>(depthStencilDesc.backFace.stencilPassOp);
	m_psoDesc.DepthStencilState.BackFace.StencilFunc = static_cast<D3D12_COMPARISON_FUNC>(depthStencilDesc.backFace.stencilFunc);
}


void GraphicsPSO::SetSampleMask(uint32_t sampleMask)
{
	m_psoDesc.SampleMask = sampleMask;
}


void GraphicsPSO::SetPrimitiveTopology(PrimitiveTopologyType topology)
{
	m_psoDesc.PrimitiveTopologyType = static_cast<D3D12_PRIMITIVE_TOPOLOGY_TYPE>(topology);
}


void GraphicsPSO::SetRenderTargetFormat(ColorFormat colorFormat, DepthFormat depthFormat, uint32_t msaaCount, uint32_t msaaQuality)
{
	SetRenderTargetFormats(1, &colorFormat, depthFormat, msaaCount, msaaQuality);
}


void GraphicsPSO::SetRenderTargetFormats(uint32_t numRTVs, const ColorFormat* colorFormats, DepthFormat depthFormat, uint32_t msaaCount,
	uint32_t msaaQuality)
{
	assert(numRTVs == 0 || colorFormats != nullptr);
	m_psoDesc.NumRenderTargets = numRTVs;
	for (uint32_t i = 0; i < numRTVs; ++i)
	{
		m_psoDesc.RTVFormats[i] = DXGIUtility::ConvertToDXGI(colorFormats[i]);
	}
	for (uint32_t i = numRTVs; i < 8; ++i)
	{
		m_psoDesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
	}
	m_psoDesc.DSVFormat = DXGIUtility::ConvertToDXGI(depthFormat);
	m_psoDesc.SampleDesc.Count = msaaCount;
	m_psoDesc.SampleDesc.Quality = msaaQuality;
}


void GraphicsPSO::SetInputLayout(const InputLayout& inputLayout)
{
	const auto numElements = inputLayout.elements.size();
	m_psoDesc.InputLayout.NumElements = static_cast<UINT>(numElements);

	if (numElements > 0)
	{
		D3D12_INPUT_ELEMENT_DESC* newElements = (D3D12_INPUT_ELEMENT_DESC*)malloc(sizeof(D3D12_INPUT_ELEMENT_DESC) * numElements);
		memcpy(newElements, &inputLayout.elements[0], numElements * sizeof(D3D12_INPUT_ELEMENT_DESC));
		m_inputLayouts.reset((const D3D12_INPUT_ELEMENT_DESC*)newElements);
	}
	else
	{
		m_inputLayouts = nullptr;
	}
}


void GraphicsPSO::SetPrimitiveRestart(IndexBufferStripCutValue ibProps)
{
	m_psoDesc.IBStripCutValue = static_cast<D3D12_INDEX_BUFFER_STRIP_CUT_VALUE>(ibProps);
}


void GraphicsPSO::SetVertexShader(VertexShader* vertexShader)
{
	m_psoDesc.VS = CD3D12_SHADER_BYTECODE(vertexShader->GetByteCode(), vertexShader->GetByteCodeSize());
}


void GraphicsPSO::SetPixelShader(PixelShader* pixelShader)
{
	m_psoDesc.PS = CD3D12_SHADER_BYTECODE(pixelShader->GetByteCode(), pixelShader->GetByteCodeSize());
}


void GraphicsPSO::SetGeometryShader(GeometryShader* geometryShader)
{
	m_psoDesc.GS = CD3D12_SHADER_BYTECODE(geometryShader->GetByteCode(), geometryShader->GetByteCodeSize());
}


void GraphicsPSO::SetDomainShader(DomainShader* domainShader)
{
	m_psoDesc.DS = CD3D12_SHADER_BYTECODE(domainShader->GetByteCode(), domainShader->GetByteCodeSize());
}


void GraphicsPSO::SetHullShader(HullShader* hullShader)
{
	m_psoDesc.HS = CD3D12_SHADER_BYTECODE(hullShader->GetByteCode(), hullShader->GetByteCodeSize());
}


void GraphicsPSO::Finalize()
{
	// Make sure the root signature is finalized first
	m_psoDesc.pRootSignature = m_rootSignature->GetSignature();
	assert(m_psoDesc.pRootSignature != nullptr);

	m_psoDesc.InputLayout.pInputElementDescs = nullptr;
	size_t HashCode = HashState(&m_psoDesc);
	HashCode = HashStateArray(m_inputLayouts.get(), m_psoDesc.InputLayout.NumElements, HashCode);
	m_psoDesc.InputLayout.pInputElementDescs = m_inputLayouts.get();

	ID3D12PipelineState** PSORef = nullptr;
	bool firstCompile = false;
	{
		static mutex s_HashMapMutex;
		lock_guard<mutex> CS(s_HashMapMutex);
		auto iter = s_graphicsPSOHashMap.find(HashCode);

		// Reserve space so the next inquiry will find that someone got here first.
		if (iter == s_graphicsPSOHashMap.end())
		{
			firstCompile = true;
			PSORef = s_graphicsPSOHashMap[HashCode].GetAddressOf();
		}
		else
		{
			PSORef = iter->second.GetAddressOf();
		}
	}

	if (firstCompile)
	{
		ThrowIfFailed(g_device->CreateGraphicsPipelineState(&m_psoDesc, IID_PPV_ARGS(&m_pso)));
		s_graphicsPSOHashMap[HashCode].Attach(m_pso);
	}
	else
	{
		while (*PSORef == nullptr)
		{
			this_thread::yield();
		}
		m_pso = *PSORef;
	}
}


ComputePSO::ComputePSO()
{
	ZeroMemory(&m_psoDesc, sizeof(m_psoDesc));
	m_psoDesc.NodeMask = 1;
}


void ComputePSO::SetComputeShader(ComputeShader* computeShader)
{
	m_psoDesc.CS = CD3D12_SHADER_BYTECODE(computeShader->GetByteCode(), computeShader->GetByteCodeSize());
}


void ComputePSO::Finalize()
{
	// Make sure the root signature is finalized first
	m_psoDesc.pRootSignature = m_rootSignature->GetSignature();
	assert(m_psoDesc.pRootSignature != nullptr);

	size_t HashCode = HashState(&m_psoDesc);

	ID3D12PipelineState** PSORef = nullptr;
	bool firstCompile = false;
	{
		static mutex s_HashMapMutex;
		lock_guard<mutex> CS(s_HashMapMutex);
		auto iter = s_computePSOHashMap.find(HashCode);

		// Reserve space so the next inquiry will find that someone got here first.
		if (iter == s_computePSOHashMap.end())
		{
			firstCompile = true;
			PSORef = s_computePSOHashMap[HashCode].GetAddressOf();
		}
		else
		{
			PSORef = iter->second.GetAddressOf();
		}
	}

	if (firstCompile)
	{
		ThrowIfFailed(g_device->CreateComputePipelineState(&m_psoDesc, IID_PPV_ARGS(&m_pso)));
		s_computePSOHashMap[HashCode].Attach(m_pso);
	}
	else
	{
		while (*PSORef == nullptr)
		{
			this_thread::yield();
		}
		m_pso = *PSORef;
	}
}