// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from ParticleEffectProperties.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//
// Developed by Minigraph
//
// Original Author(s):  Julia Careaga
//                      James Stanard
//

#pragma once

#include "ParticleShaderStructs.h"
#include "Color.h"


namespace Kodiak
{


struct ParticleEffectProperties
{
	ParticleEffectProperties()
	{
		using namespace DirectX;
		using namespace Math;

		ZeroMemory(this, sizeof(*this));
		MinStartColor = Color(0.8f, 0.8f, 1.0f);
		MaxStartColor = Color(0.9f, 0.9f, 1.0f);
		MinEndColor = Color(1.0f, 1.0f, 1.0f);
		MaxEndColor = Color(1.0f, 1.0f, 1.0f);
		EmitProperties = *CreateEmissionProperties(); //Properties passed to the shader
		EmitRate = 200;
		LifeMinMax = XMFLOAT2(1.0f, 2.0f);
		MassMinMax = XMFLOAT2(0.5f, 1.0f);
		Size = Vector4(0.07f, 0.7f, 0.8f, 0.8f); // (Start size min, Start size max, End size min, End size max) 		
		Spread = XMFLOAT3(0.5f, 1.5f, 0.1f);
		TexturePath = "Resources/Textures/sparkTex.dds";
		TotalActiveLifetime = 20.0;
		Velocity = Vector4(0.5, 3.0, -0.5, 3.0); // (X velocity min, X velocity max, Y velocity min, Y velocity max)
	};


	Color MinStartColor;
	Color MaxStartColor;
	Color MinEndColor;
	Color MaxEndColor;
	EmissionProperties  EmitProperties;
	float EmitRate;
	DirectX::XMFLOAT2 LifeMinMax;
	DirectX::XMFLOAT2 MassMinMax;
	Math::Vector4 Size;
	DirectX::XMFLOAT3 Spread;
	std::string TexturePath;
	float TotalActiveLifetime;
	Math::Vector4 Velocity;
};


} // namespace Kodiak