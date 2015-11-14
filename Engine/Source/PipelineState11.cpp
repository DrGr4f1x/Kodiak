// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "PipelineState11.h"

#include "DeviceManager11.h"
#include "RenderEnums11.h"
#include "RenderUtils.h"
#include "Shader11.h"

#include <map>
#include <ppltasks.h>

using namespace Kodiak;
using namespace std;
using namespace Microsoft::WRL;

namespace
{
	// Hash-maps of the render state objects
	map<size_t, ComPtr<ID3D11BlendState>>			s_blendStateHashMap;
	map<size_t, ComPtr<ID3D11RasterizerState>>		s_rasterizerStateHashMap;
	map<size_t, ComPtr<ID3D11DepthStencilState>>	s_depthStencilStateHashMap;

} // anonymous namespace


RenderTargetBlendDesc::RenderTargetBlendDesc()
	: blendEnable(false)
	, srcBlend(Blend::One)
	, dstBlend(Blend::Zero)
	, blendOp(BlendOp::Add)
	, srcBlendAlpha(Blend::One)
	, dstBlendAlpha(Blend::Zero)
	, blendOpAlpha(BlendOp::Add)
	, writeMask(ColorWrite::All)
{}


RenderTargetBlendDesc::RenderTargetBlendDesc(Blend srcBlend, Blend dstBlend)
	: blendEnable((srcBlend != Blend::One) || (dstBlend != Blend::Zero))
	, srcBlend(srcBlend)
	, dstBlend(dstBlend)
	, blendOp(BlendOp::Add)
	, srcBlendAlpha(Blend::One)
	, dstBlendAlpha(Blend::Zero)
	, blendOpAlpha(BlendOp::Add)
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
	, scissorEnable(false)
	, multisampleEnable(false)
	, antialiasedLineEnable(false)
{}


RasterizerStateDesc::RasterizerStateDesc(CullMode cullMode, FillMode fillMode)
	: cullMode(cullMode)
	, fillMode(fillMode)
	, frontCounterClockwise(false)
	, depthBias(0)
	, slopeScaledDepthBias(0.0f)
	, depthBiasClamp(0.0f)
	, depthClipEnable(true)
	, scissorEnable(false)
	, multisampleEnable(false)
	, antialiasedLineEnable(false)
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
	, stencilReadMask(D3D11_DEFAULT_STENCIL_READ_MASK)
	, stencilWriteMask(D3D11_DEFAULT_STENCIL_WRITE_MASK)
	, frontFace()
	, backFace()
{}


DepthStencilStateDesc::DepthStencilStateDesc(bool enable, bool writeEnable)
	: depthEnable(enable)
	, depthWriteMask(writeEnable ? DepthWrite::All : DepthWrite::Zero)
	, depthFunc(ComparisonFunc::Less)
	, stencilEnable(false)
	, stencilReadMask(D3D11_DEFAULT_STENCIL_READ_MASK)
	, stencilWriteMask(D3D11_DEFAULT_STENCIL_WRITE_MASK)
	, frontFace()
	, backFace()
{}


GraphicsPSO::GraphicsPSO()
{
	ZeroMemory(&m_desc, sizeof(GraphicsPSODesc));
}


void GraphicsPSO::SetBlendState(const BlendStateDesc& blendDesc)
{
	m_desc.blendDesc.AlphaToCoverageEnable = blendDesc.alphaToCoverageEnable ? TRUE : FALSE;
	m_desc.blendDesc.IndependentBlendEnable = blendDesc.independentBlendEnable ? TRUE : FALSE;

	for (uint32_t i = 0; i < 8; ++i)
	{
		auto& rtDesc = m_desc.blendDesc.RenderTarget[i];

		rtDesc.BlendEnable = blendDesc.renderTargetBlend[i].blendEnable ? TRUE : FALSE;
		rtDesc.SrcBlend = static_cast<D3D11_BLEND>(blendDesc.renderTargetBlend[i].srcBlend);
		rtDesc.DestBlend = static_cast<D3D11_BLEND>(blendDesc.renderTargetBlend[i].dstBlend);
		rtDesc.BlendOp = static_cast<D3D11_BLEND_OP>(blendDesc.renderTargetBlend[i].blendOp);
		rtDesc.SrcBlendAlpha = static_cast<D3D11_BLEND>(blendDesc.renderTargetBlend[i].srcBlendAlpha);
		rtDesc.DestBlendAlpha = static_cast<D3D11_BLEND>(blendDesc.renderTargetBlend[i].dstBlendAlpha);
		rtDesc.BlendOpAlpha = static_cast<D3D11_BLEND_OP>(blendDesc.renderTargetBlend[i].blendOpAlpha);
		rtDesc.RenderTargetWriteMask = static_cast<UINT8>(blendDesc.renderTargetBlend[i].writeMask);
	}
}


void GraphicsPSO::SetRasterizerState(const RasterizerStateDesc& rasterizerDesc)
{
	m_desc.rasterizerDesc.FillMode = static_cast<D3D11_FILL_MODE>(rasterizerDesc.fillMode);
	m_desc.rasterizerDesc.CullMode = static_cast<D3D11_CULL_MODE>(rasterizerDesc.cullMode);
	m_desc.rasterizerDesc.FrontCounterClockwise = rasterizerDesc.frontCounterClockwise ? TRUE : FALSE;
	m_desc.rasterizerDesc.DepthBias = rasterizerDesc.depthBias;
	m_desc.rasterizerDesc.DepthBiasClamp = rasterizerDesc.depthBiasClamp;
	m_desc.rasterizerDesc.SlopeScaledDepthBias = rasterizerDesc.slopeScaledDepthBias;
	m_desc.rasterizerDesc.DepthClipEnable = rasterizerDesc.depthClipEnable ? TRUE : FALSE;
	m_desc.rasterizerDesc.ScissorEnable = rasterizerDesc.scissorEnable ? TRUE : FALSE;
	m_desc.rasterizerDesc.MultisampleEnable = rasterizerDesc.multisampleEnable ? TRUE : FALSE;
	m_desc.rasterizerDesc.AntialiasedLineEnable = rasterizerDesc.antialiasedLineEnable ? TRUE : FALSE;
}


void GraphicsPSO::SetDepthStencilState(const DepthStencilStateDesc& depthStencilDesc)
{
	m_desc.depthStencilDesc.DepthEnable = depthStencilDesc.depthEnable ? TRUE : FALSE;
	m_desc.depthStencilDesc.DepthWriteMask = static_cast<D3D11_DEPTH_WRITE_MASK>(depthStencilDesc.depthWriteMask);
	m_desc.depthStencilDesc.DepthFunc = static_cast<D3D11_COMPARISON_FUNC>(depthStencilDesc.depthFunc);
	m_desc.depthStencilDesc.StencilEnable = depthStencilDesc.stencilEnable ? TRUE : FALSE;
	m_desc.depthStencilDesc.StencilReadMask = depthStencilDesc.stencilReadMask;
	m_desc.depthStencilDesc.StencilWriteMask = depthStencilDesc.stencilWriteMask;
	m_desc.depthStencilDesc.FrontFace.StencilFailOp = static_cast<D3D11_STENCIL_OP>(depthStencilDesc.frontFace.stencilFailOp);
	m_desc.depthStencilDesc.FrontFace.StencilDepthFailOp = static_cast<D3D11_STENCIL_OP>(depthStencilDesc.frontFace.stencilDepthFailOp);
	m_desc.depthStencilDesc.FrontFace.StencilPassOp = static_cast<D3D11_STENCIL_OP>(depthStencilDesc.frontFace.stencilPassOp);
	m_desc.depthStencilDesc.FrontFace.StencilFunc = static_cast<D3D11_COMPARISON_FUNC>(depthStencilDesc.frontFace.stencilFunc);
	m_desc.depthStencilDesc.BackFace.StencilFailOp = static_cast<D3D11_STENCIL_OP>(depthStencilDesc.backFace.stencilFailOp);
	m_desc.depthStencilDesc.BackFace.StencilDepthFailOp = static_cast<D3D11_STENCIL_OP>(depthStencilDesc.backFace.stencilDepthFailOp);
	m_desc.depthStencilDesc.BackFace.StencilPassOp = static_cast<D3D11_STENCIL_OP>(depthStencilDesc.backFace.stencilPassOp);
	m_desc.depthStencilDesc.BackFace.StencilFunc = static_cast<D3D11_COMPARISON_FUNC>(depthStencilDesc.backFace.stencilFunc);
}


void GraphicsPSO::SetVertexShader(VertexShader* vertexShader)
{
	auto d3dVS = vertexShader->GetShader();
	m_desc.vertexShader = d3dVS;
	m_vertexShader = d3dVS;
}


void GraphicsPSO::SetPixelShader(PixelShader* pixelShader)
{
	auto d3dPS = pixelShader->GetShader();
	m_desc.pixelShader = d3dPS;
	m_pixelShader = d3dPS;
}


void GraphicsPSO::SetGeometryShader(GeometryShader* geometryShader)
{
	auto d3dGS = geometryShader->GetShader();
	m_desc.geometryShader = d3dGS;
	m_geometryShader = d3dGS;
}


void GraphicsPSO::SetHullShader(HullShader* hullShader)
{
	auto d3dHS = hullShader->GetShader();
	m_desc.hullShader = d3dHS;
	m_hullShader = d3dHS;
}


void GraphicsPSO::SetDomainShader(DomainShader* domainShader)
{
	auto d3dDS = domainShader->GetShader();
	m_desc.domainShader = d3dDS;
	m_domainShader = d3dDS;
}


void GraphicsPSO::Finalize()
{
	using namespace concurrency;
	
	auto blendStateTask = create_task([this]() { this->CompileBlendState(); });
	auto rasterizerStateTask = create_task([this]() { this->CompileRasterizerState(); });
	auto depthStencilStateTask = create_task([this]() { this->CompileDepthStencilState(); });

	(blendStateTask && rasterizerStateTask && depthStencilStateTask).wait();
}


void GraphicsPSO::CompileBlendState()
{
	size_t hashCode = HashState(&m_desc.blendDesc);

	ID3D11BlendState** blendStateRef = nullptr;
	bool firstCompile = false;
	{
		static mutex blendStateMutex;
		lock_guard<mutex> CS(blendStateMutex);

		auto iter = s_blendStateHashMap.find(hashCode);

		// Reserve space so the next inquiry will find that someone got here first
		if (iter == s_blendStateHashMap.end())
		{
			firstCompile = true;
			blendStateRef = s_blendStateHashMap[hashCode].GetAddressOf();
		}
		else
		{
			blendStateRef = iter->second.GetAddressOf();
		}
	}

	if (firstCompile)
	{
		ThrowIfFailed(g_device->CreateBlendState(&m_desc.blendDesc, &m_blendState));
		s_blendStateHashMap[hashCode].Attach(m_blendState);
	}
	else
	{
		while (*blendStateRef == nullptr)
		{
			this_thread::yield();
		}
		m_blendState = *blendStateRef;
	}
}


void GraphicsPSO::CompileRasterizerState()
{
	size_t hashCode = HashState(&m_desc.rasterizerDesc);

	ID3D11RasterizerState** rasterizerStateRef = nullptr;
	bool firstCompile = false;
	{
		static mutex rasterizerStateMutex;
		lock_guard<mutex> CS(rasterizerStateMutex);

		auto iter = s_rasterizerStateHashMap.find(hashCode);

		// Reserve space so the next inquiry will find that someone got here first
		if (iter == s_rasterizerStateHashMap.end())
		{
			firstCompile = true;
			rasterizerStateRef = s_rasterizerStateHashMap[hashCode].GetAddressOf();
		}
		else
		{
			rasterizerStateRef = iter->second.GetAddressOf();
		}
	}

	if (firstCompile)
	{
		ThrowIfFailed(g_device->CreateRasterizerState(&m_desc.rasterizerDesc, &m_rasterizerState));
		s_rasterizerStateHashMap[hashCode].Attach(m_rasterizerState);
	}
	else
	{
		while (*rasterizerStateRef == nullptr)
		{
			this_thread::yield();
		}
		m_rasterizerState = *rasterizerStateRef;
	}
}


void GraphicsPSO::CompileDepthStencilState()
{
	size_t hashCode = HashState(&m_desc.depthStencilDesc);

	ID3D11DepthStencilState** depthStencilStateRef = nullptr;
	bool firstCompile = false;
	{
		static mutex depthStencilStateMutex;
		lock_guard<mutex> CS(depthStencilStateMutex);

		auto iter = s_depthStencilStateHashMap.find(hashCode);

		// Reserve space so the next inquiry will find that someone got here first
		if (iter == s_depthStencilStateHashMap.end())
		{
			firstCompile = true;
			depthStencilStateRef = s_depthStencilStateHashMap[hashCode].GetAddressOf();
		}
		else
		{
			depthStencilStateRef = iter->second.GetAddressOf();
		}
	}

	if (firstCompile)
	{
		ThrowIfFailed(g_device->CreateDepthStencilState(&m_desc.depthStencilDesc, &m_depthStencilState));
		s_depthStencilStateHashMap[hashCode].Attach(m_depthStencilState);
	}
	else
	{
		while (*depthStencilStateRef == nullptr)
		{
			this_thread::yield();
		}
		m_depthStencilState = *depthStencilStateRef;
	}
}