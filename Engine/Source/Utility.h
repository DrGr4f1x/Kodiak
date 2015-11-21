// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from PlatformHelpers.h in Microsoft's DirectXTK project 
// https://github.com/Microsoft/DirectXTK
// and Utility.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#pragma once

namespace Kodiak
{

// Smart pointer helpers
struct HandleCloser 
{
	void operator()(HANDLE h) { if (h) CloseHandle(h); } 
};

typedef public std::unique_ptr<void, HandleCloser> ScopedHandle;

inline HANDLE SafeHandle(HANDLE h)
{
	return (h == INVALID_HANDLE_VALUE) ? 0 : h;
}

void SIMDMemCopy(void* __restrict dest, const void* __restrict source, size_t numQuadwords);
void SIMDMemFill(void* __restrict dest, __m128 fillVector, size_t numQuadwords);

} // namespace Kodiak