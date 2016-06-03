// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from PostEffects.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#pragma once

#include "RenderThread.h"

namespace Kodiak
{

// Forward declarations
class ColorBuffer;
class ComputeCommandList;
class ComputeKernel;
class GraphicsCommandList;
class StructuredBuffer;

class PostProcessing
{
public:
	PostProcessing();

	void Initialize(uint32_t width, uint32_t height);

	// Render targets and buffers
	ThreadParameter<std::shared_ptr<ColorBuffer>> SceneColorBuffer;

	// Feature toggles
	ThreadParameter<bool> EnableHDR;
	ThreadParameter<bool> DebugDrawSSAO;
	ThreadParameter<bool> EnableBloom;
	ThreadParameter<bool> EnableAdaptation;

	// Parameters
	ThreadParameter<float> BloomThreshold;
	ThreadParameter<float> Exposure;
	ThreadParameter<float> PeakIntensity;

	// Render post effects
	void Render(GraphicsCommandList* commandList);

private:
	void ProcessHDR(ComputeCommandList* commandList);
	void ProcessLDR(ComputeCommandList* commandList);

	void GenerateBloom(ComputeCommandList* commandList);

private:
	// Compute kernels
	std::shared_ptr<ComputeKernel>	m_bloomExtractAndDownsampleHdrCs;
	std::shared_ptr<ComputeKernel>	m_bloomExtractAndDownsampleLdrCs;

	// Render targets and UAV buffers
	std::shared_ptr<ColorBuffer> m_sceneColorBuffer;

	// Internal render targets and UAV buffers
	std::shared_ptr<ColorBuffer> m_bloomUAV1[2];	//  640x384 (1/3)
	std::shared_ptr<ColorBuffer> m_bloomUAV2[2];	//  320x192 (1/6)
	std::shared_ptr<ColorBuffer> m_bloomUAV3[2];	//  160x96  (1/12)
	std::shared_ptr<ColorBuffer> m_bloomUAV4[2];    //  80x48   (1/24)
	std::shared_ptr<ColorBuffer> m_bloomUAV5[2];    //  40x24   (1/48)
	std::shared_ptr<ColorBuffer> m_lumaBuffer;
	std::shared_ptr<ColorBuffer> m_lumaLR;
	std::shared_ptr<StructuredBuffer> m_exposureBuffer;

	// Feature toggles
	bool m_enableHDR{ true };
	bool m_debugDrawSSAO{ false };
	bool m_enableBloom{ true };
	bool m_enableAdaptation{ true };

	// Parameters
	float m_bloomThreshold{ 1.0f };
	float m_exposure{ 4.0f };
	float m_peakIntensity{ 16.0f };

	// Constants
	const uint32_t m_bloomWidth{ 640 };
	const uint32_t m_bloomHeight{ 384 };
	const float m_initialMinLog{ -12.0f };
	const float m_initialMaxLog{ 4.0f };
};

} // namespace Kodiak