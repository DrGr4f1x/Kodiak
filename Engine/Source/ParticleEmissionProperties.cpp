// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from ParticleEmissionProperties.cpp in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//
// Developed by Minigraph
//
// Original Author(s):  Julia Careaga
//

#include "Stdafx.h"

#include "ParticleShaderStructs.h"

using namespace DirectX;
using namespace Math;

namespace Kodiak
{

EmissionProperties* CreateEmissionProperties()
{
	EmissionProperties* emitProps = new EmissionProperties;
	ZeroMemory(emitProps, sizeof(*emitProps));
	emitProps->EmitPosW = emitProps->LastEmitPosW = XMFLOAT3(0.0, 0.0, 0.0);
	emitProps->EmitDirW = XMFLOAT3(0.0f, 0.0f, 1.0f);
	emitProps->EmitRightW = XMFLOAT3(1.0f, 0.0f, 0.0f);
	emitProps->EmitUpW = XMFLOAT3(0.0f, 1.0f, 0.0f);
	emitProps->Restitution = 0.6f;
	emitProps->FloorHeight = -0.7f;
	emitProps->EmitSpeed = 1.0f;
	emitProps->Gravity = XMFLOAT3(0.0f, -5.0f, 0.0f);
	emitProps->MaxParticles = 500;
	return emitProps;
};

} // namespace Kodiak