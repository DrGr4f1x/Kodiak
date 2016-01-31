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
#include <map>
#include <codecvt>

using namespace Kodiak;
using namespace std;


namespace Kodiak
{
atomic_size_t IndexBufferData16::s_baseId = 1;
atomic_size_t IndexBufferData32::s_baseId = 1;
} // namespace Kodiak


namespace
{
map<size_t, shared_ptr<IndexBuffer>>	s_indexBufferMap;
} // anonymous namespace


void IndexBuffer::Destroy()
{
	GpuResource::Destroy();
}


shared_ptr<IndexBuffer> IndexBuffer::Create(shared_ptr<BaseIndexBufferData> data, Usage usage, bool async)
{
	const auto hashCode = data->GetId();

	shared_ptr<IndexBuffer> ibuffer;

	{
		static mutex indexBufferMutex;
		lock_guard<mutex> CS(indexBufferMutex);

		auto iter = s_indexBufferMap.find(hashCode);

		if (iter == s_indexBufferMap.end())
		{
			ibuffer = make_shared<IndexBuffer>();
			s_indexBufferMap[hashCode] = ibuffer;

			if (async)
			{
				// Non-blocking asynchronous create
				ibuffer->loadTask = concurrency::create_task([ibuffer, data, usage]()
				{
					IndexBuffer::CreateInternal(ibuffer, data, usage);
				});
			}
			else
			{
				// Blocking synchronous create
				CreateInternal(ibuffer, data, usage);
			}
		}
		else
		{
			ibuffer = iter->second;
		}
	}

	return ibuffer;
}


void IndexBuffer::CreateInternal(shared_ptr<IndexBuffer> ibuffer, shared_ptr<BaseIndexBufferData> data, Usage usage)
{
	D3D12_RESOURCE_DESC resourceDesc = ibuffer->DescribeBuffer(data->GetNumElements(), data->GetElementSize());

	ibuffer->m_usageState = D3D12_RESOURCE_STATE_COMMON;

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
		ibuffer->m_usageState,
		nullptr,
		IID_PPV_ARGS(&ibuffer->m_resource)));

	ibuffer->m_gpuVirtualAddress = ibuffer->m_resource->GetGPUVirtualAddress();

	if (data->GetData())
	{
		CommandList::InitializeBuffer(*ibuffer, data->GetData(), data->GetDataSize());
	}

	// Set debug name
	const auto& debugName = data->GetDebugName();
	if (!debugName.empty())
	{
		wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
		wstring wide = converter.from_bytes(debugName);

		ibuffer->m_resource->SetName(wide.c_str());
	}

	// Create the IBV
	ibuffer->m_ibv.BufferLocation = ibuffer->m_gpuVirtualAddress;
	ibuffer->m_ibv.Format = data->GetFormat();
	ibuffer->m_ibv.SizeInBytes = (UINT)data->GetDataSize();
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