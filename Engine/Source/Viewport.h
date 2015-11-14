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

struct Viewport
{
	Viewport() = default;
	Viewport(float topLeftX, float topLeftY, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f)
		: topLeftX(topLeftX)
		, topLeftY(topLeftY)
		, width(width)
		, height(height)
		, minDepth(minDepth)
		, maxDepth(maxDepth)
	{}

	float topLeftX{ 0.0f };
	float topLeftY{ 0.0f };
	float width{ 1.0f };
	float height{ 1.0f };
	float minDepth{ 0.0f };
	float maxDepth{ 1.0f };
};

} // namespace Kodiak