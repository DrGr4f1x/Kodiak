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

#include "RenderThread.h"

namespace Kodiak
{

// Forward declarations
class Camera;
class ColorBuffer;
class ComputeKernel;
class DepthBuffer;
class GraphicsCommandList;
class RenderTask;
namespace RenderThread { class Camera; }

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
	void Render(GraphicsCommandList* commandList);

private:
	// Compute shader kernels
	std::shared_ptr<ComputeKernel>	m_depthPrepare1Cs;
	std::shared_ptr<ComputeKernel>	m_depthPrepare2Cs;
	std::shared_ptr<ComputeKernel>	m_render1Cs;
	std::shared_ptr<ComputeKernel>	m_render2Cs;
	std::shared_ptr<ComputeKernel>	m_blurUpsampleBlend[2];
	std::shared_ptr<ComputeKernel>	m_blurUpsampleFinal[2];
	std::shared_ptr<ComputeKernel>	m_linearizeDepthCs;
	std::shared_ptr<ComputeKernel>	m_debugSsaoCs;

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

	// Camera
	std::shared_ptr<RenderThread::Camera> m_camera;

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