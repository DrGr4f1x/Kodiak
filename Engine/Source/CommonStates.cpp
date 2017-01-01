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


using namespace Kodiak;


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