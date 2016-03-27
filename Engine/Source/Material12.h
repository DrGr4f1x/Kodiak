// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include <ppltasks.h>

namespace Kodiak
{

// Forward declarations
class Effect;
class GraphicsCommandList;
class GraphicsPSO;
class MaterialParameter;
class MaterialResource;
class RenderPass;
class RootSignature;
struct ShaderConstantBufferDesc;

namespace RenderThread
{
struct MaterialData;
}


class Material : public std::enable_shared_from_this<Material>
{

public:
	Material();
	Material(const std::string& name);

	void SetName(const std::string& name) { m_name = name; }
	const std::string& GetName() const { return m_name; }

	void SetEffect(std::shared_ptr<Effect> effect);
	void SetRenderPass(std::shared_ptr<RenderPass> pass);

	std::shared_ptr<MaterialParameter> GetParameter(const std::string& name);
	std::shared_ptr<MaterialResource> GetResource(const std::string& name);

	std::shared_ptr<RenderThread::MaterialData> GetRenderThreadData() { return m_renderThreadData; }

	std::shared_ptr<Material> Clone();

	concurrency::task<void> prepareTask;

private:
	void CreateRenderThreadData();

private:
	std::string						m_name;

	std::shared_ptr<Effect>			m_effect;
	std::shared_ptr<RenderPass>		m_renderPass;

	std::mutex													m_parameterLock;
	std::map<std::string, std::shared_ptr<MaterialParameter>>	m_parameters;

	std::mutex													m_resourceLock;
	std::map<std::string, std::shared_ptr<MaterialResource>>	m_resources;

	std::shared_ptr<RenderThread::MaterialData>					m_renderThreadData;
};


namespace RenderThread
{

struct MaterialData
{
	void Update(GraphicsCommandList& commandList);
	void Commit(GraphicsCommandList& commandList);

	// Render pass
	std::shared_ptr<RenderPass>		renderPass;

	// PSO
	std::shared_ptr<GraphicsPSO>	pso;

	// Root signature
	std::shared_ptr<RootSignature>	rootSignature;
};

} // namespace RenderThread


} // namespace Kodiak