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

#include "GpuBuffer12.h"
#include <ppltasks.h>

namespace Kodiak
{

// Forward declarations
class ComputePSO;
class ComputeParameter;
class ComputeResource;
class ComputeShader;
class RootSignature;
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

	std::shared_ptr<ComputePSO>					m_pso;
	std::shared_ptr<RootSignature>				m_rootSignature;

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

	// Constant buffer data
	MappedConstantBuffer			cbuffer;
	byte*							cbufferData{ nullptr };
	uint32_t						cbufferSize{ 0 };

	std::vector<ShaderReflection::DescriptorRange> rootParameters;

	// Master list of CPU handles
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> cpuHandles;
};

} // namespace RenderThread


} // namespace Kodiak