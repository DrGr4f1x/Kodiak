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
class ByteAddressBuffer;
class ColorBuffer;
class ComputeCommandList;
class ComputeKernel;
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

	// Feature toggles
	ThreadParameter<bool> EnableHDR;
	ThreadParameter<bool> DebugDrawSSAO;
	ThreadParameter<bool> EnableBloom;
	ThreadParameter<bool> EnableAdaptation;
	ThreadParameter<bool> HighQualityBloom;
	ThreadParameter<bool> ToneMapOnlyLuma;

	// Parameters
	ThreadParameter<float> BloomThreshold;
	ThreadParameter<float> Exposure;
	ThreadParameter<float> PeakIntensity;
	ThreadParameter<float> BloomUpsampleFactor;
	ThreadParameter<float> BloomStrength;
	ThreadParameter<float> LogLumaConstant;
	ThreadParameter<float> TargetLuminance;
	ThreadParameter<float> AdaptationRate;
	ThreadParameter<float> MinExposure;
	ThreadParameter<float> MaxExposure;

	// Render post effects
	void Render(GraphicsCommandList* commandList);

private:
	void ProcessHDR(ComputeCommandList* commandList);
	void ProcessLDR(ComputeCommandList* commandList);

	void GenerateBloom(ComputeCommandList* commandList);
	void ExtractLuma(ComputeCommandList* commandList);
	void BlurBuffer(ComputeCommandList* commandList, uint32_t blurKernelIndex, std::shared_ptr<ColorBuffer> buffer[2], std::shared_ptr<ColorBuffer> lowerResBuf,
		uint32_t bufferWidth, uint32_t bufferHeight, float upsampleBlendFactor);
	void UpdateExposure(ComputeCommandList* commandList);

#if DX11
	void InitializeSamplers();
#endif


private:
	// Compute kernels
	std::shared_ptr<ComputeKernel>	m_bloomExtractAndDownsampleHdrCs;
	std::shared_ptr<ComputeKernel>	m_bloomExtractAndDownsampleLdrCs;
	std::shared_ptr<ComputeKernel>	m_extractLumaCs;
	std::shared_ptr<ComputeKernel>	m_downsampleBloom4Cs;
	std::shared_ptr<ComputeKernel>	m_downsampleBloom2Cs;
	std::shared_ptr<ComputeKernel>	m_blurCs[5];
	std::shared_ptr<ComputeKernel>	m_upsampleAndBlurCs[5];
	std::shared_ptr<ComputeKernel>	m_toneMapCs;
	std::shared_ptr<ComputeKernel>	m_toneMap2Cs;
	std::shared_ptr<ComputeKernel>	m_generateHistogramCs;
	std::shared_ptr<ComputeKernel>	m_adaptExposureCs;

	// Graphics shaders
	std::shared_ptr<GraphicsPSO>	m_copyPSO;

	// Render targets and UAV buffers
	std::shared_ptr<ColorBuffer>	m_sceneColorBuffer;

	// Internal render targets and UAV buffers
	std::shared_ptr<ColorBuffer>		m_sceneColorCopy;
	std::shared_ptr<ColorBuffer>		m_bloomUAV1[2];	//  640x384 (1/3)
	std::shared_ptr<ColorBuffer>		m_bloomUAV2[2];	//  320x192 (1/6)
	std::shared_ptr<ColorBuffer>		m_bloomUAV3[2];	//  160x96  (1/12)
	std::shared_ptr<ColorBuffer>		m_bloomUAV4[2];    //  80x48   (1/24)
	std::shared_ptr<ColorBuffer>		m_bloomUAV5[2];    //  40x24   (1/48)
	std::shared_ptr<ColorBuffer>		m_lumaBuffer;
	std::shared_ptr<ColorBuffer>		m_lumaLR;
	std::shared_ptr<StructuredBuffer>	m_exposureBuffer;
	std::shared_ptr<ByteAddressBuffer>	m_histogram;

	// Default black texture
	std::shared_ptr<Texture>			m_defaultBlackTexture;

	// Feature toggles
	bool m_enableHDR{ true };
	bool m_debugDrawSSAO{ false };
	bool m_enableBloom{ true };
	bool m_enableAdaptation{ true };
	bool m_highQualityBloom{ true };
	bool m_toneMapOnlyLuma{ false };

	// Parameters
	float m_bloomThreshold{ 1.0f };
	float m_exposure{ 4.0f };
	float m_peakIntensity{ 16.0f };
	float m_bloomUpsampleFactor{ 0.65f };
	float m_bloomStrength{ 0.1f };
	float m_logLumaConstant{ 4.0f };
	float m_targetLuminance{ 0.4f };
	float m_adaptationRate{ 0.05f };
	float m_minExposure{ 1.0f };
	float m_maxExposure{ 8.0f };

	// Constants
	const uint32_t m_bloomWidth{ 640 };
	const uint32_t m_bloomHeight{ 384 };
	const float m_initialMinLog{ -12.0f };
	const float m_initialMaxLog{ 4.0f };
#if DX11
	const bool m_copySceneColor{ true };
#else
	const bool m_copySceneColor{ false };
#endif

	// Internal state
	uint32_t m_width{ 1 };
	uint32_t m_height{ 1 };

#if DX11
	Microsoft::WRL::ComPtr<ID3D11SamplerState>	m_linearClampSampler;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>	m_linearBorderSampler;
#endif
};

} // namespace Kodiak