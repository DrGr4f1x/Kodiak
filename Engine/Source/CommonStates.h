// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

namespace Kodiak
{

// Forward declarations
struct BlendStateDesc;
struct DepthStencilStateDesc;
struct RasterizerStateDesc;
struct SamplerDesc;

class CommonStates
{
public:
	// Blend states
	static const BlendStateDesc& Opaque();
	static const BlendStateDesc& AlphaBlend();
	static const BlendStateDesc& Additive();
	static const BlendStateDesc& NonPremultiplied();

	// Depth-stencil states
	static const DepthStencilStateDesc& DepthNone();
	static const DepthStencilStateDesc& DepthDefault();
	static const DepthStencilStateDesc& DepthRead();
	static const DepthStencilStateDesc& DepthGreaterEqual();
	static const DepthStencilStateDesc& DepthReadEqual();

	// Rasterizer states
	static const RasterizerStateDesc& CullNone();
	static const RasterizerStateDesc& CullClockwise();
	static const RasterizerStateDesc& CullCounterClockwise();
	static const RasterizerStateDesc& Wireframe();
	static const RasterizerStateDesc& Shadow();

	// Sampler states
	static const SamplerDesc& AnisotropicWrap();
	static const SamplerDesc& LinearClamp();
	static const SamplerDesc& PointClamp();
	static const SamplerDesc& LinearBorder();
	static const SamplerDesc& PointBorder();
	static const SamplerDesc& ShadowSampler();
	static void AddNamedSampler(const std::string& name, const SamplerDesc& desc);
	static const SamplerDesc& NamedSampler(const std::string& name);
};

} // namespace Kodiak