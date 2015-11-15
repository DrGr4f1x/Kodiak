// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from Utility.cpp in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#include "Stdafx.h"

#include "Utility.h"

#include "MathUtil.h"


namespace Kodiak
{

// A faster version of memcopy that uses SSE instructions.  TODO:  Write an ARM variant if necessary.
void SIMDMemCopy(void* __restrict _dest, const void* __restrict _source, size_t numQuadwords)
{
	assert(Math::IsAligned(_dest, 16));
	assert(Math::IsAligned(_source, 16));

	__m128i* __restrict dest = (__m128i* __restrict)_dest;
	const __m128i* __restrict source = (const __m128i* __restrict)_source;

	// Discover how many quadwords precede a cache line boundary.  Copy them separately.
	size_t initialQuadwordCount = (4 - ((size_t)source >> 4) & 3) & 3;
	if (initialQuadwordCount > numQuadwords)
	{
		initialQuadwordCount = numQuadwords;
	}

	switch (initialQuadwordCount)
	{
	case 3: _mm_stream_si128(dest + 2, _mm_load_si128(source + 2));	 // Fall through
	case 2: _mm_stream_si128(dest + 1, _mm_load_si128(source + 1));	 // Fall through
	case 1: _mm_stream_si128(dest + 0, _mm_load_si128(source + 0));	 // Fall through
	default:
		break;
	}

	if (numQuadwords == initialQuadwordCount)
	{
		return;
	}

	dest += initialQuadwordCount;
	source += initialQuadwordCount;
	numQuadwords -= initialQuadwordCount;

	size_t cacheLines = numQuadwords >> 2;

	switch (cacheLines)
	{
	default:
	case 10: _mm_prefetch((char*)(source + 36), _MM_HINT_NTA);	// Fall through
	case 9:  _mm_prefetch((char*)(source + 32), _MM_HINT_NTA);	// Fall through
	case 8:  _mm_prefetch((char*)(source + 28), _MM_HINT_NTA);	// Fall through
	case 7:  _mm_prefetch((char*)(source + 24), _MM_HINT_NTA);	// Fall through
	case 6:  _mm_prefetch((char*)(source + 20), _MM_HINT_NTA);	// Fall through
	case 5:  _mm_prefetch((char*)(source + 16), _MM_HINT_NTA);	// Fall through
	case 4:  _mm_prefetch((char*)(source + 12), _MM_HINT_NTA);	// Fall through
	case 3:  _mm_prefetch((char*)(source + 8), _MM_HINT_NTA);	// Fall through
	case 2:  _mm_prefetch((char*)(source + 4), _MM_HINT_NTA);	// Fall through
	case 1:  _mm_prefetch((char*)(source + 0), _MM_HINT_NTA);	// Fall through

																// Do four quadwords per loop to minimize stalls.
		for (size_t i = cacheLines; i > 0; --i)
		{
			// If this is a large copy, start prefetching future cache lines.  This also prefetches the
			// trailing quadwords that are not part of a whole cache line.
			if (i >= 10)
			{
				_mm_prefetch((char*)(source + 40), _MM_HINT_NTA);
			}

			_mm_stream_si128(dest + 0, _mm_load_si128(source + 0));
			_mm_stream_si128(dest + 1, _mm_load_si128(source + 1));
			_mm_stream_si128(dest + 2, _mm_load_si128(source + 2));
			_mm_stream_si128(dest + 3, _mm_load_si128(source + 3));

			dest += 4;
			source += 4;
		}

	case 0:	// No whole cache lines to read
		break;
	}

	// Copy the remaining quadwords
	switch (numQuadwords & 3)
	{
	case 3: _mm_stream_si128(dest + 2, _mm_load_si128(source + 2));	 // Fall through
	case 2: _mm_stream_si128(dest + 1, _mm_load_si128(source + 1));	 // Fall through
	case 1: _mm_stream_si128(dest + 0, _mm_load_si128(source + 0));	 // Fall through
	default:
		break;
	}

	_mm_sfence();
}


void SIMDMemFill(void* __restrict _dest, __m128 fillVector, size_t numQuadwords)
{
	assert(Math::IsAligned(_dest, 16));

	register const __m128i source = _mm_castps_si128(fillVector);
	__m128i* __restrict dest = (__m128i* __restrict)_dest;

	switch (((size_t)dest >> 4) & 3)
	{
	case 1: _mm_stream_si128(dest++, source); --numQuadwords;	 // Fall through
	case 2: _mm_stream_si128(dest++, source); --numQuadwords;	 // Fall through
	case 3: _mm_stream_si128(dest++, source); --numQuadwords;	 // Fall through
	default:
		break;
	}

	size_t WholeCacheLines = numQuadwords >> 2;

	// Do four quadwords per loop to minimize stalls.
	while (WholeCacheLines--)
	{
		_mm_stream_si128(dest++, source);
		_mm_stream_si128(dest++, source);
		_mm_stream_si128(dest++, source);
		_mm_stream_si128(dest++, source);
	}

	// Copy the remaining quadwords
	switch (numQuadwords & 3)
	{
	case 3: _mm_stream_si128(dest++, source);	 // Fall through
	case 2: _mm_stream_si128(dest++, source);	 // Fall through
	case 1: _mm_stream_si128(dest++, source);	 // Fall through
	default:
		break;
	}

	_mm_sfence();
}


} // namespace Kodiak