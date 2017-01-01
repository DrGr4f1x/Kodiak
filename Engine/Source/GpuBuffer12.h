// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from GpuBuffer.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#pragma once

#include "GpuResource.h"

namespace Kodiak
{

class CommandList;
class EsramAllocator;

class GpuBuffer : public GpuResource
{
public:
	virtual ~GpuBuffer() { Destroy(); }

	virtual void Destroy();

	// Create a buffer.  If initial data is provided, it will be copied into the buffer using the default command context.
	void Create(const std::string& name, uint32_t numElements, uint32_t elementSize,
		const void* initialData = nullptr);

	// Create a buffer in an upload heap.
	void CreateUpload(const std::string& name, uint32_t size);

	// Create a buffer in ESRAM.  On Windows, ESRAM is not used.
	void Create(const std::string& name, uint32_t NumElements, uint32_t ElementSize,
		EsramAllocator& Allocator, const void* initialData = nullptr);

	// Sub-Allocate a buffer out of a pre-allocated heap.  If initial data is provided, it will be copied into the buffer using the default command context.
	void CreatePlaced(const std::string& name, ID3D12Heap* pBackingHeap, uint32_t HeapOffset, uint32_t NumElements, uint32_t ElementSize,
		const void* initialData = nullptr);

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetUAV() const { return m_uav; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV() const { return m_srv; }

	D3D12_GPU_VIRTUAL_ADDRESS RootConstantBufferView() const { return m_gpuVirtualAddress; }

	D3D12_CPU_DESCRIPTOR_HANDLE CreateConstantBufferView(uint32_t offset, uint32_t size) const;

protected:
	GpuBuffer(void) : m_bufferSize(0), m_elementCount(0), m_elementSize(0)
	{
		m_resourceFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		m_uav.ptr = ~0ull;
		m_srv.ptr = ~0ull;
		m_uploadHeap = false;
	}

	D3D12_RESOURCE_DESC DescribeBuffer();
	virtual void CreateDerivedViews() = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE m_uav;
	D3D12_CPU_DESCRIPTOR_HANDLE m_srv;

	size_t m_bufferSize;
	uint32_t m_elementCount;
	uint32_t m_elementSize;
	D3D12_RESOURCE_FLAGS m_resourceFlags;
	bool m_uploadHeap;
};


class ByteAddressBuffer : public GpuBuffer
{
public:
	void CreateDerivedViews() override;
};


class IndirectArgsBuffer : public ByteAddressBuffer
{
public:
	IndirectArgsBuffer() {}
};


class StructuredBuffer : public GpuBuffer
{
public:
	void Destroy() override
	{
		m_counterBuffer->Destroy();
		GpuBuffer::Destroy();
	}

	void CreateDerivedViews() override;

	std::shared_ptr<ByteAddressBuffer> GetCounterBuffer() { return m_counterBuffer; }

	std::shared_ptr<ByteAddressBuffer> GetCounterSRV(CommandList& commandList);
	std::shared_ptr<ByteAddressBuffer> GetCounterUAV(CommandList& commandList);

private:
	std::shared_ptr<ByteAddressBuffer> m_counterBuffer;
};


class TypedBuffer : public GpuBuffer
{
public:
	// TODO: change format parameter to ColorFormat, instead of DXGI_FORMAT
	TypedBuffer(DXGI_FORMAT format) : m_dataFormat(format) {}
	void CreateDerivedViews() override;

protected:
	DXGI_FORMAT m_dataFormat;
};


class MappedConstantBuffer : public GpuBuffer
{
public:
	MappedConstantBuffer() {}
	void CreateDerivedViews() override {}

	byte* Map();
};

} // namespace Kodiak