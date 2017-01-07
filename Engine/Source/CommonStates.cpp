// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "CommonStates.h"

#include "RenderEnums.h"
#include "PipelineState.h"
#include "SamplerDesc.h"


using namespace Kodiak;
using namespace std;


static vector<pair<string, SamplerDesc>> s_namedSamplers;
static bool s_namedSamplersInitialized{ false };
static std::shared_mutex s_namedSamplerMutex;


void InitializeNamedSamplers()
{
	if (s_namedSamplersInitialized) return;

	unique_lock<shared_mutex> CS(s_namedSamplerMutex);

	s_namedSamplers.reserve(16);
	s_namedSamplers.push_back(make_pair("AnisotropicWrap", CommonStates::AnisotropicWrap()));
	s_namedSamplers.push_back(make_pair("LinearSampler", CommonStates::LinearClamp()));
	s_namedSamplers.push_back(make_pair("PointSampler", CommonStates::PointClamp()));
	s_namedSamplers.push_back(make_pair("LinearBorderSampler", CommonStates::LinearBorder()));
	s_namedSamplers.push_back(make_pair("PointBorderSampler", CommonStates::PointBorder()));
	s_namedSamplers.push_back(make_pair("ShadowSampler", CommonStates::ShadowSampler()));

	s_namedSamplersInitialized = true;
}

const BlendStateDesc& CommonStates::Opaque()
{
	static BlendStateDesc desc = BlendStateDesc(Blend::One, Blend::Zero);
	return desc;
}


const BlendStateDesc& CommonStates::AlphaBlend()
{
	static BlendStateDesc desc = BlendStateDesc(Blend::One, Blend::InvSrcAlpha);
	return desc;
}


const BlendStateDesc& CommonStates::Additive()
{
	static BlendStateDesc desc = BlendStateDesc(Blend::SrcAlpha, Blend::One);
	return desc;
}


const BlendStateDesc& CommonStates::NonPremultiplied()
{
	static BlendStateDesc desc = BlendStateDesc(Blend::SrcAlpha, Blend::InvSrcAlpha);
	return desc;
}


const DepthStencilStateDesc& CommonStates::DepthNone()
{
	static DepthStencilStateDesc desc = DepthStencilStateDesc(false, false);
	return desc;
}


const DepthStencilStateDesc& CommonStates::DepthDefault()
{
	static DepthStencilStateDesc desc = DepthStencilStateDesc(true, true);
	return desc;
}


const DepthStencilStateDesc& CommonStates::DepthRead()
{
	static DepthStencilStateDesc desc = DepthStencilStateDesc(true, false);
	return desc;
}


const DepthStencilStateDesc& CommonStates::DepthGreaterEqual()
{
	static DepthStencilStateDesc desc = DepthStencilStateDesc(true, true);
	desc.depthFunc = ComparisonFunc::GreaterEqual;
	return desc;
}


const DepthStencilStateDesc& CommonStates::DepthReadEqual()
{
	static DepthStencilStateDesc desc = DepthStencilStateDesc(true, false);
	desc.depthFunc = ComparisonFunc::Equal;
	return desc;
}


const RasterizerStateDesc& CommonStates::CullNone()
{
	static RasterizerStateDesc desc = RasterizerStateDesc(CullMode::None, FillMode::Solid);
	return desc;
}


const RasterizerStateDesc& CommonStates::CullClockwise()
{
	static RasterizerStateDesc desc = RasterizerStateDesc(CullMode::Front, FillMode::Solid);
	return desc;
}


const RasterizerStateDesc& CommonStates::CullCounterClockwise()
{
	static RasterizerStateDesc desc = RasterizerStateDesc(CullMode::Back, FillMode::Solid);
	return desc;
}


const RasterizerStateDesc& CommonStates::Wireframe()
{
	static RasterizerStateDesc desc = RasterizerStateDesc(CullMode::None, FillMode::Wireframe);
	return desc;
}


const RasterizerStateDesc& CommonStates::Shadow()
{
	static RasterizerStateDesc desc = RasterizerStateDesc(CullMode::Back, FillMode::Solid);
	desc.slopeScaledDepthBias = -1.5f;
	desc.depthBias = -100;
	return desc;
}


const SamplerDesc& CommonStates::AnisotropicWrap()
{
	static SamplerDesc desc{};
	return desc;
}


const SamplerDesc& CommonStates::LinearClamp()
{
	static SamplerDesc desc{ TextureFilter::MinMagMipLinear };
	return desc;
}


const SamplerDesc& CommonStates::PointClamp()
{
	static SamplerDesc desc{ TextureFilter::MinMagMipPoint };
	return desc;
}


const SamplerDesc& CommonStates::LinearBorder()
{
	static SamplerDesc desc{ TextureFilter::MinMagMipLinear, TextureAddress::Border };
	desc.SetBorderColor(Color{ 0.0f, 0.0f, 0.0f, 0.0f });
	return desc;
}


const SamplerDesc& CommonStates::PointBorder()
{
	static SamplerDesc desc{ TextureFilter::MinMagMipPoint, TextureAddress::Border };
	desc.SetBorderColor(Color{ 0.0f, 0.0f, 0.0f, 0.0f });
	return desc;
}


const SamplerDesc& CommonStates::ShadowSampler()
{
	static SamplerDesc desc{ TextureFilter::ComparisonMinMagLinearMipPoint, TextureAddress::Clamp };
	desc.comparisonFunc = ComparisonFunc::GreaterEqual;
	desc.SetBorderColor(Color{ 0.0f, 0.0f, 0.0f, 0.0f });
	return desc;
}


void CommonStates::AddNamedSampler(const string& name, const SamplerDesc& samplerDesc)
{
	InitializeNamedSamplers();

	bool found = false;
	{
		shared_lock<shared_mutex> CS(s_namedSamplerMutex);

		for (const auto& namedSampler : s_namedSamplers)
		{
			if (namedSampler.first == name)
			{
				found = true;
				break;
			}
		}
	}

	if (!found)
	{
		unique_lock<shared_mutex> CS(s_namedSamplerMutex);

		s_namedSamplers.push_back(make_pair(name, samplerDesc));
	}
}


const SamplerDesc& CommonStates::NamedSampler(const string& name)
{
	InitializeNamedSamplers();

	shared_lock<shared_mutex> CS(s_namedSamplerMutex);

	for (const auto& namedSampler : s_namedSamplers)
	{
		if (namedSampler.first == name)
		{
			return namedSampler.second;
		}
	}

	assert_msg(false, "Named sampler %s not found!", name.c_str());
	static SamplerDesc defaultDesc;
	return defaultDesc;
}