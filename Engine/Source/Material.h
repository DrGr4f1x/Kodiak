// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "PipelineState.h"
#include "Shader.h"

namespace Kodiak
{

// Forward declarations
class RenderPass;

struct MaterialDesc
{
	ShaderPath		vertexShaderPath;
	ShaderPath		domainShaderPath;
	ShaderPath		hullShaderPath;
	ShaderPath		geometryShaderPath;
	ShaderPath		pixelShaderPath;

	BlendStateDesc			blendStateDesc;
	DepthStencilStateDesc	depthStencilStateDesc;
	RasterizerStateDesc		rasterizerStateDesc;

	std::shared_ptr<RenderPass>	renderPass;
};


size_t ComputeBaseHash(const MaterialDesc& desc);
size_t ComputeHash(const MaterialDesc& desc);



struct ShaderState
{
	std::shared_ptr<VertexShader>		vertexShader;
	std::shared_ptr<DomainShader>		domainShader;
	std::shared_ptr<HullShader>			hullShader;
	std::shared_ptr<GeometryShader>		geometryShader;
	std::shared_ptr<PixelShader>		pixelShader;
};

} // namespace Kodiak


#if defined(DX12)
#include "Material12.h"
#elif defined(DX11)
#include "Material11.h"
#elif defined(VK)
#include "MaterialVk.h"
#else
#error No graphics API defined!
#endif