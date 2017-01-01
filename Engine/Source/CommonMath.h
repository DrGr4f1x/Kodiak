// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from Common.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#pragma once

namespace Math
{

template <typename T> __forceinline T AlignUpWithMask(T value, size_t mask)
{
	return (T)(((size_t)value + mask) & ~mask);
}


template <typename T> __forceinline T AlignDownWithMask(T value, size_t mask)
{
	return (T)((size_t)value & ~mask);
}


template <typename T> __forceinline T AlignUp(T value, size_t alignment)
{
	return AlignUpWithMask(value, alignment - 1);
}


template <typename T> __forceinline T AlignDown(T value, size_t alignment)
{
	return AlignDownWithMask(value, alignment - 1);
}


template <typename T> __forceinline bool IsAligned(T value, size_t alignment)
{
	return 0 == ((size_t)value & (alignment - 1));
}


template <typename T> __forceinline T DivideByMultiple(T value, size_t alignment)
{
	return (T)((value + alignment - 1) / alignment);
}


template <typename T> __forceinline bool IsPowerOfTwo(T value)
{
	return 0 == (value & (value - 1));
}


template <typename T> __forceinline bool IsDivisible(T value, T divisor)
{
	return (value / divisor) * divisor == value;
}


__forceinline DirectX::XMVECTOR SplatZero()
{
	return DirectX::XMVectorZero();
}


#if !defined(_XM_NO_INTRINSICS_) && defined(_XM_SSE_INTRINSICS_)

__forceinline DirectX::XMVECTOR SplatOne(DirectX::XMVECTOR zero = SplatZero())
{
	__m128i AllBits = _mm_castps_si128(_mm_cmpeq_ps(zero, zero));
	return _mm_castsi128_ps(_mm_slli_epi32(_mm_srli_epi32(AllBits, 25), 23));	// return 0x3F800000
																				//return _mm_cvtepi32_ps(_mm_srli_epi32(SetAllBits(zero), 31));				// return (float)1;  (alternate method)
}

#if defined(_XM_SSE4_INTRINSICS_)
__forceinline DirectX::XMVECTOR CreateXUnitVector(DirectX::XMVECTOR one = SplatOne())
{
	return _mm_insert_ps(one, one, 0x0E);
}


__forceinline DirectX::XMVECTOR CreateYUnitVector(DirectX::XMVECTOR one = SplatOne())
{
	return _mm_insert_ps(one, one, 0x0D);
}


__forceinline DirectX::XMVECTOR CreateZUnitVector(DirectX::XMVECTOR one = SplatOne())
{
	return _mm_insert_ps(one, one, 0x0B);
}


__forceinline DirectX::XMVECTOR CreateWUnitVector(DirectX::XMVECTOR one = SplatOne())
{
	return _mm_insert_ps(one, one, 0x07);
}


__forceinline DirectX::XMVECTOR SetWToZero(DirectX::FXMVECTOR vec)
{
	return _mm_insert_ps(vec, vec, 0x08);
}


__forceinline DirectX::XMVECTOR SetWToOne(DirectX::FXMVECTOR vec)
{
	return _mm_blend_ps(vec, SplatOne(), 0x8);
}

#else

__forceinline DirectX::XMVECTOR CreateXUnitVector(DirectX::XMVECTOR one = SplatOne())
{
	return _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(one), 12));
}


__forceinline DirectX::XMVECTOR CreateYUnitVector(DirectX::XMVECTOR one = SplatOne())
{
	DirectX::XMVECTOR unitx = CreateXUnitVector(one);
	return _mm_castsi128_ps(_mm_slli_si128(_mm_castps_si128(unitx), 4));
}


__forceinline DirectX::XMVECTOR CreateZUnitVector(DirectX::XMVECTOR one = SplatOne())
{
	DirectX::XMVECTOR unitx = CreateXUnitVector(one);
	return _mm_castsi128_ps(_mm_slli_si128(_mm_castps_si128(unitx), 8));
}


__forceinline DirectX::XMVECTOR CreateWUnitVector(DirectX::XMVECTOR one = SplatOne())
{
	return _mm_castsi128_ps(_mm_slli_si128(_mm_castps_si128(one), 12));
}


__forceinline DirectX::XMVECTOR SetWToZero(DirectX::FXMVECTOR vec)
{
	__m128i maskOffW = _mm_srli_si128(_mm_castps_si128(_mm_cmpeq_ps(vec, vec)), 4);
	return _mm_and_ps(vec, _mm_castsi128_ps(maskOffW));
}


__forceinline DirectX::XMVECTOR SetWToOne(DirectX::FXMVECTOR vec)
{
	return _mm_movelh_ps(vec, _mm_unpackhi_ps(vec, SplatOne()));
}

#endif

#else // !_XM_SSE_INTRINSICS_

__forceinline DirectX::XMVECTOR SplatOne() { return DirectX::XMVectorSplatOne(); }
__forceinline DirectX::XMVECTOR CreateXUnitVector() { return DirectX::g_XMIdentityR0; }
__forceinline DirectX::XMVECTOR CreateYUnitVector() { return DirectX::g_XMIdentityR1; }
__forceinline DirectX::XMVECTOR CreateZUnitVector() { return DirectX::g_XMIdentityR2; }
__forceinline DirectX::XMVECTOR CreateWUnitVector() { return DirectX::g_XMIdentityR3; }
__forceinline DirectX::XMVECTOR SetWToZero(DirectX::FXMVECTOR vec) { return DirectX::XMVectorAndInt(vec, DirectX::g_XMMask3); }
__forceinline DirectX::XMVECTOR SetWToOne(DirectX::FXMVECTOR vec) { return DirectX::XMVectorSelect(DirectX::g_XMIdentityR3, vec, DirectX::g_XMMask3); }

#endif

__forceinline DirectX::XMVECTOR CreateNegXUnitVector() { return DirectX::XMVectorNegate(CreateXUnitVector()); }
__forceinline DirectX::XMVECTOR CreateNegYUnitVector() { return DirectX::XMVectorNegate(CreateYUnitVector()); }
__forceinline DirectX::XMVECTOR CreateNegZUnitVector() { return DirectX::XMVectorNegate(CreateZUnitVector()); }
__forceinline DirectX::XMVECTOR CreateNegWUnitVector() { return DirectX::XMVectorNegate(CreateWUnitVector()); }

__forceinline float ConvertToRadians(float deg) { return DirectX::XMConvertToRadians(deg); }
__forceinline float ConvertToDegrees(float rad) { return DirectX::XMConvertToDegrees(rad); }

enum EZeroTag { kZero, kOrigin };
enum EIdentityTag { kOne, kIdentity };
enum EXUnitVector { kXUnitVector };
enum EYUnitVector { kYUnitVector };
enum EZUnitVector { kZUnitVector };
enum EWUnitVector { kWUnitVector };
enum ENegXUnitVector { kNegXUnitVector };
enum ENegYUnitVector { kNegYUnitVector };
enum ENegZUnitVector { kNegZUnitVector };
enum ENegWUnitVector { kNegWUnitVector };

} // namespace Math