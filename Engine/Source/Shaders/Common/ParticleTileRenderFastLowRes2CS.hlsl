// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from ParticleTileRenderFastLowRes2CS.hlsl in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//
// Developed by Minigraph
//
// Original Author(s):  James Stanard 
//

#define LOW_RESOLUTION
#define DISABLE_DEPTH_TESTS
#define SUPPORT_TYPED_UAV_LOADS 1
#include "ParticleTileRenderCS.hlsl"