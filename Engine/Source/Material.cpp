// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "Material.h"

#include "RenderUtils.h"

using namespace Kodiak;
using namespace std;


namespace Kodiak
{

size_t ComputeBaseHash(const MaterialDesc& desc)
{
	size_t hashVal = 0;

	// Vertex shader
	{
		hash<string> hashFunc;
		size_t hashCode = hashFunc(desc.vertexShaderPath.GetFullPath());

		hashVal = HashIterate(hashCode);
	}

	// Domain shader
	if (desc.domainShaderPath.HasPath())
	{
		hash<string> hashFunc;
		size_t hashCode = hashFunc(desc.domainShaderPath.GetFullPath());

		hashVal = HashIterate(hashCode, hashVal);
	}

	// Hull shader
	if (desc.hullShaderPath.HasPath())
	{
		hash<string> hashFunc;
		size_t hashCode = hashFunc(desc.hullShaderPath.GetFullPath());

		hashVal = HashIterate(hashCode, hashVal);
	}

	// Geometry shader
	if (desc.geometryShaderPath.HasPath())
	{
		hash<string> hashFunc;
		size_t hashCode = hashFunc(desc.geometryShaderPath.GetFullPath());

		hashVal = HashIterate(hashCode, hashVal);
	}

	// Pixel shader
	if (desc.pixelShaderPath.HasPath())
	{
		hash<string> hashFunc;
		size_t hashCode = hashFunc(desc.pixelShaderPath.GetFullPath());

		hashVal = HashIterate(hashCode, hashVal);
	}

	// State
	hashVal = HashState(&desc.blendStateDesc, hashVal);
	hashVal = HashState(&desc.depthStencilStateDesc, hashVal);
	hashVal = HashState(&desc.rasterizerStateDesc);

	return hashVal;
}

} // namespace Kodiak