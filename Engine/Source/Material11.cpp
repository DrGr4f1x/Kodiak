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

#include "ConstantBuffer.h"
#include "CommandList.h"
#include "Effect.h"
#include "MaterialConstantBuffer.h"
#include "MaterialParameter.h"
#include "MaterialResource11.h"
#include "PipelineState.h"
#include "Renderer.h"
#include "RenderEnums.h"
#include "RenderPass.h"
#include "Texture.h"


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
	assert(effect->IsFinalized());

	m_effect = effect;

	CreateRenderThreadData();
}


void Material::SetRenderPass(shared_ptr<RenderPass> pass)
{
	m_renderPass = pass;
	if (m_renderThreadData)
	{
		m_renderThreadData->renderPass = pass;
	}
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


shared_ptr<MaterialConstantBuffer> Material::GetConstantBuffer(const string& name)
{
	lock_guard<mutex> CR(m_constantBufferLock);

	auto it = m_constantBuffers.find(name);
	if (end(m_constantBuffers) != it)
	{
		return it->second;
	}

	auto cbuffer = make_shared<MaterialConstantBuffer>(name);
	m_constantBuffers[name] = cbuffer;

	return cbuffer;
}


void Material::SetResource(const string& name, shared_ptr<Texture> texture)
{
	m_textures[name] = texture;

	auto resource = GetResource(name);
	resource->SetSRV(*texture);
}


shared_ptr<Material> Material::Clone()
{
	auto clone = make_shared<Material>();

	clone->SetName(m_name);
	clone->SetEffect(m_effect);
	clone->SetRenderPass(m_renderPass);

	return clone;
}


// Helper functions for setting up render-thread material data
namespace
{

void SetupCBuffer(RenderThread::MaterialData& materialData, size_t cbufferSizeInBytes)
{
	// Create an uber-cbuffer for the entire material
	auto cbuffer = make_shared<ConstantBuffer>();
	cbuffer->Create(cbufferSizeInBytes, Usage::Dynamic);
	materialData.cbuffer = cbuffer;

	// CPU-side memory mirror
	if (materialData.cbufferData)
	{
		_aligned_free(materialData.cbufferData);
		materialData.cbufferData = nullptr;
	}
	materialData.cbufferData = (byte*)_aligned_malloc(cbufferSizeInBytes, 16);
}


void SetupBinding(Material* material, RenderThread::MaterialData::CBufferBinding& binding, ID3D11Buffer* d3dBuffer, 
	const vector<ShaderReflection::CBVLayout>& effectCBuffers, uint32_t index)
{
	uint32_t minSlot = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;
	uint32_t numBuffers = static_cast<uint32_t>(effectCBuffers.size());

	if (numBuffers > 0)
	{
		// Initialize cbuffer binding record with default values
		fill(begin(binding.cbuffers), end(binding.cbuffers), nullptr);
		fill(begin(binding.firstConstant), end(binding.firstConstant), 0);
		fill(begin(binding.numConstants), end(binding.numConstants), 0);

		// Figure out the starting shader register slot
		for (const auto& effectCBuffer : effectCBuffers)
		{
			minSlot = min(minSlot, effectCBuffer.shaderRegister);
		}

		auto materialData = material->GetRenderThreadData();
		byte* baseDestination = materialData->cbufferData;

		// Fill out the cbuffer binding record
		for (const auto& effectCBuffer : effectCBuffers)
		{
			const auto index = effectCBuffer.shaderRegister - minSlot;

			binding.cbuffers[index] = d3dBuffer;
			binding.firstConstant[index] = effectCBuffer.byteOffset;
			binding.numConstants[index] = effectCBuffer.sizeInBytes;

			auto cbuffer = material->GetConstantBuffer(effectCBuffer.name);
			cbuffer->CreateRenderThreadData(index, effectCBuffer.sizeInBytes, baseDestination + effectCBuffer.byteOffset);
		}
	}
	
	binding.startSlot = minSlot;
	binding.numBuffers = numBuffers;
}


void SetupShaderCBufferBindings(Material* material, RenderThread::MaterialData& materialData, const Effect::Signature& effectSig, ShaderType shaderType)
{
	const uint32_t index = static_cast<uint32_t>(shaderType);

	auto& binding = materialData.cbufferBindings[index];

	const auto& effectCBuffers = effectSig.cbvBindings[index];

	auto d3dBuffer = materialData.cbuffer->constantBuffer.Get();
	SetupBinding(material, binding, d3dBuffer, effectCBuffers, index);
}

} // anonymous namespace


void Material::CreateRenderThreadData()
{
	const auto& effectSig = m_effect->GetSignature();

	m_renderThreadData = make_shared<RenderThread::MaterialData>();
	m_renderThreadData->renderPass = m_renderPass;
	m_renderThreadData->pso = m_effect->GetPSO();

	auto& materialData = *m_renderThreadData;

	// Setup the DX11 cbuffer and bindings per shader stage
	materialData.cbufferSize = effectSig.cbvPerMaterialDataSize;
	if (materialData.cbufferSize > 0)
	{
		SetupCBuffer(materialData, materialData.cbufferSize);

		SetupShaderCBufferBindings(this, materialData, effectSig, ShaderType::Vertex);
		SetupShaderCBufferBindings(this, materialData, effectSig, ShaderType::Domain);
		SetupShaderCBufferBindings(this, materialData, effectSig, ShaderType::Hull);
		SetupShaderCBufferBindings(this, materialData, effectSig, ShaderType::Geometry);
		SetupShaderCBufferBindings(this, materialData, effectSig, ShaderType::Pixel);
	}
	
	// Table bindings
	for (uint32_t i = 0; i < 5; ++i)
	{
		// SRV tables
		for (const auto& fxLayout : effectSig.srvBindings[i])
		{
			RenderThread::MaterialData::ResourceTable<ID3D11ShaderResourceView>::TableLayout layout;
			layout.shaderRegister = fxLayout.shaderRegister;
			layout.numItems = fxLayout.numItems;
			layout.resources.reserve(layout.numItems);
			layout.resources.insert(layout.resources.end(), layout.numItems, nullptr);

			m_renderThreadData->srvTables[i].layouts.push_back(layout);
		}

		// UAV tables
		for (const auto& fxLayout : effectSig.uavBindings[i])
		{
			RenderThread::MaterialData::ResourceTable<ID3D11UnorderedAccessView>::TableLayout layout;
			layout.shaderRegister = fxLayout.shaderRegister;
			layout.numItems = fxLayout.numItems;
			layout.resources.reserve(layout.numItems);
			layout.resources.insert(layout.resources.end(), layout.numItems, nullptr);

			m_renderThreadData->uavTables[i].layouts.push_back(layout);
		}

		// Sampler tables
		for (const auto& fxLayout : effectSig.samplerBindings[i])
		{
			RenderThread::MaterialData::ResourceTable<ID3D11SamplerState>::TableLayout layout;
			layout.shaderRegister = fxLayout.shaderRegister;
			layout.numItems = fxLayout.numItems;
			layout.resources.reserve(layout.numItems);
			layout.resources.insert(layout.resources.end(), layout.numItems, nullptr);

			m_renderThreadData->samplerTables[i].layouts.push_back(layout);
		}
	}
	
	// Parameters
	for (const auto& parameter : effectSig.parameters)
	{
		auto materialParameter = GetParameter(parameter.second.name);
		materialParameter->CreateRenderThreadData(m_renderThreadData, parameter.second);
	}

	// Resource SRVs
	for (const auto& resource : effectSig.srvs)
	{
		auto materialResource = GetResource(resource.second.name);
		materialResource->CreateRenderThreadData(m_renderThreadData, resource.second);
	}

	// Resource UAVs
	for (const auto& uav : effectSig.uavs)
	{
		auto materialUAV = GetResource(uav.second.name);
		materialUAV->CreateRenderThreadData(m_renderThreadData, uav.second);
	}

	// TODO: Samplers
}


void RenderThread::MaterialData::Update(GraphicsCommandList& commandList)
{
	if (cbufferDirty && cbufferSize > 0)
	{
		auto dest = commandList.MapConstants(*cbuffer);
		memcpy(dest, cbufferData, cbufferSize);
		commandList.UnmapConstants(*cbuffer);

		cbufferDirty = false;
	}
}


void RenderThread::MaterialData::Commit(GraphicsCommandList& commandList)
{
	// Set the PSO for this material
	commandList.SetPipelineState(*pso);

	const auto& materialData = *this;

	// VS
	uint32_t shaderIndex = static_cast<uint32_t>(ShaderType::Vertex);
	if (cbufferBindings[shaderIndex].numBuffers > 0)
	{
		commandList.SetVertexShaderConstants(
			cbufferBindings[shaderIndex].startSlot,
			cbufferBindings[shaderIndex].numBuffers,
			&cbufferBindings[shaderIndex].cbuffers[0],
			&cbufferBindings[shaderIndex].firstConstant[0],
			&cbufferBindings[shaderIndex].numConstants[0]);
	}

	// HS
	shaderIndex = static_cast<uint32_t>(ShaderType::Hull);
	if (cbufferBindings[shaderIndex].numBuffers > 0)
	{
		commandList.SetHullShaderConstants(
			cbufferBindings[shaderIndex].startSlot,
			cbufferBindings[shaderIndex].numBuffers,
			&cbufferBindings[shaderIndex].cbuffers[0],
			&cbufferBindings[shaderIndex].firstConstant[0],
			&cbufferBindings[shaderIndex].numConstants[0]);
	}

	// DS
	shaderIndex = static_cast<uint32_t>(ShaderType::Domain);
	if (cbufferBindings[shaderIndex].numBuffers > 0)
	{
		commandList.SetDomainShaderConstants(
			cbufferBindings[shaderIndex].startSlot,
			cbufferBindings[shaderIndex].numBuffers,
			&cbufferBindings[shaderIndex].cbuffers[0],
			&cbufferBindings[shaderIndex].firstConstant[0],
			&cbufferBindings[shaderIndex].numConstants[0]);
	}

	// GS
	shaderIndex = static_cast<uint32_t>(ShaderType::Geometry);
	if (cbufferBindings[shaderIndex].numBuffers > 0)
	{
		commandList.SetGeometryShaderConstants(
			cbufferBindings[shaderIndex].startSlot,
			cbufferBindings[shaderIndex].numBuffers,
			&cbufferBindings[shaderIndex].cbuffers[0],
			&cbufferBindings[shaderIndex].firstConstant[0],
			&cbufferBindings[shaderIndex].numConstants[0]);
	}

	// PS
	shaderIndex = static_cast<uint32_t>(ShaderType::Pixel);
	if (cbufferBindings[shaderIndex].numBuffers > 0)
	{
		commandList.SetPixelShaderConstants(
			cbufferBindings[shaderIndex].startSlot,
			cbufferBindings[shaderIndex].numBuffers,
			&cbufferBindings[shaderIndex].cbuffers[0],
			&cbufferBindings[shaderIndex].firstConstant[0],
			&cbufferBindings[shaderIndex].numConstants[0]);
	}

	// VS
	shaderIndex = static_cast<uint32_t>(ShaderType::Vertex);
	for (const auto& layout : srvTables[shaderIndex].layouts)
	{
		commandList.SetVertexShaderResources(layout.shaderRegister, layout.numItems, &layout.resources[0]);
	}

	// HS
	shaderIndex = static_cast<uint32_t>(ShaderType::Hull);
	for (const auto& layout : srvTables[shaderIndex].layouts)
	{
		commandList.SetHullShaderResources(layout.shaderRegister, layout.numItems, &layout.resources[0]);
	}

	// DS
	shaderIndex = static_cast<uint32_t>(ShaderType::Domain);
	for (const auto& layout : srvTables[shaderIndex].layouts)
	{
		commandList.SetDomainShaderResources(layout.shaderRegister, layout.numItems, &layout.resources[0]);
	}

	// GS
	shaderIndex = static_cast<uint32_t>(ShaderType::Geometry);
	for (const auto& layout : srvTables[shaderIndex].layouts)
	{
		commandList.SetGeometryShaderResources(layout.shaderRegister, layout.numItems, &layout.resources[0]);
	}

	// PS
	shaderIndex = static_cast<uint32_t>(ShaderType::Pixel);
	for (const auto& layout : srvTables[shaderIndex].layouts)
	{
		commandList.SetPixelShaderResources(layout.shaderRegister, layout.numItems, &layout.resources[0]);
	}
}