// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "MaterialParameter11.h"

#include "Material11.h"
#include "RenderEnums.h"
#include "Renderer.h"

using namespace Kodiak;
using namespace DirectX;
using namespace std;


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


void MaterialParameter::CreateRenderThreadData(std::shared_ptr<RenderThread::MaterialData> materialData, const ShaderReflection::Parameter<5>& parameter)
{
	m_renderThreadData = make_shared<RenderThread::MaterialParameterData>(materialData);

	m_renderThreadData->m_type = parameter.type;
	m_renderThreadData->m_data = m_data;

	for (uint32_t i = 0; i < 5; ++i)
	{
		if (parameter.byteOffset[i] != kInvalid)
		{
			m_renderThreadData->m_bindings[i] = materialData->cbufferData + parameter.byteOffset[i];

			memcpy(m_renderThreadData->m_bindings[i], &m_data[0], parameter.sizeInBytes);
			materialData->cbufferDirty = true;
		}
		else
		{
			m_renderThreadData->m_bindings[i] = nullptr;
		}
	}
}


RenderThread::MaterialParameterData::MaterialParameterData(shared_ptr<RenderThread::MaterialData> materialData)
	: m_data()
	, m_type(ShaderVariableType::Unsupported)
	, m_materialData(materialData)
	, m_bindings({ nullptr, nullptr, nullptr, nullptr, nullptr })
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
			}
		}
		materialData->cbufferDirty = true;
	}
}


void RenderThread::MaterialParameterData::SetValue(int32_t value)
{
	if (auto materialData = m_materialData.lock())
	{
		assert(m_type == ShaderVariableType::Int);
		InternalSetValue(value);
		materialData->cbufferDirty = true;
	}
}


void RenderThread::MaterialParameterData::SetValue(XMINT2 value)
{
	if (auto materialData = m_materialData.lock())
	{
		assert(m_type == ShaderVariableType::Int2);
		InternalSetValue(value);
		materialData->cbufferDirty = true;
	}
}


void RenderThread::MaterialParameterData::SetValue(XMINT3 value)
{
	if (auto materialData = m_materialData.lock())
	{
		assert(m_type == ShaderVariableType::Int3);
		InternalSetValue(value);
		materialData->cbufferDirty = true;
	}
}


void RenderThread::MaterialParameterData::SetValue(XMINT4 value)
{
	if (auto materialData = m_materialData.lock())
	{
		assert(m_type == ShaderVariableType::Int4);
		InternalSetValue(value);
		materialData->cbufferDirty = true;
	}
}


void RenderThread::MaterialParameterData::SetValue(uint32_t value)
{
	if (auto materialData = m_materialData.lock())
	{
		assert(m_type == ShaderVariableType::UInt);
		InternalSetValue(value);
		materialData->cbufferDirty = true;
	}
}


void RenderThread::MaterialParameterData::SetValue(XMUINT2 value)
{
	if (auto materialData = m_materialData.lock())
	{
		assert(m_type == ShaderVariableType::UInt2);
		InternalSetValue(value);
		materialData->cbufferDirty = true;
	}
}


void RenderThread::MaterialParameterData::SetValue(XMUINT3 value)
{
	if (auto materialData = m_materialData.lock())
	{
		assert(m_type == ShaderVariableType::UInt3);
		InternalSetValue(value);
		materialData->cbufferDirty = true;
	}
}


void RenderThread::MaterialParameterData::SetValue(XMUINT4 value)
{
	if (auto materialData = m_materialData.lock())
	{
		assert(m_type == ShaderVariableType::UInt4);
		InternalSetValue(value);
		materialData->cbufferDirty = true;
	}
}


void RenderThread::MaterialParameterData::SetValue(float value)
{
	if (auto materialData = m_materialData.lock())
	{
		assert(m_type == ShaderVariableType::Float);
		InternalSetValue(value);
		materialData->cbufferDirty = true;
	}
}


void RenderThread::MaterialParameterData::SetValue(XMFLOAT2 value)
{
	if (auto materialData = m_materialData.lock())
	{
		assert(m_type == ShaderVariableType::Float2);
		InternalSetValue(value);
		materialData->cbufferDirty = true;
	}
}


void RenderThread::MaterialParameterData::SetValue(XMFLOAT3 value)
{
	if (auto materialData = m_materialData.lock())
	{
		assert(m_type == ShaderVariableType::Float3);
		InternalSetValue(value);
		materialData->cbufferDirty = true;
	}
}


void RenderThread::MaterialParameterData::SetValue(XMFLOAT4 value)
{
	if (auto materialData = m_materialData.lock())
	{
		assert(m_type == ShaderVariableType::Float4);
		InternalSetValue(value);
		materialData->cbufferDirty = true;
	}
}


void RenderThread::MaterialParameterData::SetValue(XMFLOAT4X4 value)
{
	if (auto materialData = m_materialData.lock())
	{
		assert(m_type == ShaderVariableType::Float4x4);
		InternalSetValue(value);
		materialData->cbufferDirty = true;
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
		}
	}
}