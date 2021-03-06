// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from ConvertHDRToDisplayPS.hlsl in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#include "ShaderUtility.hlsli"

Texture2D<float3> ColorTex : register(t0);
//Texture2D<float4> Overlay : register(t1);

struct PS_OUT
{
	float4 HdrOutput : SV_Target0;
#if _XBOX_ONE
	float4 LdrOutput : SV_Target1;
#endif
};

cbuffer CB0 : register(b0)
{
	float PaperWhite;
	float MaxBrightness;
	float ToeStrength;
	uint DebugMode;
}

float3 ConvertCS_709to2020(float3 RGB709)
{
	//return ConvertCS_XYZto2020(ConvertCS_709toXYZ(RGB709));
	static const float3x3 ConvMat =
	{
		0.62740192, 0.32929197, 0.0433061,
		0.06909549, 0.91954428, 0.01136023,
		0.01639371, 0.08802816, 0.89557813
	};
	return mul(ConvMat, RGB709);
}


PS_OUT main(float4 position : SV_Position)
{
	PS_OUT Out;

	//float4 UI = Overlay[(int2)position.xy];
	float3 HDR = ColorTex[(int2)position.xy];
	float3 SDR = ApplyToeRGB(ToneMapRGB(HDR), ToeStrength);

	// Better to blend in linear space (unlike the hardware compositor)
	//UI.rgb = SRGBToLinear(UI.rgb);

	// SDR was not explicitly clamped to [0, 1] on input, but it will be on output
	//SDR = saturate(SDR) * (1 - UI.a) + UI.rgb;

#if _XBOX_ONE
	// This will output Rec.709 Limited Range
	Out.LdrOutput = float4(ApplyColorProfile(SDR, DISPLAY_PLANE_FORMAT), 1.0f);
#endif

	HDR = ConvertCS_709to2020(HDR);
	//UI.rgb = ConvertCS_709to2020(UI.rgb) * PaperWhite;
	SDR = ConvertCS_709to2020(SDR) * PaperWhite;

	HDR = ApplyToeRGB(ToneMapRGB(HDR * ComputeHDRRescale(PaperWhite, MaxBrightness)), ToeStrength) * MaxBrightness;

	// Composite HDR buffer with UI
	//HDR = HDR * (1 - UI.a) + UI.rgb;

	float3 FinalOutput;
	switch (DebugMode)
	{
	case 0: FinalOutput = HDR; break;
	case 1: FinalOutput = SDR; break;
	default: FinalOutput = (position.x < 960 ? HDR : SDR); break;
	}

	// Current values are specified in nits.  Normalize to max specified brightness.
	Out.HdrOutput = float4(LinearToREC2084(FinalOutput / 10000.0), 1.0f);

	return Out;
}