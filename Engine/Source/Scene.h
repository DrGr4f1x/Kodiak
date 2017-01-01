// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include <concurrent_queue.h>
#include "RenderThread.h"

namespace Kodiak
{

// Forward declarations
class Camera;
class ConstantBuffer;
class ColorBuffer;
class GraphicsCommandList;
class GraphicsPSO;
class RenderPass;
class ShadowBuffer;
class ShadowCamera;
class StaticModel;
#if defined(DX12)
class RootSignature;
#endif


namespace RenderThread
{
struct StaticModelData;
} // namespace RenderThread


class Scene : public std::enable_shared_from_this<Scene>
{
	friend class Renderer;

public:
	Scene();

	void AddStaticModel(std::shared_ptr<StaticModel> model);

	void Update(GraphicsCommandList& commandList);
	void Render(std::shared_ptr<RenderPass> renderPass, GraphicsCommandList& commandList);

	// TODO: This is hacky
	void RenderShadows(std::shared_ptr<RenderPass> renderPass, GraphicsCommandList& commandList);

	void SetCamera(std::shared_ptr<Camera> camera);

	// TODO: This is hacky
	void SetShadowBuffer(std::shared_ptr<ShadowBuffer> buffer);
	void SetShadowCamera(std::shared_ptr<ShadowCamera> camera);

	// To be called from the render thread only
	void SetCameraDeferred(std::shared_ptr<Camera> camera);
	void AddStaticModelDeferred(std::shared_ptr<RenderThread::StaticModelData> model);
	void RemoveStaticModelDeferred(std::shared_ptr<RenderThread::StaticModelData> model);

	// TODO: Super-hacky way to ram sampler states into the engine
	void BindSamplerStates(GraphicsCommandList& commandList);

#if DX11
	ThreadParameter<std::shared_ptr<ColorBuffer>> SsaoFullscreen;
#endif

private:
	void Initialize();

private:
	std::shared_ptr<ConstantBuffer>		m_perViewConstantBuffer;
	std::shared_ptr<GraphicsPSO>		m_pso;

#if DX12
	std::shared_ptr<RootSignature>		m_rootSignature;
#endif

	// TODO allocation/alignment problem with this struct
	struct PerViewConstants
	{
		Math::Matrix4 viewProjection;
		Math::Matrix4 modelToShadow;
		Math::Vector3 viewPosition;
	} m_perViewConstants;

	// Scene camera
	// TODO: this is not thread-safe!
	std::shared_ptr<Camera> m_camera;

	// HACK
#if DX11
	Microsoft::WRL::ComPtr<ID3D11SamplerState>	m_samplerState;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>  m_shadowSamplerState;
	std::shared_ptr<ColorBuffer>				m_ssaoFullscreen;
#endif

	// TODO: hacks
	std::shared_ptr<ShadowBuffer> m_shadowBuffer;
	std::shared_ptr<ShadowCamera> m_shadowCamera;

	// Maps from static model pointers into the m_staticModels list for faster adds/removes
	std::map<std::shared_ptr<RenderThread::StaticModelData>, size_t>	m_staticModelMap;
	// Main list of static models for culling and rendering
	std::vector<std::shared_ptr<RenderThread::StaticModelData>>			m_staticModels;
};


} // namespace Kodiak
