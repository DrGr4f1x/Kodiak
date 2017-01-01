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

#include "ColorBuffer.h"
#include "ComputeKernel.h"
#include "GpuBuffer.h"
#include "RenderThread.h"

namespace Kodiak
{

// Forward declarations
class ByteAddressBuffer;
class ColorBuffer;
class ComputeCommandList;
class GraphicsCommandList;
class GraphicsPSO;
class StructuredBuffer;
class Texture;


class PostProcessing
{
public:
	PostProcessing();

	void Initialize(uint32_t width, uint32_t height);

	// Render targets and buffers
	ThreadParameter<std::shared_ptr<ColorBuffer>> SceneColorBuffer;
	ThreadParameter<std::shared_ptr<ColorBuffer>> PostEffectsBuffer;
	ThreadParameter<std::shared_ptr<ColorBuffer>> LumaBuffer;

	// Feature toggles
	ThreadParameter<bool> EnableHDR;
	ThreadParameter<bool> DebugDrawSSAO;
	ThreadParameter<bool> EnableBloom;
	ThreadParameter<bool> EnableAdaptation;
	ThreadParameter<bool> HighQualityBloom;
	ThreadParameter<bool> DrawHistogram;
	ThreadParameter<bool> DebugLuminance;

	// Parameters
	ThreadParameter<float> BloomThreshold;
	ThreadParameter<float> Exposure;
	ThreadParameter<float> BloomUpsampleFactor;
	ThreadParameter<float> BloomStrength;
	ThreadParameter<float> LogLumaConstant;
	ThreadParameter<float> TargetLuminance;
	ThreadParameter<float> AdaptationRate;
	ThreadParameter<float> MinExposure;
	ThreadParameter<float> MaxExposure;
	ThreadParameter<float> ToeStrength;

	// Render post effects
	void Render(GraphicsCommandList& commandList);
	
	void FinalizePostProcessing(GraphicsCommandList& commandList);
	void RenderHistogram(GraphicsCommandList& commandList);

private:
	void ProcessHDR(ComputeCommandList& commandList);
	void ProcessLDR(ComputeCommandList& commandList);

	void GenerateBloom(ComputeCommandList& commandList);
	void ExtractLuma(ComputeCommandList& commandList);
	void BlurBuffer(ComputeCommandList& commandList, uint32_t blurKernelIndex, ColorBuffer buffer[2], ColorBuffer& lowerResBuf,
		uint32_t bufferWidth, uint32_t bufferHeight, float upsampleBlendFactor);
	void UpdateExposure(ComputeCommandList& commandList);

#if DX11
	void InitializeSamplers();
#endif


private:
	// Compute kernels
	ComputeKernel	m_bloomExtractAndDownsampleHdrCs;
	ComputeKernel	m_bloomExtractAndDownsampleLdrCs;
	ComputeKernel	m_extractLumaCs;
	ComputeKernel	m_downsampleBloom4Cs;
	ComputeKernel	m_downsampleBloom2Cs;
	ComputeKernel	m_blurCs[5];
	ComputeKernel	m_upsampleAndBlurCs[5];
	ComputeKernel	m_toneMapCs;
	ComputeKernel	m_toneMapHdrCs;
	ComputeKernel	m_generateHistogramCs;
	ComputeKernel	m_adaptExposureCs;
	ComputeKernel	m_debugDrawHistogramCs;
	ComputeKernel	m_debugLuminanceHdrCs;
	ComputeKernel	m_debugLuminanceLdrCs;
	ComputeKernel	m_copyPostToSceneCs;

	// Render targets and UAV buffers
	std::shared_ptr<ColorBuffer>	m_sceneColorBuffer;
	std::shared_ptr<ColorBuffer>	m_postEffectsBuffer;
	std::shared_ptr<ColorBuffer>	m_lumaBuffer;

	// Internal render targets and UAV buffers
	ColorBuffer			m_bloomUAV1[2];	//  640x384 (1/3)
	ColorBuffer			m_bloomUAV2[2];	//  320x192 (1/6)
	ColorBuffer			m_bloomUAV3[2];	//  160x96  (1/12)
	ColorBuffer			m_bloomUAV4[2]; //  80x48   (1/24)
	ColorBuffer			m_bloomUAV5[2]; //  40x24   (1/48)
	ColorBuffer			m_lumaLR;
	StructuredBuffer	m_exposureBuffer;
	ByteAddressBuffer	m_histogram;

	// Default black texture
	std::shared_ptr<Texture>			m_defaultBlackTexture;

	// Feature toggles
	bool m_enableHDR{ true };
	bool m_debugDrawSSAO{ false };
	bool m_enableBloom{ true };
	bool m_enableAdaptation{ true };
	bool m_highQualityBloom{ true };
	bool m_drawHistogram{ false };
	bool m_debugLuminance{ false };

	// Parameters
	float m_bloomThreshold{ 1.0f };
	float m_exposure{ 2.0f };
	float m_bloomUpsampleFactor{ 0.65f };
	float m_bloomStrength{ 0.1f };
	float m_logLumaConstant{ 4.0f };
	float m_targetLuminance{ 0.08f };
	float m_adaptationRate{ 0.05f };
	float m_minExposure{ 1.0f / 64.0f };
	float m_maxExposure{ 64.0f };
	float m_toeStrength{ 0.01f };

	// Constants
	const uint32_t m_bloomWidth{ 640 };
	const uint32_t m_bloomHeight{ 384 };
	const float m_initialMinLog{ -12.0f };
	const float m_initialMaxLog{ 4.0f };

	// Internal state
	uint32_t m_width{ 1 };
	uint32_t m_height{ 1 };

#if DX11
	Microsoft::WRL::ComPtr<ID3D11SamplerState>	m_linearClampSampler;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>	m_linearBorderSampler;
#endif
};

} // namespace Kodiak