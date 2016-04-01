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
using namespace Math;
using namespace std;


MaterialParameter::MaterialParameter(const string& name)
	: m_name(name)
	, m_type(ShaderVariableType::Unsupported)
	, m_size(kInvalid)
	, m_renderThreadData()
	, m_bindings({nullptr, nullptr, nullptr, nullptr, nullptr})
{
	ZeroMemory(&m_data[0], 64);
}


void MaterialParameter::SetValue(bool value)
{
	memcpy(&m_data[0], &value, sizeof(bool));

	SubmitToRenderThread();
}


void MaterialParameter::SetValue(int32_t value)
{
	memcpy(&m_data[0], &value, sizeof(int32_t));

	SubmitToRenderThread();
}


void MaterialParameter::SetValue(XMINT2 value)
{
	memcpy(&m_data[0], &value, sizeof(XMINT2));

	SubmitToRenderThread();
}


void MaterialParameter::SetValue(XMINT3 value)
{
	memcpy(&m_data[0], &value, sizeof(XMINT3));

	SubmitToRenderThread();
}


void MaterialParameter::SetValue(XMINT4 value)
{
	memcpy(&m_data[0], &value, sizeof(XMINT4));

	SubmitToRenderThread();
}


void MaterialParameter::SetValue(uint32_t value)
{
	memcpy(&m_data[0], &value, sizeof(uint32_t));

	SubmitToRenderThread();
}


void MaterialParameter::SetValue(XMUINT2 value)
{
	memcpy(&m_data[0], &value, sizeof(XMUINT2));

	SubmitToRenderThread();
}


void MaterialParameter::SetValue(XMUINT3 value)
{
	memcpy(&m_data[0], &value, sizeof(XMUINT3));

	SubmitToRenderThread();
}


void MaterialParameter::SetValue(XMUINT4 value)
{
	memcpy(&m_data[0], &value, sizeof(XMUINT4));

	SubmitToRenderThread();
}


void MaterialParameter::SetValue(float value)
{
	memcpy(&m_data[0], &value, sizeof(float));

	SubmitToRenderThread();
}


void MaterialParameter::SetValue(XMFLOAT2 value)
{
	memcpy(&m_data[0], &value, sizeof(XMFLOAT2));

	SubmitToRenderThread();
}


void MaterialParameter::SetValue(Vector3 value)
{
	memcpy(&m_data[0], &value, sizeof(XMFLOAT3));

	SubmitToRenderThread();
}


void MaterialParameter::SetValue(Vector4 value)
{
	memcpy(&m_data[0], &value, sizeof(XMFLOAT4));

	SubmitToRenderThread();
}


void MaterialParameter::SetValue(const Matrix4& value)
{
	memcpy(&m_data[0], &value, sizeof(XMFLOAT4X4));

	SubmitToRenderThread();
}


void MaterialParameter::CreateRenderThreadData(std::shared_ptr<RenderThread::MaterialData> materialData, const ShaderReflection::Parameter<5>& parameter)
{
	m_renderThreadData = materialData;

	m_type = parameter.type;
	m_size = parameter.sizeInBytes;

	for (uint32_t i = 0; i < 5; ++i)
	{
		if (parameter.byteOffset[i] != kInvalid)
		{
			m_bindings[i] = materialData->cbufferData + parameter.byteOffset[i];
		}
	}

	UpdateParameterOnRenderThread(materialData.get(), m_data);
}


void MaterialParameter::UpdateParameterOnRenderThread(RenderThread::MaterialData* materialData, const array<byte, 64>& data)
{
	for (uint32_t i = 0; i < 5; ++i)
	{
		if (m_bindings[i])
		{
			memcpy(m_bindings[i], &data[0], m_size);
		}
	}
}


void MaterialParameter::SubmitToRenderThread()
{
	if (m_size == kInvalid || m_type == ShaderVariableType::Unsupported)
	{
		return;
	}

	if (auto materialData = m_renderThreadData.lock())
	{
		auto thisParameter = shared_from_this();
		auto thisData = m_data;
		Renderer::GetInstance().EnqueueTask([materialData, thisParameter, thisData](RenderTaskEnvironment& rte)
		{
			thisParameter->UpdateParameterOnRenderThread(materialData.get(), thisData);
		});
	}
}