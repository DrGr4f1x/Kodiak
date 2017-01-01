// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "MaterialConstantBuffer.h"


using namespace Kodiak;
using namespace std;


MaterialConstantBuffer::MaterialConstantBuffer(const string& name)
	: m_name(name)
{
	for (uint32_t i = 0; i < 5; ++i)
	{
		m_bindings[i] = nullptr;
	}
}


void MaterialConstantBuffer::SetDataImmediate(size_t sizeInBytes, const byte* data)
{
	assert(m_size >= sizeInBytes);

	for (uint32_t i = 0; i < 5; ++i)
	{
		if (m_bindings[i])
		{
			memcpy(m_bindings[i], data, sizeInBytes);
		}
	}
}


void MaterialConstantBuffer::CreateRenderThreadData(uint32_t index, size_t sizeInBytes, byte* destination)
{
	assert(m_size == kInvalid || m_size == sizeInBytes);
	m_size = sizeInBytes;

	assert(m_bindings[index] == nullptr);
	m_bindings[index] = destination;
}