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

enum class ColorFormat
{
	Unknown,
	R8G8B8A8,
	R8_UNorm,
	R8_UInt,
	R11G11B10_Float,
	R16_Float,
	R32_Float,
	R32_UInt
};

enum class DepthFormat
{
	Unknown,
	D32,
	D32S8,
	D24S8,
	D16
};

} // namespace Kodiak