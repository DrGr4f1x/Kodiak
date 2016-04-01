// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: DavSid Elder
//

#pragma once

#include <ppltasks.h>

namespace Kodiak
{

// Forward declarations
class ConstantBuffer;
class Effect;
class GraphicsCommandList;
class GraphicsPSO;
class MaterialParameter;
class MaterialResource;
class RenderPass;
class Texture;
enum class ShaderResourceDimension;
enum class ShaderResourceType;
enum class ShaderType;
enum class ShaderVariableType;

namespace RenderThread
{
struct MaterialData;
} // namespace RenderThread


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
	~MaterialData() 
	{ 
		_aligned_free(cbufferData); 
	}

	void Update(GraphicsCommandList& commandList);
	void Commit(GraphicsCommandList& commandList);
	bool IsReady() { return true; }

	// Render pass
	std::shared_ptr<RenderPass>		renderPass;

	// PSO
	std::shared_ptr<GraphicsPSO>	pso;

	// We're using a single cbuffer for all shader stages, via the DX11.1 large cbuffer feature
	std::shared_ptr<ConstantBuffer> cbuffer;
	uint8_t*						cbufferData{ nullptr };
	bool							cbufferDirty{ true };
	size_t							cbufferSize{ 0 };

	// Per-shader stage cbuffer bindings
	struct CBufferBinding
	{
		uint32_t startSlot;
		uint32_t numBuffers;
		std::array<ID3D11Buffer*, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT>	cbuffers;
		std::array<uint32_t, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT>			firstConstant;
		std::array<uint32_t, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT>			numConstants;
	};

	std::array<CBufferBinding, 5> cbufferBindings;

	template<class ResourceType>
	struct ResourceTable
	{
		struct TableLayout
		{
			uint32_t						shaderRegister;
			uint32_t						numItems;
			std::vector<ResourceType*>		resources;
		};
		std::vector<TableLayout>			layouts;
	};

	std::array<ResourceTable<ID3D11ShaderResourceView>,  5>		srvTables;
	std::array<ResourceTable<ID3D11UnorderedAccessView>, 5> 	uavTables;
	std::array<ResourceTable<ID3D11SamplerState>,		 5>		samplerTables;
};


} // namespace RenderThread

} // namespace Kodiak