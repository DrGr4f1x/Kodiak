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
class ComputeConstantBuffer;
class ComputePSO;
class ComputeParameter;
class ComputeResource;
class ComputeShader;
class GpuBuffer;
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

	void SetComputeShaderPath(const std::string& path);
	void SetComputeShaderPath(const std::string& path, concurrency::task<void>& waitTask);

	std::shared_ptr<ComputeConstantBuffer> GetConstantBuffer(const std::string& name);
	std::shared_ptr<ComputeParameter> GetParameter(const std::string& name);
	std::shared_ptr<ComputeResource> GetResource(const std::string& name);

	void Dispatch(ComputeCommandList& commandList, size_t groupCountX = 1, size_t groupCountY = 1, size_t groupCountZ = 1);
	void Dispatch1D(ComputeCommandList& commandList, size_t threadCountX, size_t groupSizeX = 64);
	void Dispatch2D(ComputeCommandList& commandList, size_t threadCountX, size_t threadCountY, size_t groupSizeX = 8, size_t groupSizeY = 8);
	void Dispatch3D(ComputeCommandList& commandList, size_t threadCountX, size_t threadCountY, size_t threadCountZ, size_t groupSizeX,
		size_t groupSizeY, size_t groupSizeZ);
	void DispatchIndirect(ComputeCommandList& commandList, GpuBuffer& argumentBuffer, size_t argumentBufferOffset = 0);

	void UnbindSRVs(ComputeCommandList& commandList) {}
	void UnbindUAVs(ComputeCommandList& commandList) {}

private:
	void SetupKernel();

private:
	std::string							m_name;
	std::string							m_shaderPath;
	std::shared_ptr<ComputeShader>		m_computeShader;
	
	std::mutex														m_constantBufferLock;
	std::map<std::string, std::shared_ptr<ComputeConstantBuffer>>	m_constantBuffers;

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
	void Commit(ComputeCommandList& commandList);

	std::shared_ptr<ComputePSO>					pso;
	std::shared_ptr<RootSignature>				rootSignature;

	// Constant buffer data
	MappedConstantBuffer			cbuffer;
	byte*							cbufferData{ nullptr };
	bool							cbufferDirty{ true };
	uint32_t						cbufferSize{ 0 };

	std::vector<ShaderReflection::DescriptorRange> rootParameters;

	// Master list of CPU handles
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> cpuHandles;
};

} // namespace RenderThread


} // namespace Kodiak