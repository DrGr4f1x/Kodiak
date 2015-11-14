// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from Hash.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#pragma once

namespace Kodiak
{

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw;
	}
}


inline size_t HashIterate(size_t next, size_t currentHash = 2166136261U)
{
	return 16777619U * currentHash ^ next;
}


template <typename T> inline size_t HashRange(const T* begin, const T* end, size_t initialVal = 2166136261U)
{
	size_t val = initialVal;

	while (begin < end)
	{
		val = HashIterate((size_t)*begin++, val);
	}

	return val;
}


template <typename T> inline size_t HashStateArray(const T* stateDesc, size_t count, size_t initialVal = 2166136261U)
{
	static_assert((sizeof(T) & 3) == 0, "State object is not word-aligned");
	return HashRange((uint32_t*)stateDesc, (uint32_t*)(stateDesc + count), initialVal);
}


template <typename T> inline size_t HashState(const T* stateDesc, size_t initialVal = 2166136261U)
{
	static_assert((sizeof(T) & 3) == 0, "State object is not word-aligned");
	return HashRange((uint32_t*)stateDesc, (uint32_t*)(stateDesc + 1), initialVal);
}


} // namespace Kodiak