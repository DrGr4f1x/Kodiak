// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "Material.h"

#include "Effect.h"
#include "CommandList12.h"
#include "MaterialParameter12.h"
#include "MaterialResource12.h"
#include "RenderPass.h"
#include "RootSignature12.h"


using namespace Kodiak;
using namespace std;


Material::Material()
{}


Material::Material(const string& name)
	: m_name(name)
{}


void Material::SetEffect(shared_ptr<Effect> effect)
{
	assert(effect);

	m_effect = effect;

	auto thisMaterial = shared_from_this();

	prepareTask = effect->loadTask.then([thisMaterial, effect]
	{
		thisMaterial->CreateRenderThreadData();
	});
}


void Material::SetRenderPass(shared_ptr<RenderPass> pass)
{
	m_renderPass = pass;
}


shared_ptr<MaterialParameter> Material::GetParameter(const string& name)
{
	lock_guard<mutex> CS(m_parameterLock);

	auto it = m_parameters.find(name);
	if (end(m_parameters) != it)
	{
		return it->second;
	}

	auto parameter = make_shared<MaterialParameter>(name);
	m_parameters[name] = parameter;

	return parameter;
}


shared_ptr<MaterialResource> Material::GetResource(const string& name)
{
	lock_guard<mutex> CS(m_resourceLock);

	auto it = m_resources.find(name);
	if (end(m_resources) != it)
	{
		return it->second;
	}

	auto resource = make_shared<MaterialResource>(name);
	m_resources[name] = resource;

	return resource;
}


shared_ptr<Material> Material::Clone()
{
	auto clone = make_shared<Material>();

	clone->SetName(m_name);
	clone->SetEffect(m_effect);
	clone->SetRenderPass(m_renderPass);

	return clone;
}


void Material::CreateRenderThreadData()
{
	m_renderThreadData = make_shared<RenderThread::MaterialData>();
	RenderThread::MaterialData& materialData = *m_renderThreadData;

	materialData.renderPass = m_renderPass;
	materialData.pso = m_effect->GetPSO();
	materialData.rootSignature = m_effect->GetRootSignature();

	auto& effectSig = m_effect->GetSignature();

	// Create the master list of CPU handles for everything bound to the pipeline for this material
	materialData.cpuHandles.reserve(effectSig.totalDescriptors);
	CD3DX12_CPU_DESCRIPTOR_HANDLE nullHandle(D3D12_DEFAULT);
	materialData.cpuHandles.insert(end(materialData.cpuHandles), effectSig.totalDescriptors, nullHandle);

	// Copy the root parameters from the effect
	materialData.rootParameters = effectSig.rootParameters;

	// Setup constant buffers
	materialData.constantDataSize = effectSig.cbvPerMaterialDataSize;
	if (materialData.constantDataSize)
	{
		// Create mapped ConstantBuffer
		materialData.cbuffer.CreateUpload("Material cbuffer", materialData.constantDataSize);
		materialData.cbufferData = materialData.cbuffer.Map();

		// Create CBVs
		if (!effectSig.cbvBindings.empty())
		{
			for (uint32_t i = 0; i < 5; ++i)
			{
				for (const auto& cbv : effectSig.cbvBindings[i])
				{
					auto handle = materialData.cbuffer.CreateConstantBufferView(cbv.byteOffset, cbv.sizeInBytes);
					materialData.cpuHandles[cbv.binding.tableIndex] = handle;
				}
			}
		}
	}

	// Parameters
	{
		lock_guard<mutex> CS(m_parameterLock);

		for (const auto& parameter : effectSig.parameters)
		{
			shared_ptr<MaterialParameter> matParameter;

			auto it = m_parameters.find(parameter.second.name);
			if (end(m_parameters) != it)
			{
				matParameter = it->second;
			}
			else
			{
				matParameter = make_shared<MaterialParameter>(parameter.second.name);
				m_parameters[parameter.second.name] = matParameter;
			}

			matParameter->CreateRenderThreadData(m_renderThreadData, parameter.second);
		}
	}

	// Resources
	{
		lock_guard<mutex> CS(m_resourceLock);

		for (const auto& resource : effectSig.srvs)
		{
			shared_ptr<MaterialResource> matResource;

			auto it = m_resources.find(resource.second.name);
			if (end(m_resources) != it)
			{
				matResource = it->second;
			}
			else
			{
				matResource = make_shared<MaterialResource>(resource.second.name);
			}

			matResource->CreateRenderThreadData(m_renderThreadData, resource.second);
		}
	}

	// TODO: UAVs
	// TODO: Samplers
}


void RenderThread::MaterialData::Update(GraphicsCommandList& commandList)
{}


void RenderThread::MaterialData::Commit(GraphicsCommandList& commandList)
{
	commandList.SetRootSignature(*rootSignature);
	commandList.SetPipelineState(*pso);

	const uint32_t numParams = static_cast<uint32_t>(rootParameters.size());
	// TODO: hack, skipping first 2 slots (per-view and per-object data)
	uint32_t rootIndex = kInvalid;
	uint32_t offset = 0;
	for (uint32_t i = 2; i < numParams; ++i)
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


bool RenderThread::MaterialData::IsReady()
{
	// TODO: hack, skipping first 2 slots (per-view and per-object data)
	const uint32_t numParams = static_cast<uint32_t>(cpuHandles.size());
	for (uint32_t i = 2; i < numParams; ++i)
	{
		if (cpuHandles[i].ptr == 0 || cpuHandles[i].ptr == ~0ull)
		{
			return false;
		}
	}
	return true;
}