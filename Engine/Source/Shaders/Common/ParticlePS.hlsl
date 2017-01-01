// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from ParticlePS.hlsl in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//
// Developed by Minigraph
//
// Original Author(s):  Julia Careaga
//                      James Stanard 
//

#include "ParticleUpdateCommon.hlsli"
#include "ParticleUtility.hlsli"

Texture2DArray<float4> ColorTex : register(t1);
Texture2D<float> LinearDepthTex : register(t2);

float4 main(ParticleVertexOutput input) : SV_Target0
{
	float3 uv = float3(input.TexCoord.xy, input.TexID);
	float4 TextureColor = ColorTex.Sample(LinearBorderSampler, uv);
	TextureColor.a *= saturate(1000.0 * (LinearDepthTex[(uint2)input.Pos.xy] - input.LinearZ));
	TextureColor.rgb *= TextureColor.a;
	return TextureColor * input.Color;
}