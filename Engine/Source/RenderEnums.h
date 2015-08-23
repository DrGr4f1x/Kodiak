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

} // namespace Kodiak