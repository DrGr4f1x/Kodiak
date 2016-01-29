// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#if defined(DX12)
#include "RenderEnums12.h"
#elif defined(DX11)
#include "RenderEnums11.h"
#elif defined(VK)
#include "RenderEnumsVk.h"
#else
#error No graphics API defined!
#endif

namespace Kodiak
{

enum class PrimitiveTopology
{
	PointList = D3D_PRIMITIVE_TOPOLOGY_POINTLIST,
	LineList = D3D_PRIMITIVE_TOPOLOGY_LINELIST,
	LineStrip = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP,
	TriangleList = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
	TriangleStrip = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
};


enum class IndexBufferFormat
{
	UInt16 = DXGI_FORMAT_R16_UINT,
	UInt32 = DXGI_FORMAT_R32_UINT
};


enum class ShaderType
{
	Vertex,
	Domain,
	Hull,
	Geometry,
	Pixel,
	Compute
};


enum class ShaderVariableType
{
	Bool,
	Int,
	Int2,
	Int3,
	Int4,
	Float,
	Float2,
	Float3,
	Float4,
	Float4x4,

	Unsupported
};


enum class ShaderResourceType
{
	Texture,
	TBuffer,
	UAVRWTyped,
	Structured,
	UAVRWStructured,
	ByteAddress,
	UAVRWByteAddress,
	UAVAppendStructured,
	UAVConsumeStructured,
	UAVRWStructuredWithCounter,

	Unsupported
};


enum class ShaderResourceDimension
{
	Buffer,
	Texture1d,
	Texture1dArray,
	Texture2d,
	Texture2dArray,
	Texture2dMS,
	Texture2dMSArray,
	Texture3d,
	TextureCube,
	TextureCubeArray,

	Unsupported
};


} // namespace Kodiak