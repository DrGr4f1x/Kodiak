// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include <concurrent_queue.h>

#pragma once

namespace Kodiak
{

// Forward declarations
namespace RenderThread { class Camera; }
class Camera;
class ConstantBuffer;
class GraphicsCommandList;
class GraphicsPSO;
class Model;
class StaticModel;
#if defined(DX12)
class RootSignature;
#endif

namespace RenderThread
{
struct StaticModelData;
} // namespace RenderThread

class Scene
{
public:
	Scene();

	void AddModel(std::shared_ptr<Model> model);
	void AddStaticModel(std::shared_ptr<StaticModel> model);

	void Update(GraphicsCommandList& commandList);
	void Render(GraphicsCommandList& commandList);

	void SetCamera(std::shared_ptr<Camera> camera);

private:
	void Initialize();

	void AddStaticModelDeferred(std::shared_ptr<RenderThread::StaticModelData> model);

private:
	std::vector<std::shared_ptr<Model>> m_models;
	std::shared_ptr<ConstantBuffer>		m_perViewConstantBuffer;
	std::shared_ptr<ConstantBuffer>		m_perObjectConstantBuffer;
	std::shared_ptr<GraphicsPSO>		m_pso;

#if defined(DX12)
	std::shared_ptr<RootSignature>		m_rootSignature;
#endif

	struct PerViewConstants
	{
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	} m_perViewConstants;

	struct PerObjectConstants
	{
		DirectX::XMFLOAT4X4 model;
	} m_perObjectConstants;

	// Scene camera
	std::shared_ptr<RenderThread::Camera> m_camera;

	// HACK
	DirectX::XMFLOAT4X4 m_modelTransform;

	// Deferred add lists
	concurrency::concurrent_queue<std::shared_ptr<RenderThread::StaticModelData>> m_deferredAddModels;
};


} // namespace Kodiak
