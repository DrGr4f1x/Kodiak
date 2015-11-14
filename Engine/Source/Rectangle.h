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

struct Rectangle
{
	Rectangle() = default;
	Rectangle(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom)
		: left(left)
		, top(top)
		, right(right)
		, bottom(bottom)
	{}

	uint32_t left{ 0 };
	uint32_t top{ 0 };
	uint32_t right{ 1 };
	uint32_t bottom{ 1 };
};

} // namespace Kodiak