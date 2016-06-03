// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from ShaderUtility.hlsl in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

// This assumes the default color gamut found in sRGB and REC709.  The color primaries determine these
// coefficients.  Note that this operates on linear values, not gamma space.
float RGBToLuminance(float3 x)
{
	return dot(x, float3(0.212671, 0.715160, 0.072169));		// Defined by sRGB gamut
//	return dot( x, float3(0.2989164, 0.5865990, 0.1144845) );	// NTSC - don't use this
}