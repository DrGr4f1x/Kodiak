// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "IndexBuffer.h"

#include "CommandList12.h"
#include "DeviceManager12.h"
#include "RenderEnums12.h"
#include "RenderUtils.h"

#include <locale>
#include <codecvt>

using namespace Kodiak;
using namespace std;


void IndexBuffer::Destroy()
{
	GpuResource::Destroy();
}


void IndexBuffer::Create(shared_ptr<BaseIndexBufferData> data, Usage usage, const string& debugName)
{
	loadTask = concurrency::create_task([this, data, usage, debugName]()
	{
		D3D12_RESOURCE_DESC resourceDesc = DescribeBuffer(data->GetNumElements(), data->GetElementSize());

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

		if (data->GetData())
		{
			CommandList::InitializeBuffer(*this, data->GetData(), data->GetDataSize());
		}

		// Set debug name
		wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
		wstring wide = converter.from_bytes(debugName);

		m_resource->SetName(wide.c_str());

		// Create the IBV
		m_ibv.BufferLocation = m_gpuVirtualAddress;
		m_ibv.Format = data->GetFormat();
		m_ibv.SizeInBytes = (UINT)data->GetDataSize();
	});
}


D3D12_RESOURCE_DESC IndexBuffer::DescribeBuffer(size_t numElements, size_t elementSize)
{
	m_bufferSize = numElements * elementSize;
	m_elementCount = numElements;
	m_elementSize = elementSize;

	D3D12_RESOURCE_DESC Desc = {};
	Desc.Alignment = 0;
	Desc.DepthOrArraySize = 1;
	Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	Desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	Desc.Format = DXGI_FORMAT_UNKNOWN;
	Desc.Height = 1;
	Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	Desc.MipLevels = 1;
	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;
	Desc.Width = (UINT64)(numElements * elementSize);

	return Desc;
}