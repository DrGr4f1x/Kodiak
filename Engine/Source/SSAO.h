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

namespace Kodiak
{

// Forward declarations
class ComputeKernel;
class RenderTask;


class SSAO : public std::enable_shared_from_this<SSAO>
{
public:
	void Initialize();

	std::shared_ptr<RenderTask> GetRenderTask();

	// Enable/disable SSAO
	bool GetEnabled() const { return m_enabled; }
	void SetEnabled(bool enabled);

	// Enable/disable debug mode
	bool GetDebugDraw() const { return m_debugDraw; }
	void SetDebugDraw(bool enabled);


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
};

} // namespace Kodiak