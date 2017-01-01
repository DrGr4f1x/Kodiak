// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "ComputeConstantBuffer.h"

#include "ComputeKernel.h"

using namespace Kodiak;
using namespace std;


ComputeConstantBuffer::ComputeConstantBuffer(const string& name)
	: m_name(name)
{}


void ComputeConstantBuffer::SetDataImmediate(size_t sizeInBytes, const byte* data)
{
	assert(m_size >= sizeInBytes);

	memcpy(m_binding, data, sizeInBytes);
}


void ComputeConstantBuffer::CreateRenderThreadData(shared_ptr<RenderThread::ComputeData> computeData, const ShaderReflection::CBVLayout& cbv)
{
	m_renderThreadData = computeData;

	m_size = cbv.sizeInBytes;

	if (cbv.byteOffset != kInvalid)
	{
		m_binding = computeData->cbufferData + cbv.byteOffset;
	}
}