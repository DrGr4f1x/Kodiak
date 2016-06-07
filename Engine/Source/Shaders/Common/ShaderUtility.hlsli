// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from ShaderUtility.hlsl in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//


// Encodes a smooth logarithmic gradient for even distribution of precision natural to vision
float LinearToLogLuminance(float x, float gamma = 4.0)
{
	return log2(lerp(1, exp2(gamma), x)) / gamma;
}


// This assumes the default color gamut found in sRGB and REC709.  The color primaries determine these
// coefficients.  Note that this operates on linear values, not gamma space.
float RGBToLuminance(float3 x)
{
	return dot(x, float3(0.212671, 0.715160, 0.072169));		// Defined by sRGB gamut
//	return dot( x, float3(0.2989164, 0.5865990, 0.1144845) );	// NTSC - don't use this
}


// Assumes the "white point" is 1.0.  Prescale your HDR values if otherwise.  'E' affects the rate
// at which colors blow out to white.
float3 ToneMap(float3 hdr, float E = 4.0)
{
	return (1 - exp2(-E * hdr)) / (1 - exp2(-E));
}


// This variant rescales only the luminance of the color to fit in the [0, 1] range while preserving hue.
float3 ToneMap2(float3 hdr, float E = 4.0)
{
	float luma = RGBToLuminance(hdr);
	return hdr * (1 - exp2(-E * luma)) / (1 - exp2(-E)) / (luma + 0.0001);
}


float ToneMapLuma(float Luma, float E = 4.0)
{
	return (1 - exp2(-E * Luma)) / (1 - exp2(-E));
}


// This is the same as above, but converts the linear luminance value to a more subjective "perceived luminance",
// which could be called the Log-Luminance.
float RGBToLogLuminance(float3 x, float gamma = 4.0)
{
	return LinearToLogLuminance(RGBToLuminance(x), gamma);
}