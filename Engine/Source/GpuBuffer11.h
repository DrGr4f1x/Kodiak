// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "GpuResource11.h"

namespace Kodiak
{

// Forward declarations
class CommandList;


class GpuBuffer : public GpuResource
{
public:
	virtual ~GpuBuffer() { Destroy(); }

	virtual void Destroy();

	// Create a buffer.  If initial data is provided, it will be copied into the buffer using the default command context.
	void Create(const std::string& name, uint32_t numElements, uint32_t elementSize,
		const void* initialData = nullptr);

	ID3D11ShaderResourceView* GetSRV() { return m_srv.Get(); }
	ID3D11UnorderedAccessView* GetUAV() { return m_uav.Get(); }
	ID3D11Buffer* GetBuffer() { return m_buffer.Get(); }

	uint32_t GetCounterInitialValue() const { return m_counterInitialValue; }
	void SetCounterInitialValue(uint32_t value) { m_counterInitialValue = value; }

protected:
	GpuBuffer() 
	{
		m_bindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	}

	D3D11_BUFFER_DESC DescribeBuffer();
	virtual void CreateDerivedViews() = 0;
	virtual void SetBuffer(ID3D11Buffer* buffer) {}

protected:
	size_t		m_bufferSize{ 0 };
	uint32_t	m_elementCount{ 0 };
	uint32_t	m_elementSize{ 0 };
	uint32_t	m_bindFlags{ 0 };
	uint32_t	m_miscFlags{ 0 };
	uint32_t	m_counterInitialValue{ 0 };

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_srv;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>	m_uav;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				m_buffer;
};


class ByteAddressBuffer : public GpuBuffer
{
public:
	ByteAddressBuffer() { m_miscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS; }
	void CreateDerivedViews() override;
};


class IndirectArgsBuffer : public ByteAddressBuffer
{
public:
	IndirectArgsBuffer() 
	{ 
		m_miscFlags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
	}
};


class StructuredBuffer : public GpuBuffer
{
public:
	StructuredBuffer()
	{
		m_miscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	}

	void Destroy() override
	{
		GpuBuffer::Destroy();
	}

	void CreateDerivedViews() override;
};


class TypedBuffer : public GpuBuffer
{
public:
	TypedBuffer(DXGI_FORMAT format) : m_dataFormat(format) {}
	void CreateDerivedViews() override;

protected:
	DXGI_FORMAT m_dataFormat;
};

} // namespace Kodiak