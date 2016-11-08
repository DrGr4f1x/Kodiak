// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from SSAO.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#pragma once

#include "ComputeKernel.h"
#include "RenderThread.h"

namespace Kodiak
{

// Forward declarations
class Camera;
class ColorBuffer;
class ComputeCommandList;
class DepthBuffer;
class GraphicsCommandList;
class RenderTask;


class SSAO : public std::enable_shared_from_this<SSAO>
{
public:
	SSAO();

	void Initialize(uint32_t width, uint32_t height);

	// Feature toggles
	ThreadParameter<bool> Enable;
	ThreadParameter<bool> DebugDraw;
	ThreadParameter<bool> ComputeLinearDepth;

	// Render targets and buffers
	ThreadParameter<std::shared_ptr<ColorBuffer>> SsaoFullscreen;
	ThreadParameter<std::shared_ptr<ColorBuffer>> SceneColorBuffer;
	ThreadParameter<std::shared_ptr<DepthBuffer>> SceneDepthBuffer;
	ThreadParameter<std::shared_ptr<ColorBuffer>> LinearDepthBuffer;

	// Camera
	void SetCamera(std::shared_ptr<Camera> camera);

	// Render AO
	void Render(GraphicsCommandList& commandList);

private:
	void ComputeAO(ComputeCommandList& commandList, ComputeKernel& kernel, std::shared_ptr<ColorBuffer> destination,
		std::shared_ptr<ColorBuffer> depthBuffer, const float tanHalfFovH);

	void BlurAndUpsample(ComputeCommandList& commandList,
		std::shared_ptr<ColorBuffer> destination, std::shared_ptr<ColorBuffer> hiResDepth, std::shared_ptr<ColorBuffer> loResDepth,
		std::shared_ptr<ColorBuffer> interleavedAO, std::shared_ptr<ColorBuffer> highQualityAO, std::shared_ptr<ColorBuffer> hiResAO);

private:
	// Compute shader kernels
	ComputeKernel	m_depthPrepare1Cs;
	ComputeKernel	m_depthPrepare2Cs;
	ComputeKernel	m_render1Cs[4];
	ComputeKernel	m_render2Cs[4];
	ComputeKernel	m_blurUpsampleBlend[4][2];
	ComputeKernel	m_blurUpsampleFinal[4][2];
	ComputeKernel	m_linearizeDepthCs;
	ComputeKernel	m_debugSsaoCs;
	uint32_t						m_currentBlurUpsampleBlend{ 0 };
	uint32_t						m_currentBlurUpsampleFinal{ 0 };

	// Render targets and UAV buffers
	std::shared_ptr<ColorBuffer>	m_ssaoFullscreen;
	std::shared_ptr<ColorBuffer>	m_sceneColorBuffer;
	std::shared_ptr<DepthBuffer>	m_sceneDepthBuffer;
	std::shared_ptr<ColorBuffer>	m_linearDepth;

	// Internal render targets and UAV buffers
	std::shared_ptr<ColorBuffer>	m_depthDownsize1;
	std::shared_ptr<ColorBuffer>	m_depthDownsize2;
	std::shared_ptr<ColorBuffer>	m_depthDownsize3;
	std::shared_ptr<ColorBuffer>	m_depthDownsize4;
	std::shared_ptr<ColorBuffer>	m_depthTiled1;
	std::shared_ptr<ColorBuffer>	m_depthTiled2;
	std::shared_ptr<ColorBuffer>	m_depthTiled3;
	std::shared_ptr<ColorBuffer>	m_depthTiled4;
	std::shared_ptr<ColorBuffer>	m_aoMerged1;
	std::shared_ptr<ColorBuffer>	m_aoMerged2;
	std::shared_ptr<ColorBuffer>	m_aoMerged3;
	std::shared_ptr<ColorBuffer>	m_aoMerged4;
	std::shared_ptr<ColorBuffer>	m_aoHighQuality1;
	std::shared_ptr<ColorBuffer>	m_aoHighQuality2;
	std::shared_ptr<ColorBuffer>	m_aoHighQuality3;
	std::shared_ptr<ColorBuffer>	m_aoHighQuality4;
	std::shared_ptr<ColorBuffer>	m_aoSmooth1;
	std::shared_ptr<ColorBuffer>	m_aoSmooth2;
	std::shared_ptr<ColorBuffer>	m_aoSmooth3;

#if DX11
	// Semi-hacky way to set samplers
	Microsoft::WRL::ComPtr<ID3D11SamplerState>	m_linearBorderSampler;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>	m_linearClampSampler;
#endif

	// Camera
	// TODO Not threadsafe!!!
	std::shared_ptr<Camera> m_camera;

	// Parameters
	enum QualityLevel { kSsaoQualityLow, kSsaoQualityMedium, kSsaoQualityHigh, kSsaoQualityVeryHigh, kNumSsaoQualitySettings};
	QualityLevel	m_qualityLevel{ kSsaoQualityHigh };
	float			m_noiseFilterTolerance{ -3.0f }; // log10
	float			m_blurTolerance{ -5.0f }; // log10
	float			m_upsampleTolerance{ -7.0f };
	float			m_rejectionFalloff{ 2.5f };
	float			m_accentuation{ 0.1f };
	int32_t			m_hierarchyDepth{ 3 };

	// Pre-computed sample thicknesses
	float			m_sampleThickness[12];

	// Feature toggles
	bool			m_enabled{ true };
	bool			m_debugDraw{ false };
	bool			m_computeLinearZ{ true };
};

} // namespace Kodiak