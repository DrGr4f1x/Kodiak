// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from ParticleShaderStructs.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//
// Developed by Minigraph
//
// Original Author(s):  Julia Careaga
//                      James Stanard
//

#pragma once

namespace Kodiak
{

__declspec(align(16)) struct EmissionProperties
{
	DirectX::XMFLOAT3 LastEmitPosW;
	float EmitSpeed;
	DirectX::XMFLOAT3 EmitPosW;
	float FloorHeight;
	DirectX::XMFLOAT3 EmitDirW;
	float Restitution;
	DirectX::XMFLOAT3 EmitRightW;
	float EmitterVelocitySensitivity;
	DirectX::XMFLOAT3 EmitUpW;
	uint32_t MaxParticles;
	DirectX::XMFLOAT3 Gravity;
	uint32_t TextureID;
	DirectX::XMFLOAT3 EmissiveColor;
	float pad1;
	DirectX::XMUINT4 RandIndex[64];
};


EmissionProperties* CreateEmissionProperties();


struct ParticleSpawnData
{
	float AgeRate;
	float RotationSpeed;
	float StartSize;
	float EndSize;
	DirectX::XMFLOAT3 Velocity; float Mass;
	DirectX::XMFLOAT3 SpreadOffset; float Random;
	Color StartColor;
	Color EndColor;
};


struct ParticleMotion
{
	DirectX::XMFLOAT3 Position;
	float Mass;
	DirectX::XMFLOAT3 Velocity;
	float Age;
	float Rotation;
	uint32_t ResetDataIndex;
};


struct ParticleVertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT4 Color;
	float Size;
	uint32_t TextureID;
};


struct ParticleScreenData
{
	float Corner[2];
	float RcpSize[2];
	float Color[4];
	float Depth;
	float TextureIndex;
	float TextureLevel;
	uint32_t Bounds;
};

} // namespace Kodiak