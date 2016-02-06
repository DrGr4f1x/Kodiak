// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include <concurrent_queue.h>
#include <map>

#pragma once

namespace Kodiak
{

// Forward declarations
namespace RenderThread { class Camera; }
class Camera;
class ConstantBuffer;
class GraphicsCommandList;
class GraphicsPSO;
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
	void Render(GraphicsCommandList& commandList);

	void SetCamera(std::shared_ptr<Camera> camera);

	// To be called from the render thread only
	void SetCameraDeferred(std::shared_ptr<Camera> camera);
	void AddStaticModelDeferred(std::shared_ptr<RenderThread::StaticModelData> model);
	void RemoveStaticModelDeferred(std::shared_ptr<RenderThread::StaticModelData> model);

private:
	void Initialize();

private:
	std::shared_ptr<ConstantBuffer>		m_perViewConstantBuffer;
	std::shared_ptr<GraphicsPSO>		m_pso;

#if defined(DX12)
	std::shared_ptr<RootSignature>		m_rootSignature;
#endif

	struct PerViewConstants
	{
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	} m_perViewConstants;

	// Scene camera
	std::shared_ptr<RenderThread::Camera> m_camera;

	// HACK
	DirectX::XMFLOAT4X4 m_modelTransform;

	// Maps from static model pointers into the m_staticModels list for faster adds/removes
	std::map<std::shared_ptr<RenderThread::StaticModelData>, size_t>	m_staticModelMap;
	// Main list of static models for culling and rendering
	std::vector<std::shared_ptr<RenderThread::StaticModelData>>			m_staticModels;
};


} // namespace Kodiak
