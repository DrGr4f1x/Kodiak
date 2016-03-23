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
#include "MathUtil.h"
#include "PipelineState.h"
#include "Renderer.h"
#include "RenderEnums.h"
#include "RenderPass.h"
#include "Texture.h"


using namespace Kodiak;
using namespace std;
using namespace DirectX;


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


// Helper functions for setting up render-thread material data
namespace
{
	
void SetupDefaultCBufferCallbacks(RenderThread::MaterialData& materialData)
{
	materialData.cbufferCallbacks[0] = [](const RenderThread::MaterialData& materialData, GraphicsCommandList& commandList) {};
	materialData.cbufferCallbacks[1] = [](const RenderThread::MaterialData& materialData, GraphicsCommandList& commandList) {};
	materialData.cbufferCallbacks[2] = [](const RenderThread::MaterialData& materialData, GraphicsCommandList& commandList) {};
	materialData.cbufferCallbacks[3] = [](const RenderThread::MaterialData& materialData, GraphicsCommandList& commandList) {};
	materialData.cbufferCallbacks[4] = [](const RenderThread::MaterialData& materialData, GraphicsCommandList& commandList) {};
}


void SetupResourceCallbacks(RenderThread::MaterialData& materialData)
{
	materialData.resourceCallbacks[0] = [](const RenderThread::MaterialData& materialData, GraphicsCommandList& commandList)
	{
		for (const auto& binding : materialData.resourceBindings[0].resourceRanges)
		{
			commandList.SetVertexShaderResources(binding.startSlot, binding.numResources, &binding.resources[0]);
		}
	};

	materialData.resourceCallbacks[1] = [](const RenderThread::MaterialData& materialData, GraphicsCommandList& commandList)
	{
		for (const auto& binding : materialData.resourceBindings[1].resourceRanges)
		{
			commandList.SetHullShaderResources(binding.startSlot, binding.numResources, &binding.resources[0]);
		}
	};

	materialData.resourceCallbacks[2] = [](const RenderThread::MaterialData& materialData, GraphicsCommandList& commandList)
	{
		for (const auto& binding : materialData.resourceBindings[2].resourceRanges)
		{
			commandList.SetDomainShaderResources(binding.startSlot, binding.numResources, &binding.resources[0]);
		}
	};

	materialData.resourceCallbacks[3] = [](const RenderThread::MaterialData& materialData, GraphicsCommandList& commandList)
	{
		for (const auto& binding : materialData.resourceBindings[3].resourceRanges)
		{
			commandList.SetGeometryShaderResources(binding.startSlot, binding.numResources, &binding.resources[0]);
		}
	};

	materialData.resourceCallbacks[4] = [](const RenderThread::MaterialData& materialData, GraphicsCommandList& commandList)
	{
		for (const auto& binding : materialData.resourceBindings[4].resourceRanges)
		{
			commandList.SetPixelShaderResources(binding.startSlot, binding.numResources, &binding.resources[0]);
		}
	};
}


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
	materialData.cbufferData = (uint8_t*)_aligned_malloc(cbufferSizeInBytes, 16);
}


void SetupBinding(uint32_t& constantOffset, RenderThread::MaterialData::CBufferBinding& binding, ID3D11Buffer* d3dBuffer, const std::vector<EffectConstantBuffer>& effectCBuffers)
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

		// Fill out the cbuffer binding record
		for (const auto& effectCBuffer : effectCBuffers)
		{
			const auto index = effectCBuffer.shaderRegister - minSlot;

			binding.cbuffers[index] = d3dBuffer;
			binding.firstConstant[index] = 16 * constantOffset;
			binding.numConstants[index] = static_cast<uint32_t>(Math::AlignUp(effectCBuffer.size, 16));
			constantOffset += binding.numConstants[index];
		}
	}
	
	binding.startSlot = minSlot;
	binding.numBuffers = numBuffers;
}


void SetupShaderCBufferBindings(uint32_t& constantOffset, RenderThread::MaterialData& materialData, const EffectSignature& effectSig, ShaderType shaderType)
{
	const uint32_t index = static_cast<uint32_t>(shaderType);

	auto& binding = materialData.cbufferBindings[index];

	const auto& effectCBuffers = effectSig.constantBuffers[index];

	auto d3dBuffer = materialData.cbuffer->constantBuffer.Get();
	SetupBinding(constantOffset, binding, d3dBuffer, effectCBuffers);

	// Setup callback
	if (binding.numBuffers > 0)
	{
		switch (shaderType)
		{
		case ShaderType::Vertex:
			materialData.cbufferCallbacks[index] = [](const RenderThread::MaterialData& materialData, GraphicsCommandList& commandList)
			{
				const auto& b = materialData.cbufferBindings[0];
				commandList.SetVertexShaderConstants(b.startSlot, b.numBuffers, &b.cbuffers[0], &b.firstConstant[0], &b.numConstants[0]);
			};
			break;

		case ShaderType::Domain:
			materialData.cbufferCallbacks[index] = [](const RenderThread::MaterialData& materialData, GraphicsCommandList& commandList)
			{
				const auto& b = materialData.cbufferBindings[1];
				commandList.SetDomainShaderConstants(b.startSlot, b.numBuffers, &b.cbuffers[0], &b.firstConstant[0], &b.numConstants[0]);
			};
			break;

		case ShaderType::Hull:
			materialData.cbufferCallbacks[index] = [](const RenderThread::MaterialData& materialData, GraphicsCommandList& commandList)
			{
				const auto& b = materialData.cbufferBindings[2];
				commandList.SetHullShaderConstants(b.startSlot, b.numBuffers, &b.cbuffers[0], &b.firstConstant[0], &b.numConstants[0]);
			};
			break;

		case ShaderType::Geometry:
			materialData.cbufferCallbacks[index] = [](const RenderThread::MaterialData& materialData, GraphicsCommandList& commandList)
			{
				const auto& b = materialData.cbufferBindings[3];
				commandList.SetGeometryShaderConstants(b.startSlot, b.numBuffers, &b.cbuffers[0], &b.firstConstant[0], &b.numConstants[0]);
			};
			break;

		case ShaderType::Pixel:
			materialData.cbufferCallbacks[index] = [](const RenderThread::MaterialData& materialData, GraphicsCommandList& commandList)
			{
				const auto& b = materialData.cbufferBindings[4];
				commandList.SetPixelShaderConstants(b.startSlot, b.numBuffers, &b.cbuffers[0], &b.firstConstant[0], &b.numConstants[0]);
			};
			break;
		}
	}
}

} // anonymous namespace


void Material::CreateRenderThreadData()
{
	const auto& effectSig = m_effect->GetSignature();

	m_renderThreadData = make_shared<RenderThread::MaterialData>();
	m_renderThreadData->renderPass = m_renderPass;
	m_renderThreadData->pso = m_effect->GetPSO();

	auto& materialData = *m_renderThreadData;

	SetupDefaultCBufferCallbacks(materialData);
	
	// Setup the DX11 cbuffer and bindings per shader stage
	materialData.cbufferSize = effectSig.perMaterialDataSize;
	if (materialData.cbufferSize > 0)
	{
		SetupCBuffer(materialData, materialData.cbufferSize);

		uint32_t constantOffset = 0;
		SetupShaderCBufferBindings(constantOffset, materialData, effectSig, ShaderType::Vertex);
		SetupShaderCBufferBindings(constantOffset, materialData, effectSig, ShaderType::Domain);
		SetupShaderCBufferBindings(constantOffset, materialData, effectSig, ShaderType::Hull);
		SetupShaderCBufferBindings(constantOffset, materialData, effectSig, ShaderType::Geometry);
		SetupShaderCBufferBindings(constantOffset, materialData, effectSig, ShaderType::Pixel);
	}
	
	// Bind parameters
	{
		lock_guard<mutex> CS(m_parameterLock);

		for (const auto& effectVar : effectSig.variables)
		{
			shared_ptr<MaterialParameter> materialParam;
			bool copyData = false;

			auto it = m_parameters.find(effectVar.name);
			if (end(m_parameters) != it)
			{
				materialParam = it->second;
				copyData = true;
			}
			else
			{
				materialParam = make_shared<MaterialParameter>(effectVar.name);
			}
			
			// Render thread data
			materialParam->m_renderThreadData = make_shared<RenderThread::MaterialParameterData>(m_renderThreadData);
			auto& materialParamData = *materialParam->m_renderThreadData;
			materialParamData.m_type = effectVar.type;

			// Bindings into cbuffer memory
			for (uint32_t i = 0; i < 5; ++i)
			{
				const auto& effectBinding = effectVar.shaderSlots[i];

				if (effectBinding.cbufferIndex != 0xFFFFFFFF)
				{
					assert(materialData.cbufferBindings[i].numBuffers >= effectBinding.cbufferIndex);
					assert(materialData.cbufferBindings[i].numConstants[effectBinding.cbufferIndex] != 0);

					materialParamData.m_size = effectVar.size;

					// Compute destination address for this parameter in the CPU-side memory store for the buffer
					materialParamData.m_bindings[i] =
						materialData.cbufferData +													// Base address
						materialData.cbufferBindings[i].firstConstant[effectBinding.cbufferIndex] + // Offset to start of logical cbuffer
						effectVar.shaderSlots[i].offset;											// Offset to param within logical cbuffer
					materialParamData.m_dirtyFlag = &materialData.cbufferDirty;
				}
				else
				{
					materialParamData.m_size = 0;
					materialParamData.m_bindings[i] = nullptr;
					materialParamData.m_dirtyFlag = nullptr;
				}
			}

			// TODO: make a MaterialParameter::CreateRenderThreadData() method to do this
			if (copyData)
			{
				materialParamData.m_data = materialParam->m_data;
				for (uint32_t i = 0; i < 5; ++i)
				{
					if (materialParamData.m_bindings[i] != nullptr)
					{
						memcpy(materialParamData.m_bindings[i], &materialParamData.m_data[0], materialParamData.m_size);
					}
				}
				*materialParamData.m_dirtyFlag = true;
			}

			m_parameters[materialParam->GetName()] = materialParam;
		}
	}

	// Resource ranges
	for (uint32_t i = 0; i < 5; ++i)
	{
		materialData.resourceBindings[i].resourceRanges.clear();
		for (const auto& effectResourceRange : effectSig.resourceBindings[i].resourceRanges)
		{
			RenderThread::MaterialData::ResourceBinding::ResourceRange range;

			range.startSlot = effectResourceRange.startSlot;
			range.numResources = effectResourceRange.numResources;

			range.resources.reserve(range.numResources);
			for (uint32_t j = 0; j < range.numResources; ++j)
			{
				range.resources.push_back(nullptr);
			}

			materialData.resourceBindings[i].resourceRanges.push_back(range);
		}
	}

	// Must do this here, after ranges have been set up
	SetupResourceCallbacks(materialData);

	// Bind resources
	{
		lock_guard<mutex> CS(m_resourceLock);

		for (const auto& effectRes : effectSig.resources)
		{
			shared_ptr<MaterialResource> materialRes;
			bool copyResource = false;

			auto it = m_resources.find(effectRes.name);
			if (end(m_resources) != it)
			{
				materialRes = it->second;
				copyResource = true;
			}
			else
			{
				materialRes = make_shared<MaterialResource>(effectRes.name);
			}

			materialRes->m_renderThreadData = make_shared<RenderThread::MaterialResourceData>(m_renderThreadData);

			materialRes->m_shaderSlots = effectRes.shaderSlots;
			
			// Map the resource destination ranges
			for (uint32_t i = 0; i < 5; ++i)
			{
				const auto slot = materialRes->m_shaderSlots[i];

				if (slot != 0xFFFFFFFF)
				{
					// Find the appropriate destination resource range for this shader stage
					bool found = false;
					uint32_t rangeIndex = 0;
					for (const auto& range : materialData.resourceBindings[i].resourceRanges)
					{
						if (slot >= range.startSlot && slot < (range.startSlot + range.numResources))
						{
							const uint32_t resourceIndex = slot - range.startSlot;
							materialRes->m_renderThreadData->BindDestination(i, rangeIndex, resourceIndex);
							found = true;
							break;
						}
						++rangeIndex;
					}
					assert(found);
				}
				else
				{
					// Default range (resource not bound to this shader stage)
					materialRes->m_renderThreadData->BindDestination(i, 0xFFFFFFFF, 0xFFFFFFFF);
				}
			}

			// TODO: make a MaterialResource::CreateRenderThreadData() method to do this
			if (materialRes->m_texture)
			{
				materialRes->SetResource(materialRes->m_texture);
			}

			m_resources[materialRes->GetName()] = materialRes;
		}
	}
}


MaterialParameter::MaterialParameter(const string& name) 
	: m_name(name) 
{
	ZeroMemory(&m_data[0], 64);
}


void MaterialParameter::SetValue(bool value)
{
	memcpy(&m_data[0], &value, sizeof(bool));

	_ReadWriteBarrier();

	if (m_renderThreadData)
	{
		auto renderThreadData = m_renderThreadData;
		Renderer::GetInstance().EnqueueTask([renderThreadData, value](RenderTaskEnvironment& rte)
		{
			renderThreadData->SetValue(value);
		});
	}
}


void MaterialParameter::SetValue(int32_t value)
{
	memcpy(&m_data[0], &value, sizeof(int32_t));

	_ReadWriteBarrier();

	if (m_renderThreadData)
	{
		auto renderThreadData = m_renderThreadData;
		Renderer::GetInstance().EnqueueTask([renderThreadData, value](RenderTaskEnvironment& rte)
		{
			renderThreadData->SetValue(value);
		});
	}
}


void MaterialParameter::SetValue(XMINT2 value)
{
	memcpy(&m_data[0], &value, sizeof(XMINT2));

	_ReadWriteBarrier();

	if (m_renderThreadData)
	{
		auto renderThreadData = m_renderThreadData;
		Renderer::GetInstance().EnqueueTask([renderThreadData, value](RenderTaskEnvironment& rte)
		{
			renderThreadData->SetValue(value);
		});
	}
}


void MaterialParameter::SetValue(XMINT3 value)
{
	memcpy(&m_data[0], &value, sizeof(XMINT3));

	_ReadWriteBarrier();

	if (m_renderThreadData)
	{
		auto renderThreadData = m_renderThreadData;
		Renderer::GetInstance().EnqueueTask([renderThreadData, value](RenderTaskEnvironment& rte)
		{
			renderThreadData->SetValue(value);
		});
	}
}


void MaterialParameter::SetValue(XMINT4 value)
{
	memcpy(&m_data[0], &value, sizeof(XMINT4));

	_ReadWriteBarrier();

	if (m_renderThreadData)
	{
		auto renderThreadData = m_renderThreadData;
		Renderer::GetInstance().EnqueueTask([renderThreadData, value](RenderTaskEnvironment& rte)
		{
			renderThreadData->SetValue(value);
		});
	}
}


void MaterialParameter::SetValue(uint32_t value)
{
	memcpy(&m_data[0], &value, sizeof(uint32_t));

	_ReadWriteBarrier();

	if (m_renderThreadData)
	{
		auto renderThreadData = m_renderThreadData;
		Renderer::GetInstance().EnqueueTask([renderThreadData, value](RenderTaskEnvironment& rte)
		{
			renderThreadData->SetValue(value);
		});
	}
}


void MaterialParameter::SetValue(XMUINT2 value)
{
	memcpy(&m_data[0], &value, sizeof(XMUINT2));

	_ReadWriteBarrier();

	if (m_renderThreadData)
	{
		auto renderThreadData = m_renderThreadData;
		Renderer::GetInstance().EnqueueTask([renderThreadData, value](RenderTaskEnvironment& rte)
		{
			renderThreadData->SetValue(value);
		});
	}
}


void MaterialParameter::SetValue(XMUINT3 value)
{
	memcpy(&m_data[0], &value, sizeof(XMUINT3));

	_ReadWriteBarrier();

	if (m_renderThreadData)
	{
		auto renderThreadData = m_renderThreadData;
		Renderer::GetInstance().EnqueueTask([renderThreadData, value](RenderTaskEnvironment& rte)
		{
			renderThreadData->SetValue(value);
		});
	}
}


void MaterialParameter::SetValue(XMUINT4 value)
{
	memcpy(&m_data[0], &value, sizeof(XMUINT4));

	_ReadWriteBarrier();

	if (m_renderThreadData)
	{
		auto renderThreadData = m_renderThreadData;
		Renderer::GetInstance().EnqueueTask([renderThreadData, value](RenderTaskEnvironment& rte)
		{
			renderThreadData->SetValue(value);
		});
	}
}


void MaterialParameter::SetValue(float value)
{
	memcpy(&m_data[0], &value, sizeof(float));

	_ReadWriteBarrier();

	if (m_renderThreadData)
	{
		auto renderThreadData = m_renderThreadData;
		Renderer::GetInstance().EnqueueTask([renderThreadData, value](RenderTaskEnvironment& rte)
		{
			renderThreadData->SetValue(value);
		});
	}
}


void MaterialParameter::SetValue(XMFLOAT2 value)
{
	memcpy(&m_data[0], &value, sizeof(XMFLOAT2));

	_ReadWriteBarrier();

	if (m_renderThreadData)
	{
		auto renderThreadData = m_renderThreadData;
		Renderer::GetInstance().EnqueueTask([renderThreadData, value](RenderTaskEnvironment& rte)
		{
			renderThreadData->SetValue(value);
		});
	}
}


void MaterialParameter::SetValue(XMFLOAT3 value)
{
	memcpy(&m_data[0], &value, sizeof(XMFLOAT3));

	_ReadWriteBarrier();

	if (m_renderThreadData)
	{
		auto renderThreadData = m_renderThreadData;
		Renderer::GetInstance().EnqueueTask([renderThreadData, value](RenderTaskEnvironment& rte)
		{
			renderThreadData->SetValue(value);
		});
	}
}


void MaterialParameter::SetValue(XMFLOAT4 value)
{
	memcpy(&m_data[0], &value, sizeof(XMFLOAT4));

	_ReadWriteBarrier();

	if (m_renderThreadData)
	{
		auto renderThreadData = m_renderThreadData;
		Renderer::GetInstance().EnqueueTask([renderThreadData, value](RenderTaskEnvironment& rte)
		{
			renderThreadData->SetValue(value);
		});
	}
}


void MaterialParameter::SetValue(XMFLOAT4X4 value)
{
	memcpy(&m_data[0], &value, sizeof(XMFLOAT4X4));

	_ReadWriteBarrier();

	if (m_renderThreadData)
	{
		auto renderThreadData = m_renderThreadData;
		Renderer::GetInstance().EnqueueTask([renderThreadData, value](RenderTaskEnvironment& rte)
		{
			renderThreadData->SetValue(value);
		});
	}
}


MaterialResource::MaterialResource(const string& name)
	: m_name(name)
	, m_shaderSlots({0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF})
	, m_type(ShaderResourceType::Unsupported)
	, m_dimension(ShaderResourceDimension::Unsupported)
	, m_texture(nullptr)
{}


void MaterialResource::SetResource(shared_ptr<Texture> texture)
{
	m_texture = texture;

	_ReadWriteBarrier();

	if(m_renderThreadData)
	{ 
		auto renderThreadData = m_renderThreadData;

		if (m_texture)
		{
			m_texture->loadTask.then([renderThreadData, texture]
			{
				Renderer::GetInstance().EnqueueTask([renderThreadData, texture](RenderTaskEnvironment& rte)
				{
					if (texture)
					{
						renderThreadData->SetResource(texture->GetSRV());
					}
					else
					{
						renderThreadData->SetResource(nullptr);
					}
				});
			});
		}
		else 
		{
			Renderer::GetInstance().EnqueueTask([renderThreadData, texture](RenderTaskEnvironment& rte)
			{
				if (texture)
				{
					renderThreadData->SetResource(texture->GetSRV());
				}
				else
				{
					renderThreadData->SetResource(nullptr);
				}
			});
		}
	}
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

	// Execute cbuffer binding callbacks
	cbufferCallbacks[0](materialData, commandList); // VS
	cbufferCallbacks[1](materialData, commandList); // DS
	cbufferCallbacks[2](materialData, commandList); // HS
	cbufferCallbacks[3](materialData, commandList); // GS
	cbufferCallbacks[4](materialData, commandList); // PS

	// Execute resource binding callbacks
	resourceCallbacks[0](materialData, commandList); // VS
	resourceCallbacks[1](materialData, commandList); // DS
	resourceCallbacks[2](materialData, commandList); // HS
	resourceCallbacks[3](materialData, commandList); // GS
	resourceCallbacks[4](materialData, commandList); // PS
}


RenderThread::MaterialParameterData::MaterialParameterData(shared_ptr<RenderThread::MaterialData> materialData)
	: m_data()
	, m_type(ShaderVariableType::Unsupported)
	, m_materialData(materialData)
	, m_bindings({ nullptr, nullptr, nullptr, nullptr, nullptr })
	, m_dirtyFlag(nullptr)
{
	ZeroMemory(&m_data[0], 64);
}


void RenderThread::MaterialParameterData::SetValue(bool value)
{
	if (auto materialData = m_materialData.lock())
	{
		assert(m_type == ShaderVariableType::Bool);
		memcpy(&m_data[0], &value, sizeof(bool));

		for (uint32_t i = 0; i < 5; ++i)
		{
			if (m_bindings[i])
			{
				memcpy(m_bindings[i], &m_data[0], sizeof(bool));
				*m_dirtyFlag = true;
			}
		}
	}
}


void RenderThread::MaterialParameterData::SetValue(int32_t value)
{
	if (auto materialData = m_materialData.lock())
	{
		assert(m_type == ShaderVariableType::Int);
		InternalSetValue(value);
	}
}


void RenderThread::MaterialParameterData::SetValue(XMINT2 value)
{
	if (auto materialData = m_materialData.lock())
	{
		assert(m_type == ShaderVariableType::Int2);
		InternalSetValue(value);
	}
}


void RenderThread::MaterialParameterData::SetValue(XMINT3 value)
{
	if (auto materialData = m_materialData.lock())
	{
		assert(m_type == ShaderVariableType::Int3);
		InternalSetValue(value);
	}
}


void RenderThread::MaterialParameterData::SetValue(XMINT4 value)
{
	if (auto materialData = m_materialData.lock())
	{
		assert(m_type == ShaderVariableType::Int4);
		InternalSetValue(value);
	}
}


void RenderThread::MaterialParameterData::SetValue(uint32_t value)
{
	if (auto materialData = m_materialData.lock())
	{
		assert(m_type == ShaderVariableType::UInt);
		InternalSetValue(value);
	}
}


void RenderThread::MaterialParameterData::SetValue(XMUINT2 value)
{
	if (auto materialData = m_materialData.lock())
	{
		assert(m_type == ShaderVariableType::UInt2);
		InternalSetValue(value);
	}
}


void RenderThread::MaterialParameterData::SetValue(XMUINT3 value)
{
	if (auto materialData = m_materialData.lock())
	{
		assert(m_type == ShaderVariableType::UInt3);
		InternalSetValue(value);
	}
}


void RenderThread::MaterialParameterData::SetValue(XMUINT4 value)
{
	if (auto materialData = m_materialData.lock())
	{
		assert(m_type == ShaderVariableType::UInt4);
		InternalSetValue(value);
	}
}


void RenderThread::MaterialParameterData::SetValue(float value)
{
	if (auto materialData = m_materialData.lock())
	{
		assert(m_type == ShaderVariableType::Float);
		InternalSetValue(value);
	}
}


void RenderThread::MaterialParameterData::SetValue(XMFLOAT2 value)
{
	if (auto materialData = m_materialData.lock())
	{
		assert(m_type == ShaderVariableType::Float2);
		InternalSetValue(value);
	}
}


void RenderThread::MaterialParameterData::SetValue(XMFLOAT3 value)
{
	if (auto materialData = m_materialData.lock())
	{
		assert(m_type == ShaderVariableType::Float3);
		InternalSetValue(value);
	}
}


void RenderThread::MaterialParameterData::SetValue(XMFLOAT4 value)
{
	if (auto materialData = m_materialData.lock())
	{
		assert(m_type == ShaderVariableType::Float4);
		InternalSetValue(value);
	}
}


void RenderThread::MaterialParameterData::SetValue(XMFLOAT4X4 value)
{
	if (auto materialData = m_materialData.lock())
	{
		assert(m_type == ShaderVariableType::Float4x4);
		InternalSetValue(value);
	}
}


template <typename T>
void RenderThread::MaterialParameterData::InternalSetValue(T value)
{
	memcpy(&m_data[0], &value, sizeof(T));

	for (uint32_t i = 0; i < 5; ++i)
	{
		if (m_bindings[i])
		{
			memcpy(m_bindings[i], &m_data[0], sizeof(T));
			*(m_dirtyFlag) = true;
		}
	}
}


RenderThread::MaterialResourceData::MaterialResourceData(shared_ptr<RenderThread::MaterialData> materialData)
	: m_materialData(materialData)
{}


void RenderThread::MaterialResourceData::SetResource(ID3D11ShaderResourceView* srv)
{
	if (auto materialData = m_materialData.lock())
	{
		if (m_srv.Get() != srv)
		{
			m_srv = srv;

			for (uint32_t i = 0; i < 5; ++i)
			{
				const auto& range = m_shaderSlots[i];
				if (range.first != 0xFFFFFFFF)
				{
					materialData->resourceBindings[i].resourceRanges[range.first].resources[range.second] = srv;
				}
			}
		}
	}
}


void RenderThread::MaterialResourceData::BindDestination(uint32_t shaderIndex, uint32_t rangeIndex, uint32_t resourceIndex)
{
	m_shaderSlots[shaderIndex].first = rangeIndex;
	m_shaderSlots[shaderIndex].second = resourceIndex;
}