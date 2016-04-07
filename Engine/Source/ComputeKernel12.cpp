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

#include "ComputeParameter.h"
#include "ComputeResource12.h"
#include "PipelineState12.h"
#include "RootSignature12.h"
#include "ShaderManager.h"

using namespace Kodiak;
using namespace std;


ComputeKernel::ComputeKernel()
{
	loadTask = concurrency::create_task([] {});
}


ComputeKernel::ComputeKernel(const string& name)
	: m_name(name)
{
	loadTask = concurrency::create_task([] {});
}


void ComputeKernel::SetComputeShaderPath(const ShaderPath& shaderPath)
{
	m_shaderPath = shaderPath;
	m_computeShader = ShaderManager::GetInstance().LoadComputeShader(shaderPath);

	loadTask = m_computeShader->loadTask.then([this] { SetupKernel(); });
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


void ComputeKernel::SetupKernel()
{
	using namespace ShaderReflection;

	m_renderThreadData = make_shared<RenderThread::ComputeData>();
	auto& computeData = *m_renderThreadData;

	const auto& shaderSig = m_computeShader->GetSignature();


	// Figure out the number of root parameters we need
	uint32_t numRootParameters = 0;
	numRootParameters += static_cast<uint32_t>(shaderSig.cbvTable.size());  // One root param per CBV
	numRootParameters += !shaderSig.srvTable.empty() ? 1 : 0;				// One root param for SRV table (holds all SRVs)
	numRootParameters += !shaderSig.uavTable.empty() ? 1 : 0;				// One root param for UAV table (holds all UAVs)

	// Setup root signature
	m_rootSignature = make_shared<RootSignature>(numRootParameters, shaderSig.numSamplers);
	auto& rootSig = *m_rootSignature;
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
		// Allocate the CPU-side buffer mirror
		computeData.cbufferData = (byte*)_aligned_malloc(computeData.cbufferSize, 16);
	}


	// Create the master list of CPU handles for everything bound to the pipeline for this material
	computeData.cpuHandles.reserve(shaderSig.numDescriptors);
	CD3DX12_CPU_DESCRIPTOR_HANDLE nullHandle(D3D12_DEFAULT);
	computeData.cpuHandles.insert(end(computeData.cpuHandles), shaderSig.numDescriptors, nullHandle);


	uint32_t currentDescriptor = 0;

	// CBVs - one root parameter per constant buffer
	for (const auto& shaderCBV : shaderSig.cbvTable)
	{
		// Create root parameter for the CBV in the root signature
		const auto curRootIndex = rootIndex;
		auto& rootParam = rootSig[rootIndex++];
		rootParam.InitAsConstantBuffer(shaderCBV.shaderRegister);

		// Create root parameter binding
		computeData.rootParameters.push_back(DescriptorRange(curRootIndex, currentDescriptor, 1));

		// Create the actual constant buffer view and assign its cpu handle in the master list
		auto cpuHandle = computeData.cbuffer.CreateConstantBufferView(shaderCBV.byteOffset, shaderCBV.sizeInBytes);
		computeData.cpuHandles[currentDescriptor] = cpuHandle;

		// Track the current descriptor index for the master CPU handle list
		++currentDescriptor;
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
						lock_guard<mutex> CS(m_resourceLock);

						shared_ptr<ComputeResource> computeResource;

						auto it = m_resources.find(srv.name);
						if (end(m_resources) != it)
						{
							computeResource = it->second;
						}
						else
						{
							computeResource = make_shared<ComputeResource>(srv.name);
						}

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
						lock_guard<mutex> CS(m_resourceLock);

						shared_ptr<ComputeResource> computeResource;

						auto it = m_resources.find(uav.name);
						if (end(m_resources) != it)
						{
							computeResource = it->second;
						}
						else
						{
							computeResource = make_shared<ComputeResource>(uav.name);
						}

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
	

	// Parameters
	{
		lock_guard<mutex> CS(m_parameterLock);

		for (const auto& parameter : shaderSig.parameters)
		{
			shared_ptr<ComputeParameter> computeParameter;

			auto it = m_parameters.find(parameter.name);
			if (end(m_parameters) != it)
			{
				computeParameter = it->second;
			}
			else
			{
				computeParameter = make_shared<ComputeParameter>(parameter.name);
				m_parameters[parameter.name] = computeParameter;
			}

			computeParameter->CreateRenderThreadData(m_renderThreadData, parameter);
		}
	}

	// Finalize the Root Signature
	rootSig.Finalize();

	// Setup the PSO
	m_pso = make_shared<ComputePSO>();
	m_pso->SetComputeShader(m_computeShader.get());
	m_pso->SetRootSignature(*m_rootSignature);
	m_pso->Finalize();
}