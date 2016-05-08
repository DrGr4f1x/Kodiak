// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "ComputeParameter.h"

#include "ComputeKernel.h"
#include "RenderEnums.h"
#include "Renderer.h"

using namespace Kodiak;
using namespace Math;
using namespace DirectX;
using namespace std;


ComputeParameter::ComputeParameter(const string& name)
	: m_name(name)
	, m_type(ShaderVariableType::Unsupported)
	, m_renderThreadData()
{
	ZeroMemory(&m_data[0], 64);
}


void ComputeParameter::SetValue(bool value)
{
	memcpy(&m_data[0], &value, sizeof(bool));

	SubmitToRenderThread();
}


void ComputeParameter::SetValue(int32_t value)
{
	memcpy(&m_data[0], &value, sizeof(int32_t));

	SubmitToRenderThread();
}


void ComputeParameter::SetValue(XMINT2 value)
{
	memcpy(&m_data[0], &value, sizeof(XMINT2));

	SubmitToRenderThread();
}


void ComputeParameter::SetValue(XMINT3 value)
{
	memcpy(&m_data[0], &value, sizeof(XMINT3));

	SubmitToRenderThread();
}


void ComputeParameter::SetValue(XMINT4 value)
{
	memcpy(&m_data[0], &value, sizeof(XMINT4));

	SubmitToRenderThread();
}


void ComputeParameter::SetValue(uint32_t value)
{
	memcpy(&m_data[0], &value, sizeof(uint32_t));

	SubmitToRenderThread();
}


void ComputeParameter::SetValue(XMUINT2 value)
{
	memcpy(&m_data[0], &value, sizeof(XMUINT2));

	SubmitToRenderThread();
}


void ComputeParameter::SetValue(XMUINT3 value)
{
	memcpy(&m_data[0], &value, sizeof(XMUINT3));

	SubmitToRenderThread();
}


void ComputeParameter::SetValue(XMUINT4 value)
{
	memcpy(&m_data[0], &value, sizeof(XMUINT4));

	SubmitToRenderThread();
}


void ComputeParameter::SetValue(float value)
{
	memcpy(&m_data[0], &value, sizeof(float));

	SubmitToRenderThread();
}


void ComputeParameter::SetValue(XMFLOAT2 value)
{
	memcpy(&m_data[0], &value, sizeof(XMFLOAT2));

	SubmitToRenderThread();
}


void ComputeParameter::SetValue(Vector3 value)
{
	memcpy(&m_data[0], &value, sizeof(XMFLOAT3));

	SubmitToRenderThread();
}


void ComputeParameter::SetValue(Vector4 value)
{
	memcpy(&m_data[0], &value, sizeof(XMFLOAT4));

	SubmitToRenderThread();
}


void ComputeParameter::SetValue(const Matrix4& value)
{
	memcpy(&m_data[0], &value, sizeof(XMFLOAT4X4));

	SubmitToRenderThread();
}


void ComputeParameter::SetValueImmediate(bool value)
{
	memcpy(&m_data[0], &value, sizeof(bool));

	UpdateParameterOnRenderThread(m_data);
}


void ComputeParameter::SetValueImmediate(int32_t value)
{
	memcpy(&m_data[0], &value, sizeof(int32_t));

	UpdateParameterOnRenderThread(m_data);
}


void ComputeParameter::SetValueImmediate(XMINT2 value)
{
	memcpy(&m_data[0], &value, sizeof(XMINT2));

	UpdateParameterOnRenderThread(m_data);
}


void ComputeParameter::SetValueImmediate(XMINT3 value)
{
	memcpy(&m_data[0], &value, sizeof(XMINT3));

	UpdateParameterOnRenderThread(m_data);
}


void ComputeParameter::SetValueImmediate(XMINT4 value)
{
	memcpy(&m_data[0], &value, sizeof(XMINT4));

	UpdateParameterOnRenderThread(m_data);
}


void ComputeParameter::SetValueImmediate(uint32_t value)
{
	memcpy(&m_data[0], &value, sizeof(uint32_t));

	UpdateParameterOnRenderThread(m_data);
}


void ComputeParameter::SetValueImmediate(XMUINT2 value)
{
	memcpy(&m_data[0], &value, sizeof(XMUINT2));

	UpdateParameterOnRenderThread(m_data);
}


void ComputeParameter::SetValueImmediate(XMUINT3 value)
{
	memcpy(&m_data[0], &value, sizeof(XMUINT3));

	UpdateParameterOnRenderThread(m_data);
}


void ComputeParameter::SetValueImmediate(XMUINT4 value)
{
	memcpy(&m_data[0], &value, sizeof(XMUINT4));

	UpdateParameterOnRenderThread(m_data);
}


void ComputeParameter::SetValueImmediate(float value)
{
	memcpy(&m_data[0], &value, sizeof(float));

	UpdateParameterOnRenderThread(m_data);
}


void ComputeParameter::SetValueImmediate(XMFLOAT2 value)
{
	memcpy(&m_data[0], &value, sizeof(XMFLOAT2));

	UpdateParameterOnRenderThread(m_data);
}


void ComputeParameter::SetValueImmediate(Vector3 value)
{
	memcpy(&m_data[0], &value, sizeof(XMFLOAT3));

	UpdateParameterOnRenderThread(m_data);
}


void ComputeParameter::SetValueImmediate(Vector4 value)
{
	memcpy(&m_data[0], &value, sizeof(XMFLOAT4));

	UpdateParameterOnRenderThread(m_data);
}


void ComputeParameter::SetValueImmediate(const Matrix4& value)
{
	memcpy(&m_data[0], &value, sizeof(XMFLOAT4X4));

	UpdateParameterOnRenderThread(m_data);
}


void ComputeParameter::CreateRenderThreadData(shared_ptr<RenderThread::ComputeData> computeData, const ShaderReflection::Parameter<1>& parameter)
{
	m_renderThreadData = computeData;

	m_type = parameter.type;
	m_size = parameter.sizeInBytes;

	if (parameter.byteOffset[0] != kInvalid)
	{
		m_binding = computeData->cbufferData + parameter.byteOffset[0];
	}
	
	UpdateParameterOnRenderThread(m_data);
}


void ComputeParameter::UpdateParameterOnRenderThread(const array<byte, 64>& data)
{
	if (m_binding)
	{
		memcpy(m_binding, &data[0], m_size);
	}
}


void ComputeParameter::SubmitToRenderThread()
{
	if (m_size == kInvalid || m_type == ShaderVariableType::Unsupported)
	{
		return;
	}

	if (auto computeData = m_renderThreadData.lock())
	{
		auto thisParameter = shared_from_this();
		auto thisData = m_data;
		Renderer::GetInstance().EnqueueTask([computeData, thisParameter, thisData](RenderTaskEnvironment& rte)
		{
			thisParameter->UpdateParameterOnRenderThread(thisData);
			computeData->cbufferDirty = true;
		});
	}
}