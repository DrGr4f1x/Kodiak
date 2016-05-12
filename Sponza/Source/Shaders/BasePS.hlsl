Texture2D<float3> texDiffuse : register(t0);
Texture2D<float3> texSpecular : register(t1);
Texture2D<float3> texNormal : register(t2);

Texture2D<float> texSSAO : register(t3);

SamplerState AnisotropicWrap : register(s0);

cbuffer PerMaterialData : register(b2)
{
	float3 sunDirection;
	float3 sunColor;
	float3 ambientColor;
	uint _pad;
	float shadowTexelSize;
};


struct PixelShaderInput
{
	float4 position : SV_Position;
	float2 texcoord0 : texcoord0;
	float3 viewDir : texcoord1;
	//float3 shadowCoord : texcoord2;
	float3 normal : normal;
	float3 tangent : tangent;
	float3 bitangent : bitangent;
};


// Apply fresnel to modulate the specular albedo
void FSchlick(inout float3 specular, inout float3 diffuse, float3 lightDir, float3 halfVec)
{
	float fresnel = pow(1.0f - saturate(dot(lightDir, halfVec)), 5.0f);
	specular = lerp(specular, 1.0f, fresnel);
	diffuse = lerp(diffuse, 0.0f, fresnel);
}


float3 ApplyAmbientLight(
	float3 diffuse,		// Diffuse albedo
	float ao,			// Pre-computed ambient-occlusion
	float3 lightColor	// Radiance of ambient light
	)
{
	return ao * diffuse * lightColor;
}


//float GetShadow(float3 ShadowCoord)
//{
//#ifdef SINGLE_SAMPLE
//	float result = ShadowMap.SampleCmpLevelZero(ShadowSampler, ShadowCoord.xy, ShadowCoord.z);
//#else
//	const float Dilation = 2.0;
//	float d1 = Dilation * ShadowTexelSize * 0.125;
//	float d2 = Dilation * ShadowTexelSize * 0.875;
//	float d3 = Dilation * ShadowTexelSize * 0.625;
//	float d4 = Dilation * ShadowTexelSize * 0.375;
//	float result = (
//		2.0 * texShadow.SampleCmpLevelZero(shadowSampler, ShadowCoord.xy, ShadowCoord.z) +
//		texShadow.SampleCmpLevelZero(shadowSampler, ShadowCoord.xy + float2(-d2, d1), ShadowCoord.z) +
//		texShadow.SampleCmpLevelZero(shadowSampler, ShadowCoord.xy + float2(-d1, -d2), ShadowCoord.z) +
//		texShadow.SampleCmpLevelZero(shadowSampler, ShadowCoord.xy + float2(d2, -d1), ShadowCoord.z) +
//		texShadow.SampleCmpLevelZero(shadowSampler, ShadowCoord.xy + float2(d1, d2), ShadowCoord.z) +
//		texShadow.SampleCmpLevelZero(shadowSampler, ShadowCoord.xy + float2(-d4, d3), ShadowCoord.z) +
//		texShadow.SampleCmpLevelZero(shadowSampler, ShadowCoord.xy + float2(-d3, -d4), ShadowCoord.z) +
//		texShadow.SampleCmpLevelZero(shadowSampler, ShadowCoord.xy + float2(d4, -d3), ShadowCoord.z) +
//		texShadow.SampleCmpLevelZero(shadowSampler, ShadowCoord.xy + float2(d3, d4), ShadowCoord.z)
//		) / 10.0;
//#endif
//	return result * result;
//}


float GetShadow(float3 shadowCoord) { return 1.0f; }


float3 ApplyDirectionalLight(
	float3 diffuseColor,	// Diffuse albedo
	float3 specularColor,	// Specular albedo
	float specularMask,		// Where is it shiny or dingy?
	float gloss,			// Specular power
	float3 normal,			// World-space normal
	float3 viewDir,			// World-space vector from eye to point
	float3 lightDir,		// World-space vector from point to light
	float3 lightColor,		// Radiance of directional light
	float3 shadowCoord		// Shadow coordinate (Shadow map UV & light-relative Z)
	)
{
	// normal and lightDir are assumed to be pre-normalized
	float nDotL = dot(normal, lightDir);
	if (nDotL <= 0.0f)
	{
		return float3(0.0f, 0.0f, 0.0f);
	}

	// viewDir is also assumed normalized
	float3 halfVec = normalize(lightDir - viewDir);
	float nDotH = max(0.0f, dot(halfVec, normal));

	FSchlick(diffuseColor, specularColor, lightDir, halfVec);

	float specularFactor = specularMask * pow(nDotH, gloss) * (gloss + 2.0f) / 8.0f;

	float shadow = GetShadow(shadowCoord);

	return shadow * nDotL * lightColor * (diffuseColor + specularFactor * specularColor);
}


void AntiAliasSpecular(inout float3 texNormal, inout float gloss)
{
	float norm = length(texNormal);
	texNormal /= norm;
	gloss = lerp(1, gloss, norm);
}


float3 main(PixelShaderInput input) : SV_Target0
{
	float3 diffuseAlbedo = texDiffuse.Sample(AnisotropicWrap, input.texcoord0);
	float3 specularAlbedo = float3(0.56f, 0.56f, 0.56f);//float3(1.0, 0.71, 0.29);
	float specularMask = texSpecular.Sample(AnisotropicWrap, input.texcoord0).g;
	float3 normal = texNormal.Sample(AnisotropicWrap, input.texcoord0) * 2.0f - 1.0f;
	float gloss = 128.0f;
	float ao = texSSAO[uint2(input.position.xy)];
	//float ao = 1.0f;
	float3 viewDir = normalize(input.viewDir);

	AntiAliasSpecular(normal, gloss);

	float3x3 tbn = float3x3(normalize(input.tangent), normalize(input.bitangent), normalize(input.normal));
	normal = normalize(mul(normal, tbn));

	float3 ambientContribution = ApplyAmbientLight(diffuseAlbedo, ao, ambientColor);

	float3 sunlightContribution = ApplyDirectionalLight(
		diffuseAlbedo, 
		specularAlbedo, 
		specularMask, 
		gloss, 
		normal, 
		viewDir, 
		sunDirection,
		sunColor,
		float3(0.0f, 0.0f, 0.0f) /*input.shadowCoord*/);

	return ambientContribution + sunlightContribution;
}