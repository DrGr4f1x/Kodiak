// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "ComputeKernel12.h"

#include "CommandList12.h"
#include "CommonStates.h"
#include "ComputeConstantBuffer.h"
#include "ComputeParameter.h"
#include "ComputeResource12.h"
#include "PipelineState12.h"
#include "RootSignature12.h"


using namespace Kodiak;
using namespace std;


ComputeKernel::ComputeKernel(const string& name)
	: m_name(name)
{}


void ComputeKernel::SetComputeShaderPath(const string& path, bool immediate)
{
	m_shaderPath = path;
	if (immediate)
	{
		m_computeShader = ComputeShader::LoadImmediate(path);
		SetupKernel();
	}
	else
	{
		m_computeShader = ComputeShader::Load(path);
	}
}


shared_ptr<ComputeConstantBuffer> ComputeKernel::GetConstantBuffer(const string& name)
{
	lock_guard<mutex> CS(m_constantBufferLock);

	auto it = m_constantBuffers.find(name);
	if (end(m_constantBuffers) != it)
	{
		return it->second;
	}

	auto constantBuffer = make_shared<ComputeConstantBuffer>(name);
	m_constantBuffers[name] = constantBuffer;

	return constantBuffer;
}


shared_ptr<ComputeParameter> ComputeKernel::GetParameter(const string& name)
{
	lock_guard<mutex> CS(m_parameterLock);

	auto it = m_parameters.find(name);
	if (end(m_parameters) != it)
	{
		return it->second;
	}

	auto parameter = make_shared<ComputeParameter>(name);
	m_parameters[name] = parameter;

	return parameter;
}


shared_ptr<ComputeResource> ComputeKernel::GetResource(const string& name)
{
	lock_guard<mutex> CS(m_resourceLock);

	auto it = m_resources.find(name);
	if (end(m_resources) != it)
	{
		return it->second;
	}

	auto resource = make_shared<ComputeResource>(name);
	m_resources[name] = resource;

	return resource;
}


void ComputeKernel::Dispatch(ComputeCommandList& commandList, size_t groupCountX, size_t groupCountY, size_t groupCountZ)
{
	assert(m_renderThreadData);

	m_renderThreadData->Commit(commandList);
	commandList.Dispatch(groupCountX, groupCountY, groupCountZ);
}


void ComputeKernel::Dispatch1D(ComputeCommandList& commandList, size_t threadCountX, size_t groupSizeX)
{
	assert(m_renderThreadData);

	m_renderThreadData->Commit(commandList);
	commandList.Dispatch1D(threadCountX, groupSizeX);
}


void ComputeKernel::Dispatch2D(ComputeCommandList& commandList, size_t threadCountX, size_t threadCountY, size_t groupSizeX, size_t groupSizeY)
{
	assert(m_renderThreadData);

	m_renderThreadData->Commit(commandList);
	commandList.Dispatch2D(threadCountX, threadCountY, groupSizeX, groupSizeY);
}


void ComputeKernel::Dispatch3D(ComputeCommandList& commandList, size_t threadCountX, size_t threadCountY, size_t threadCountZ, size_t groupSizeX,
	size_t groupSizeY, size_t groupSizeZ)
{
	assert(m_renderThreadData);

	m_renderThreadData->Commit(commandList);
	commandList.Dispatch3D(threadCountX, threadCountY, threadCountZ, groupSizeX, groupSizeY, groupSizeZ);
}


void ComputeKernel::DispatchIndirect(ComputeCommandList& commandList, GpuBuffer& argumentBuffer, size_t argumentBufferOffset)
{
	assert(m_renderThreadData);

	m_renderThreadData->Commit(commandList);
	commandList.DispatchIndirect(argumentBuffer, argumentBufferOffset);
}


void ComputeKernel::SetupKernel()
{
	using namespace ShaderReflection;

	m_renderThreadData = make_shared<RenderThread::ComputeData>();
	auto& computeData = *m_renderThreadData;

	const auto& shaderSig = m_computeShader->GetSignature();


	// Figure out the number of root parameters we need
	uint32_t numRootParameters = 0;
	numRootParameters += !shaderSig.cbvTable.empty() ? 1 : 0;  // One root param per CBV
	numRootParameters += !shaderSig.srvTable.empty() ? 1 : 0;				// One root param for SRV table (holds all SRVs)
	numRootParameters += !shaderSig.uavTable.empty() ? 1 : 0;				// One root param for UAV table (holds all UAVs)

	// Setup root signature
	computeData.rootSignature = make_shared<RootSignature>(numRootParameters, shaderSig.numSamplers);
	auto& rootSig = *computeData.rootSignature;
	uint32_t rootIndex = 0;


	// Figure out total CBV memory requirements so we can allocate the master CBV
	computeData.cbufferSize = 0;
	for (const auto& shaderCBV : shaderSig.cbvTable)
	{
		computeData.cbufferSize += shaderCBV.sizeInBytes;
	}

	if (computeData.cbufferSize > 0)
	{
		// Allocate the GPU-side buffer
		computeData.cbuffer.CreateUpload("Compute kernel cbuffer", computeData.cbufferSize);
		computeData.cbufferData = computeData.cbuffer.Map();
	}


	// Create the master list of CPU handles for everything bound to the pipeline for this material
	computeData.cpuHandles.reserve(shaderSig.numDescriptors);
	CD3DX12_CPU_DESCRIPTOR_HANDLE nullHandle(D3D12_DEFAULT);
	computeData.cpuHandles.insert(end(computeData.cpuHandles), shaderSig.numDescriptors, nullHandle);


	uint32_t currentDescriptor = 0;

	// CBVs - one root parameter for the whole table
	if (!shaderSig.cbvTable.empty())
	{
		// Create root parameter for the CBV in the root signature
		const auto curRootIndex = rootIndex;
		auto& rootParam = rootSig[rootIndex++];
		rootParam.InitAsDescriptorTable(static_cast<uint32_t>(shaderSig.cbvTable.size()));

		uint32_t rangeIndex = 0;

		for (const auto& shaderCBV : shaderSig.cbvTable)
		{
			rootParam.SetTableRange(rangeIndex, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, shaderCBV.shaderRegister, 1);

			// Create root parameter binding
			computeData.rootParameters.push_back(DescriptorRange(curRootIndex, currentDescriptor, 1));

			// Create the actual constant buffer view and assign its cpu handle in the master list
			auto cpuHandle = computeData.cbuffer.CreateConstantBufferView(shaderCBV.byteOffset, shaderCBV.sizeInBytes);
			computeData.cpuHandles[currentDescriptor] = cpuHandle;

			// Track the current descriptor index for the master CPU handle list
			++currentDescriptor;

			++rangeIndex;
		}
	}


	// SRVs - one root parameter for the whole table
	if (!shaderSig.srvTable.empty())
	{
		// Create root parameter for the SRV table in the root signature
		const auto curRootIndex = rootIndex;
		auto& rootParam = rootSig[rootIndex++];
		rootParam.InitAsDescriptorTable(static_cast<uint32_t>(shaderSig.srvTable.size()));

		uint32_t rangeIndex = 0;

		// Loop over the SRV ranges in the table
		for (const auto& srvTable : shaderSig.srvTable)
		{
			rootParam.SetTableRange(rangeIndex, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, srvTable.shaderRegister, srvTable.numItems);

			const auto numItems = srvTable.numItems;
			computeData.rootParameters.push_back(DescriptorRange(curRootIndex, currentDescriptor, numItems));
			
			// Bind SRVs (kernel parameters) here
			const uint32_t firstRegister = srvTable.shaderRegister;
			const uint32_t lastRegister = firstRegister + srvTable.numItems;
			uint32_t currentDescriptorSlot = currentDescriptor;

			// Loop over the shader registers included in this table
			for (uint32_t shaderRegister = firstRegister; shaderRegister < lastRegister; ++shaderRegister)
			{
				bool found = false;

				// Find the SRV from the shader mapped to the current shader register
				for (const auto& srv : shaderSig.resources)
				{
					if (srv.shaderRegister[0] == shaderRegister)
					{
						auto computeResource = GetResource(srv.name);
						computeResource->CreateRenderThreadData(m_renderThreadData, srv, currentDescriptorSlot);

						found = true;
						break;
					}
				}
				++currentDescriptorSlot;
				assert(found);
			}

			++rangeIndex;

			// Track the current descriptor index for the master CPU handle list
			currentDescriptor += numItems;
		}
	}


	// UAVs - one root parameter for the whole table
	if (!shaderSig.uavTable.empty())
	{
		// Create root parameter for the UAV table in the root signature
		const auto curRootIndex = rootIndex;
		auto& rootParam = rootSig[rootIndex++];
		rootParam.InitAsDescriptorTable(static_cast<uint32_t>(shaderSig.uavTable.size()));

		uint32_t rangeIndex = 0;

		// Loop over the UAV ranges in the table
		for (const auto& uavTable : shaderSig.uavTable)
		{
			rootParam.SetTableRange(rangeIndex, D3D12_DESCRIPTOR_RANGE_TYPE_UAV, uavTable.shaderRegister, uavTable.numItems);

			const auto numItems = uavTable.numItems;
			computeData.rootParameters.push_back(DescriptorRange(curRootIndex, currentDescriptor, numItems));

			// Bind UAVs (kernel parameters) here
			const uint32_t firstRegister = uavTable.shaderRegister;
			const uint32_t lastRegister = firstRegister + uavTable.numItems;
			uint32_t currentDescriptorSlot = currentDescriptor;

			// Loop over the shader registers included in this table
			for (uint32_t shaderRegister = firstRegister; shaderRegister < lastRegister; ++shaderRegister)
			{
				bool found = false;

				// Find the UAV from the shader mapped to the current shader register
				for (const auto& uav : shaderSig.uavs)
				{
					if (uav.shaderRegister[0] == shaderRegister)
					{
						auto computeResource = GetResource(uav.name);
						computeResource->CreateRenderThreadData(m_renderThreadData, uav, currentDescriptorSlot);

						found = true;
						break;
					}
				}
				++currentDescriptorSlot;
				assert(found);
			}

			++rangeIndex;

			// Track the current descriptor index for the master CPU handle list
			currentDescriptor += numItems;
		}
	}
	

	// Bind constant buffers
	for (const auto& cbv : shaderSig.cbvTable)
	{
		auto computeConstantBuffer = GetConstantBuffer(cbv.name);
		computeConstantBuffer->CreateRenderThreadData(m_renderThreadData, cbv);
	}

	// Parameters
	for (const auto& parameter : shaderSig.parameters)
	{
		auto computeParameter = GetParameter(parameter.name);
		computeParameter->CreateRenderThreadData(m_renderThreadData, parameter);
	}

	// Samplers
	for (const auto& sampler : shaderSig.samplers)
	{
		const auto& samplerDesc = CommonStates::NamedSampler(sampler.name);

		rootSig.InitStaticSampler(sampler.shaderRegister[0], samplerDesc, D3D12_SHADER_VISIBILITY_ALL);
	}

	// Finalize the Root Signature
	rootSig.Finalize();

	// Setup the PSO
	computeData.pso = make_shared<ComputePSO>();
	computeData.pso->SetComputeShader(m_computeShader.get());
	computeData.pso->SetRootSignature(*computeData.rootSignature);
	computeData.pso->Finalize();
}


void RenderThread::ComputeData::Commit(ComputeCommandList& commandList)
{
	commandList.SetRootSignature(*rootSignature);
	commandList.SetPipelineState(*pso);

	const uint32_t numParams = static_cast<uint32_t>(rootParameters.size());
	uint32_t rootIndex = kInvalid;
	uint32_t offset = 0;
	for (uint32_t i = 0; i < numParams; ++i)
	{
		const auto& rootParam = rootParameters[i];

		if (rootIndex != rootParam.rootIndex)
		{
			rootIndex = rootParam.rootIndex;
			offset = 0;
		}

		if (rootParam.numElements == kInvalid)
		{
			commandList.SetDynamicDescriptor(rootParam.rootIndex, 0, cpuHandles[rootParam.startSlot]);
		}
		else
		{
			commandList.SetDynamicDescriptors(rootParam.rootIndex, offset, rootParam.numElements, &cpuHandles[rootParam.startSlot]);
			offset += rootParam.numElements;
		}
	}
}