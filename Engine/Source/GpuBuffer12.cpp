// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from GpuBuffer.cpp in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#include "Stdafx.h"

#include "GpuBuffer12.h"

#include "CommandList12.h"
#include "DeviceManager12.h"
#include "Renderer.h"
#include "RenderUtils.h"

#include <locale>
#include <codecvt>

using namespace Kodiak;
using namespace std;


void GpuBuffer::Destroy()
{
	GpuResource::Destroy();
}


void GpuBuffer::Create(const std::string& name, uint32_t numElements, uint32_t elementSize, const void* initialData)
{
	m_elementCount = numElements;
	m_elementSize = elementSize;
	m_bufferSize = numElements * elementSize;

	D3D12_RESOURCE_DESC resourceDesc = DescribeBuffer();

	m_usageState = D3D12_RESOURCE_STATE_COMMON;

	D3D12_HEAP_PROPERTIES heapProps;
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	ThrowIfFailed(g_device->CreateCommittedResource(
		&heapProps, 
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc, 
		m_usageState, 
		nullptr, 
		IID_PPV_ARGS(&m_resource)));

	m_gpuVirtualAddress = m_resource->GetGPUVirtualAddress();

	if (initialData)
	{
		CommandList::InitializeBuffer(*this, initialData, m_bufferSize);
	}

	// Set debug name
	if (!name.empty())
	{
		wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
		wstring wide = converter.from_bytes(name);

		m_resource->SetName(wide.c_str());
	}

	CreateDerivedViews();
}


void GpuBuffer::CreateUpload(const std::string& name, uint32_t size)
{
	m_uploadHeap = true;

	m_bufferSize = size;

	m_resourceFlags = D3D12_RESOURCE_FLAG_NONE;
	D3D12_RESOURCE_DESC resourceDesc = DescribeBuffer();

	m_usageState = D3D12_RESOURCE_STATE_GENERIC_READ;

	D3D12_HEAP_PROPERTIES heapProps;
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	ThrowIfFailed(g_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		m_usageState,
		nullptr,
		IID_PPV_ARGS(&m_resource)));

	m_gpuVirtualAddress = m_resource->GetGPUVirtualAddress();

	// Set debug name
	if (!name.empty())
	{
		wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
		wstring wide = converter.from_bytes(name);

		m_resource->SetName(wide.c_str());
	}

	CreateDerivedViews();
}


void GpuBuffer::Create(const std::string& name, uint32_t NumElements, uint32_t ElementSize,
	EsramAllocator&, const void* initialData)
{
	Create(name, NumElements, ElementSize, initialData);
}


// Sub-Allocate a buffer out of a pre-allocated heap.  If initial data is provided, it will be copied into the buffer using the default command context.
void GpuBuffer::CreatePlaced(const std::string& name, ID3D12Heap* backingHeap, uint32_t heapOffset, uint32_t numElements, uint32_t elementSize,
	const void* initialData)
{
	m_elementCount = numElements;
	m_elementSize = elementSize;
	m_bufferSize = numElements * elementSize;

	D3D12_RESOURCE_DESC resourceDesc = DescribeBuffer();

	m_usageState = D3D12_RESOURCE_STATE_COMMON;

	ThrowIfFailed(g_device->CreatePlacedResource(
		backingHeap, 
		heapOffset, 
		&resourceDesc, 
		m_usageState, 
		nullptr, 
		IID_PPV_ARGS(&m_resource)));

	m_gpuVirtualAddress = m_resource->GetGPUVirtualAddress();

	if (initialData)
	{
		CommandList::InitializeBuffer(*this, initialData, m_bufferSize);
	}

	// Set debug name
	if (!name.empty())
	{
		wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
		wstring wide = converter.from_bytes(name);

		m_resource->SetName(wide.c_str());
	}

	CreateDerivedViews();
}


D3D12_CPU_DESCRIPTOR_HANDLE GpuBuffer::CreateConstantBufferView(uint32_t offset, uint32_t size) const
{
	assert(offset + size <= m_bufferSize);

	size = Math::AlignUp(size, 256);

	D3D12_CONSTANT_BUFFER_VIEW_DESC CBVDesc;
	CBVDesc.BufferLocation = m_gpuVirtualAddress + (size_t)offset;
	CBVDesc.SizeInBytes = size;

	auto deviceManager = Renderer::GetInstance().GetDeviceManager();
	D3D12_CPU_DESCRIPTOR_HANDLE hCBV = deviceManager->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	g_device->CreateConstantBufferView(&CBVDesc, hCBV);
	return hCBV;
}


D3D12_RESOURCE_DESC GpuBuffer::DescribeBuffer()
{
	assert(m_bufferSize != 0);

	D3D12_RESOURCE_DESC Desc = {};
	Desc.Alignment = 0;
	Desc.DepthOrArraySize = 1;
	Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	Desc.Flags = m_resourceFlags;
	Desc.Format = DXGI_FORMAT_UNKNOWN;
	Desc.Height = 1;
	Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	Desc.MipLevels = 1;
	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;
	Desc.Width = (UINT64)m_bufferSize;
	return Desc;
}


void ByteAddressBuffer::CreateDerivedViews()
{
	auto deviceManager = Renderer::GetInstance().GetDeviceManager();

	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	SRVDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SRVDesc.Buffer.NumElements = (UINT)m_bufferSize / 4;
	SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

	if (m_srv.ptr == ~0ull)
	{
		m_srv = deviceManager->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	g_device->CreateShaderResourceView(m_resource.Get(), &SRVDesc, m_srv);

	D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	UAVDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	UAVDesc.Buffer.NumElements = (UINT)m_bufferSize / 4;
	UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

	if (m_uav.ptr == ~0ull)
	{
		m_uav = deviceManager->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	g_device->CreateUnorderedAccessView(m_resource.Get(), nullptr, &UAVDesc, m_uav);
}


const D3D12_CPU_DESCRIPTOR_HANDLE& StructuredBuffer::GetCounterSRV(CommandList& commandList)
{
	commandList.TransitionResource(m_counterBuffer, ResourceState::GenericRead);
	return m_counterBuffer.GetSRV();
}


const D3D12_CPU_DESCRIPTOR_HANDLE& StructuredBuffer::GetCounterUAV(CommandList& commandList)
{
	commandList.TransitionResource(m_counterBuffer, ResourceState::UnorderedAccess);
	return m_counterBuffer.GetUAV();
}


void StructuredBuffer::CreateDerivedViews()
{
	auto deviceManager = Renderer::GetInstance().GetDeviceManager();

	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SRVDesc.Buffer.NumElements = m_elementCount;
	SRVDesc.Buffer.StructureByteStride = m_elementSize;
	SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	if (m_srv.ptr == ~0ull)
	{
		m_srv = deviceManager->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	g_device->CreateShaderResourceView(m_resource.Get(), &SRVDesc, m_srv);

	D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
	UAVDesc.Buffer.CounterOffsetInBytes = 0;
	UAVDesc.Buffer.NumElements = m_elementCount;
	UAVDesc.Buffer.StructureByteStride = m_elementSize;
	UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	m_counterBuffer.Create("StructuredBuffer::Counter", 1, 4);

	if (m_uav.ptr == ~0ull)
	{
		m_uav = deviceManager->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	g_device->CreateUnorderedAccessView(m_resource.Get(), m_counterBuffer.GetResource(), &UAVDesc, m_uav);
}


void TypedBuffer::CreateDerivedViews()
{
	auto deviceManager = Renderer::GetInstance().GetDeviceManager();

	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	SRVDesc.Format = m_dataFormat;
	SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SRVDesc.Buffer.NumElements = m_elementCount;
	SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	if (m_srv.ptr == ~0ull)
	{
		m_srv = deviceManager->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	g_device->CreateShaderResourceView(m_resource.Get(), &SRVDesc, m_srv);

	D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	UAVDesc.Format = m_dataFormat;
	UAVDesc.Buffer.NumElements = m_elementCount;
	UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	if (m_uav.ptr == ~0ull)
	{
		m_uav = deviceManager->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	g_device->CreateUnorderedAccessView(m_resource.Get(), nullptr, &UAVDesc, m_uav);
}


byte* MappedConstantBuffer::Map()
{
	byte* mappedData = nullptr;
	CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
	ThrowIfFailed(m_resource->Map(0, &readRange, reinterpret_cast<void**>(&mappedData)));

	return mappedData;
}