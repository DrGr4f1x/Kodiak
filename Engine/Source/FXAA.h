// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from FXAA.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#pragma once

#include "RenderThread.h"

namespace Kodiak
{

// Forward declarations
class ColorBuffer;
class ComputeKernel;
class GraphicsCommandList;
class IndirectArgsBuffer;
class StructuredBuffer;
class TypedBuffer;

class FXAA
{
public:
	FXAA();

	void Initialize(uint32_t width, uint32_t height);

	// Render targets and buffers
	ThreadParameter<std::shared_ptr<ColorBuffer>> SceneColorBuffer;
	ThreadParameter<std::shared_ptr<ColorBuffer>> PostEffectsBuffer;
	ThreadParameter<std::shared_ptr<ColorBuffer>> LumaBuffer;

	// Feature toggles
	ThreadParameter<bool> UsePrecomputedLuma;
	ThreadParameter<bool> DebugDraw;

	// Parameters
	ThreadParameter<float> ContrastThreshold;
	ThreadParameter<float> SubpixelRemoval;

	// Render FXAA
	void Render(GraphicsCommandList* commandList);

private:
	// Compute kernels
	std::shared_ptr<ComputeKernel>			m_pass1HdrCs;
	std::shared_ptr<ComputeKernel>			m_pass1LdrCs;
	std::shared_ptr<ComputeKernel>			m_resolveWorkCs;
	std::shared_ptr<ComputeKernel>			m_pass2HDebugCs;
	std::shared_ptr<ComputeKernel>			m_pass2HCs;
	std::shared_ptr<ComputeKernel>			m_pass2VDebugCs;
	std::shared_ptr<ComputeKernel>			m_pass2VCs;

	// Render targets and UAV buffers
	std::shared_ptr<ColorBuffer>			m_sceneColorBuffer;
	std::shared_ptr<ColorBuffer>			m_postEffectsBuffer;
	std::shared_ptr<ColorBuffer>			m_lumaBuffer;

	// Internal render targets and UAV buffers
	std::shared_ptr<StructuredBuffer>		m_fxaaWorkQueueH;
	std::shared_ptr<StructuredBuffer>		m_fxaaWorkQueueV;
	std::shared_ptr<TypedBuffer>			m_fxaaColorQueueH;
	std::shared_ptr<TypedBuffer>			m_fxaaColorQueueV;
	std::shared_ptr<IndirectArgsBuffer>		m_indirectParameters;

	// Feature toggles
	bool m_usePreComputedLuma{ true };
	bool m_debugDraw{ false };

	// Parameters
	float m_contrastThreshold{ 0.175f };
	float m_subpixelRemoval{ 0.5f };

};

} // namespace Kodiak