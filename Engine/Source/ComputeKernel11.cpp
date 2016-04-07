// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "ComputeKernel11.h"

#include "ComputeParameter.h"
#include "ComputeResource11.h"
#include "ConstantBuffer11.h"
#include "PipelineState11.h"
#include "ShaderManager.h"
#include "ShaderReflection.h"


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


	// Figure out total CBV memory requirements so we can allocate the master CBV
	computeData.cbufferSize = 0;
	for (const auto& shaderCBV : shaderSig.cbvTable)
	{
		computeData.cbufferSize += shaderCBV.sizeInBytes;
	}

	if (computeData.cbufferSize > 0)
	{
		// Create an uber-cbuffer for the entire kernel
		auto cbuffer = make_shared<ConstantBuffer>();
		cbuffer->Create(computeData.cbufferSize, Usage::Dynamic);
		computeData.cbuffer = cbuffer;

		// CPU-side memory mirror
		if (computeData.cbufferData)
		{
			_aligned_free(computeData.cbufferData);
			computeData.cbufferData = nullptr;
		}
		computeData.cbufferData = (byte*)_aligned_malloc(computeData.cbufferSize, 16);
	}

	
	// Setup constant buffer bindings
	uint32_t minSlot = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;
	uint32_t numBuffers = static_cast<uint32_t>(shaderSig.cbvTable.size());

	if (numBuffers > 0)
	{
		// Initialize cbuffer binding record with default values
		fill(begin(computeData.cbufferBinding.cbuffers), end(computeData.cbufferBinding.cbuffers), nullptr);
		fill(begin(computeData.cbufferBinding.firstConstant), end(computeData.cbufferBinding.firstConstant), 0);
		fill(begin(computeData.cbufferBinding.numConstants), end(computeData.cbufferBinding.numConstants), 0);

		// Figure out the starting shader register slot
		for (const auto& shaderCbv : shaderSig.cbvTable)
		{
			minSlot = min(minSlot, shaderCbv.shaderRegister);
		}

		// Fill out the cbuffer binding record
		for (const auto& shaderCbv : shaderSig.cbvTable)
		{
			const auto index = shaderCbv.shaderRegister - minSlot;

			computeData.cbufferBinding.cbuffers[index] = computeData.cbuffer->constantBuffer.Get();
			computeData.cbufferBinding.firstConstant[index] = shaderCbv.byteOffset;
			computeData.cbufferBinding.numConstants[index] = shaderCbv.sizeInBytes;
		}
	}

	computeData.cbufferBinding.startSlot = minSlot;
	computeData.cbufferBinding.numBuffers = numBuffers;


	// Setup SRV tables
	for (const auto& shaderSrvTable : shaderSig.srvTable)
	{
		RenderThread::ComputeData::ResourceTable<ID3D11ShaderResourceView>::TableLayout layout;
		layout.shaderRegister = shaderSrvTable.shaderRegister;
		layout.numItems = shaderSrvTable.numItems;
		layout.resources.reserve(layout.numItems);
		layout.resources.insert(layout.resources.end(), layout.numItems, nullptr);

		computeData.srvTables.layouts.push_back(layout);
	}


	// Setup UAV tables
	for (const auto& shaderUavTable : shaderSig.uavTable)
	{
		RenderThread::ComputeData::ResourceTable<ID3D11UnorderedAccessView>::TableLayout layout;
		layout.shaderRegister = shaderUavTable.shaderRegister;
		layout.numItems = shaderUavTable.numItems;
		layout.resources.reserve(layout.numItems);
		layout.resources.insert(layout.resources.end(), layout.numItems, nullptr);

		computeData.uavTables.layouts.push_back(layout);
	}


	// Setup sampler tables
	for (const auto& shaderSamplerTable : shaderSig.samplerTable)
	{
		RenderThread::ComputeData::ResourceTable<ID3D11SamplerState>::TableLayout layout;
		layout.shaderRegister = shaderSamplerTable.shaderRegister;
		layout.numItems = shaderSamplerTable.numItems;
		layout.resources.reserve(layout.numItems);
		layout.resources.insert(layout.resources.end(), layout.numItems, nullptr);

		computeData.samplerTables.layouts.push_back(layout);
	}


	// Bind parameters
	for (const auto& parameter : shaderSig.parameters)
	{
		auto computeParameter = GetParameter(parameter.name);
		computeParameter->CreateRenderThreadData(m_renderThreadData, parameter);
	}


	// Bind SRV resources
	for (const auto& resource : shaderSig.resources)
	{
		auto computeResource = GetResource(resource.name);
		computeResource->CreateRenderThreadData(m_renderThreadData, resource);
	}


	// Bind UAV resources
	for (const auto& resource : shaderSig.uavs)
	{
		auto computeResource = GetResource(resource.name);
		computeResource->CreateRenderThreadData(m_renderThreadData, resource);
	}


	// Bind samplers
	for (const auto& sampler : shaderSig.samplers)
	{
		// TODO
	}

	// Setup the PSO
	computeData.pso = make_shared<ComputePSO>();
	computeData.pso->SetComputeShader(m_computeShader.get());
	computeData.pso->Finalize();
}