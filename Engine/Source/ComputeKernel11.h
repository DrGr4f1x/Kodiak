// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "Shader.h"

#include <ppltasks.h>

namespace Kodiak
{

// Forward declarations
class ComputeParameter;
class ComputePSO;
class ComputeResource;
class ConstantBuffer;
namespace RenderThread { struct ComputeData; }

class ComputeKernel
{
public:
	concurrency::task<void> loadTask;

public:
	ComputeKernel();
	ComputeKernel(const std::string& name);

	void SetName(const std::string& name) { m_name = name; }
	const std::string& GetName() const { return m_name; }

	void SetComputeShaderPath(const std::string& path, const std::string& file)
	{
		SetComputeShaderPath(ShaderPath(path, file));
	}
	void SetComputeShaderPath(const ShaderPath& shaderPath);

	std::shared_ptr<ComputeParameter> GetParameter(const std::string& name);
	std::shared_ptr<ComputeResource> GetResource(const std::string& name);

private:
	void SetupKernel();

private:
	std::string							m_name;
	ShaderPath							m_shaderPath;
	std::shared_ptr<ComputeShader>		m_computeShader;

	std::mutex													m_parameterLock;
	std::map<std::string, std::shared_ptr<ComputeParameter>>	m_parameters;

	std::mutex													m_resourceLock;
	std::map<std::string, std::shared_ptr<ComputeResource>>		m_resources;

	// Render thread data
	std::shared_ptr<RenderThread::ComputeData>	m_renderThreadData;
};


namespace RenderThread
{

struct ComputeData
{
	~ComputeData()
	{
		_aligned_free(cbufferData);
	}

	std::shared_ptr<ComputePSO>		pso;

	// We're using a single cbuffer for all shader stages, via the DX11.1 large cbuffer feature
	std::shared_ptr<ConstantBuffer> cbuffer;
	uint8_t*						cbufferData{ nullptr };
	bool							cbufferDirty{ true };
	size_t							cbufferSize{ 0 };

	struct CBufferBinding
	{
		uint32_t startSlot;
		uint32_t numBuffers;
		std::array<ID3D11Buffer*, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT>	cbuffers;
		std::array<uint32_t, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT>			firstConstant;
		std::array<uint32_t, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT>			numConstants;
	};

	CBufferBinding cbufferBinding;

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

	ResourceTable<ID3D11ShaderResourceView>		srvTables;
	ResourceTable<ID3D11UnorderedAccessView> 	uavTables;
	ResourceTable<ID3D11SamplerState>			samplerTables;
};

} // namespace RenderThread

} // namespace Kodiak