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
{}


void ComputeParameter::SetValue(bool value, int32_t index)
{
	array<byte, 64> data;
	memcpy(&data[0], &value, sizeof(bool));

	SubmitToRenderThread(move(data), index);
}


void ComputeParameter::SetValue(int32_t value, int32_t index)
{
	array<byte, 64> data;
	memcpy(&data[0], &value, sizeof(int32_t));

	SubmitToRenderThread(move(data), index);
}


void ComputeParameter::SetValue(XMINT2 value, int32_t index)
{
	array<byte, 64> data;
	memcpy(&data[0], &value, sizeof(XMINT2));

	SubmitToRenderThread(move(data), index);
}


void ComputeParameter::SetValue(XMINT3 value, int32_t index)
{
	array<byte, 64> data;
	memcpy(&data[0], &value, sizeof(XMINT3));

	SubmitToRenderThread(move(data), index);
}


void ComputeParameter::SetValue(XMINT4 value, int32_t index)
{
	array<byte, 64> data;
	memcpy(&data[0], &value, sizeof(XMINT4));

	SubmitToRenderThread(move(data), index);
}


void ComputeParameter::SetValue(uint32_t value, int32_t index)
{
	array<byte, 64> data;
	memcpy(&data[0], &value, sizeof(uint32_t));

	SubmitToRenderThread(move(data), index);
}


void ComputeParameter::SetValue(XMUINT2 value, int32_t index)
{
	array<byte, 64> data;
	memcpy(&data[0], &value, sizeof(XMUINT2));

	SubmitToRenderThread(move(data), index);
}


void ComputeParameter::SetValue(XMUINT3 value, int32_t index)
{
	array<byte, 64> data;
	memcpy(&data[0], &value, sizeof(XMUINT3));

	SubmitToRenderThread(move(data), index);
}


void ComputeParameter::SetValue(XMUINT4 value, int32_t index)
{
	array<byte, 64> data;
	memcpy(&data[0], &value, sizeof(XMUINT4));

	SubmitToRenderThread(move(data), index);
}


void ComputeParameter::SetValue(float value, int32_t index)
{
	array<byte, 64> data;
	memcpy(&data[0], &value, sizeof(float));

	SubmitToRenderThread(move(data), index);
}


void ComputeParameter::SetValue(XMFLOAT2 value, int32_t index)
{
	array<byte, 64> data;
	memcpy(&data[0], &value, sizeof(XMFLOAT2));

	SubmitToRenderThread(move(data), index);
}


void ComputeParameter::SetValue(Vector3 value, int32_t index)
{
	array<byte, 64> data;
	memcpy(&data[0], &value, sizeof(Vector3));

	SubmitToRenderThread(move(data), index);
}


void ComputeParameter::SetValue(Vector4 value, int32_t index)
{
	array<byte, 64> data;
	memcpy(&data[0], &value, sizeof(Vector4));

	SubmitToRenderThread(move(data), index);
}


void ComputeParameter::SetValue(const Matrix4& value, int32_t index)
{
	array<byte, 64> data;
	memcpy(&data[0], &value, sizeof(Matrix4));

	SubmitToRenderThread(move(data), index);
}


void ComputeParameter::SetValueImmediate(bool value, int32_t index)
{
	array<byte, 64> data;
	memcpy(&data[0], &value, sizeof(bool));

	if (auto computeData = m_renderThreadData.lock())
	{
		UpdateParameterOnRenderThread(data, index);
		computeData->cbufferDirty = true;
	}
	else
	{
		m_pendingUpdates.push_back(make_pair(data, index));
	}
}


void ComputeParameter::SetValueImmediate(int32_t value, int32_t index)
{
	array<byte, 64> data;
	memcpy(&data[0], &value, sizeof(int32_t));

	if (auto computeData = m_renderThreadData.lock())
	{
		UpdateParameterOnRenderThread(data, index);
		computeData->cbufferDirty = true;
	}
	else
	{
		m_pendingUpdates.push_back(make_pair(data, index));
	}
}


void ComputeParameter::SetValueImmediate(XMINT2 value, int32_t index)
{
	array<byte, 64> data;
	memcpy(&data[0], &value, sizeof(XMINT2));

	if (auto computeData = m_renderThreadData.lock())
	{
		UpdateParameterOnRenderThread(data, index);
		computeData->cbufferDirty = true;
	}
	else
	{
		m_pendingUpdates.push_back(make_pair(data, index));
	}
}


void ComputeParameter::SetValueImmediate(XMINT3 value, int32_t index)
{
	array<byte, 64> data;
	memcpy(&data[0], &value, sizeof(XMINT3));

	if (auto computeData = m_renderThreadData.lock())
	{
		UpdateParameterOnRenderThread(data, index);
		computeData->cbufferDirty = true;
	}
	else
	{
		m_pendingUpdates.push_back(make_pair(data, index));
	}
}


void ComputeParameter::SetValueImmediate(XMINT4 value, int32_t index)
{
	array<byte, 64> data;
	memcpy(&data[0], &value, sizeof(XMINT4));

	if (auto computeData = m_renderThreadData.lock())
	{
		UpdateParameterOnRenderThread(data, index);
		computeData->cbufferDirty = true;
	}
	else
	{
		m_pendingUpdates.push_back(make_pair(data, index));
	}
}


void ComputeParameter::SetValueImmediate(uint32_t value, int32_t index)
{
	array<byte, 64> data;
	memcpy(&data[0], &value, sizeof(uint32_t));

	if (auto computeData = m_renderThreadData.lock())
	{
		UpdateParameterOnRenderThread(data, index);
		computeData->cbufferDirty = true;
	}
	else
	{
		m_pendingUpdates.push_back(make_pair(data, index));
	}
}


void ComputeParameter::SetValueImmediate(XMUINT2 value, int32_t index)
{
	array<byte, 64> data;
	memcpy(&data[0], &value, sizeof(XMUINT2));

	if (auto computeData = m_renderThreadData.lock())
	{
		UpdateParameterOnRenderThread(data, index);
		computeData->cbufferDirty = true;
	}
	else
	{
		m_pendingUpdates.push_back(make_pair(data, index));
	}
}


void ComputeParameter::SetValueImmediate(XMUINT3 value, int32_t index)
{
	array<byte, 64> data;
	memcpy(&data[0], &value, sizeof(XMUINT3));

	if (auto computeData = m_renderThreadData.lock())
	{
		UpdateParameterOnRenderThread(data, index);
		computeData->cbufferDirty = true;
	}
	else
	{
		m_pendingUpdates.push_back(make_pair(data, index));
	}
}


void ComputeParameter::SetValueImmediate(XMUINT4 value, int32_t index)
{
	array<byte, 64> data;
	memcpy(&data[0], &value, sizeof(XMUINT4));

	if (auto computeData = m_renderThreadData.lock())
	{
		UpdateParameterOnRenderThread(data, index);
		computeData->cbufferDirty = true;
	}
	else
	{
		m_pendingUpdates.push_back(make_pair(data, index));
	}
}


void ComputeParameter::SetValueImmediate(float value, int32_t index)
{
	array<byte, 64> data;
	memcpy(&data[0], &value, sizeof(float));

	if (auto computeData = m_renderThreadData.lock())
	{
		UpdateParameterOnRenderThread(data, index);
		computeData->cbufferDirty = true;
	}
	else
	{
		m_pendingUpdates.push_back(make_pair(data, index));
	}
}


void ComputeParameter::SetValueImmediate(XMFLOAT2 value, int32_t index)
{
	array<byte, 64> data;
	memcpy(&data[0], &value, sizeof(XMFLOAT2));

	if (auto computeData = m_renderThreadData.lock())
	{
		UpdateParameterOnRenderThread(data, index);
		computeData->cbufferDirty = true;
	}
	else
	{
		m_pendingUpdates.push_back(make_pair(data, index));
	}
}


void ComputeParameter::SetValueImmediate(Vector3 value, int32_t index)
{
	array<byte, 64> data;
	memcpy(&data[0], &value, sizeof(Vector3));

	if (auto computeData = m_renderThreadData.lock())
	{
		UpdateParameterOnRenderThread(data, index);
		computeData->cbufferDirty = true;
	}
	else
	{
		m_pendingUpdates.push_back(make_pair(data, index));
	}
}


void ComputeParameter::SetValueImmediate(Vector4 value, int32_t index)
{
	array<byte, 64> data;
	memcpy(&data[0], &value, sizeof(Vector4));

	if (auto computeData = m_renderThreadData.lock())
	{
		UpdateParameterOnRenderThread(data, index);
		computeData->cbufferDirty = true;
	}
	else
	{
		m_pendingUpdates.push_back(make_pair(data, index));
	}
}


void ComputeParameter::SetValueImmediate(const Matrix4& value, int32_t index)
{
	array<byte, 64> data;
	memcpy(&data[0], &value, sizeof(Matrix4));

	if (auto computeData = m_renderThreadData.lock())
	{
		UpdateParameterOnRenderThread(data, index);
		computeData->cbufferDirty = true;
	}
	else
	{
		m_pendingUpdates.push_back(make_pair(data, index));
	}
}


void ComputeParameter::CreateRenderThreadData(shared_ptr<RenderThread::ComputeData> computeData, const ShaderReflection::Parameter<1>& parameter)
{
	m_renderThreadData = computeData;

	m_type = parameter.type;
	m_size = parameter.sizeInBytes;
	m_numElements = parameter.numElements;

	if (m_numElements > 0)
	{
		m_size /= m_numElements;
	}

	if (parameter.byteOffset[0] != kInvalid)
	{
		m_binding = computeData->cbufferData + parameter.byteOffset[0];
	}
	
	FlushPendingUpdates();
}


void ComputeParameter::UpdateParameterOnRenderThread(const array<byte, 64>& data, int32_t index)
{
	assert((index == 0 && m_numElements == 0) || (index < static_cast<int32_t>(m_numElements)));
	if (m_binding)
	{
		memcpy(m_binding + index * m_size, &data[0], m_size);
	}
}


void ComputeParameter::SubmitToRenderThread(array<byte, 64> data, int32_t index)
{
	if (m_size == kInvalid || m_type == ShaderVariableType::Unsupported)
	{
		return;
	}

	if (auto computeData = m_renderThreadData.lock())
	{
		auto thisParameter = shared_from_this();
		EnqueueRenderCommand([computeData, thisParameter, data, index]()
		{
			thisParameter->UpdateParameterOnRenderThread(data, index);
			computeData->cbufferDirty = true;
		});
	}
}


void ComputeParameter::FlushPendingUpdates()
{
	for (const auto& updatePair : m_pendingUpdates)
	{
		UpdateParameterOnRenderThread(updatePair.first, updatePair.second);
	}
}