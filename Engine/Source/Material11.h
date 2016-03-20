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
enum class ShaderVariableType;


namespace RenderThread
{
struct MaterialData;
class MaterialParameterData;
class MaterialResourceData;
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


class MaterialParameter
{
	friend class Material;
public:
	MaterialParameter(const std::string& name);

	const std::string& GetName() const { return m_name; }
	
	template <typename T>
	void SetValue(T value);

private:
	const std::string m_name;

	std::array<uint8_t, 64> m_data;

	std::shared_ptr<RenderThread::MaterialParameterData> m_renderThreadData;
};


class MaterialResource
{
	friend class Material;
public:
	MaterialResource(const std::string& name);

	void SetResource(std::shared_ptr<Texture> texture);

private:
	const std::string m_name;

	std::array<uint32_t, 5> m_shaderSlots;
	ShaderResourceType		m_type;
	ShaderResourceDimension m_dimension;

	std::shared_ptr<Texture>	m_texture;

	std::shared_ptr<RenderThread::MaterialResourceData>	m_renderThreadData;
};


namespace RenderThread
{

struct MaterialData
{
	~MaterialData() { _aligned_free(cbufferData); }

	void Commit(GraphicsCommandList& commandList);

	// Render pass
	std::shared_ptr<RenderPass>		renderPass;

	// PSO
	std::shared_ptr<GraphicsPSO>	pso;

	// We're using a single cbuffer for all shader stages, via the DX11.1 large cbuffer feature
	std::shared_ptr<ConstantBuffer> cbuffer;
	uint8_t*						cbufferData{ nullptr };
	bool							cbufferDirty{ true };

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

	// Callbacks for binding cbuffers to the command list at render time
	std::array<std::function<void(GraphicsCommandList&)>, 5> cbufferCallbacks;
};


class MaterialParameterData
{
	friend class Material;
public:
	MaterialParameterData();

	void SetValue(bool value);
	void SetValue(int32_t value);
	void SetValue(DirectX::XMINT2 value);
	void SetValue(DirectX::XMINT3 value);
	void SetValue(DirectX::XMINT4 value);
	void SetValue(uint32_t value);
	void SetValue(DirectX::XMUINT2 value);
	void SetValue(DirectX::XMUINT3 value);
	void SetValue(DirectX::XMUINT4 value);
	void SetValue(float value);
	void SetValue(DirectX::XMFLOAT2 value);
	void SetValue(DirectX::XMFLOAT3 value);
	void SetValue(DirectX::XMFLOAT4 value);
	void SetValue(DirectX::XMFLOAT4X4 value);

private:
	template <typename T>
	void InternalSetValue(T value);

private:
	ShaderVariableType		m_type;
	std::array<uint8_t, 64> m_data;

	std::array<uint8_t*, 5>	m_bindings;
	bool*					m_dirtyFlag{ nullptr };
};


class MaterialResourceData
{
	friend class Material;
public:
	MaterialResourceData();

	void SetResource(ID3D11ShaderResourceView* srv);

private:
	std::array<uint32_t, 5> m_shaderSlots;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_srv;
};

} // namespace RenderThread

} // namespace Kodiak