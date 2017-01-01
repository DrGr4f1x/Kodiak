// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "VertexBuffer.h"

#include "CommandList12.h"
#include "DeviceManager12.h"
#include "RenderEnums12.h"
#include "RenderUtils.h"

#include <locale>
#include <codecvt>

using namespace Kodiak;
using namespace std;


namespace
{
map<size_t, shared_ptr<VertexBuffer>>	s_vertexBufferMap;
}


void VertexBuffer::Destroy()
{
	GpuResource::Destroy();
}


shared_ptr<VertexBuffer> VertexBuffer::Create(const BaseVertexBufferData& data, Usage usage)
{
	const auto hashCode = data.GetId();

	shared_ptr<VertexBuffer> vbuffer;

	{
		static mutex vertexBufferMutex;
		lock_guard<mutex> CS(vertexBufferMutex);

		auto iter = s_vertexBufferMap.find(hashCode);

		if (iter == s_vertexBufferMap.end())
		{
			vbuffer = make_shared<VertexBuffer>();
			s_vertexBufferMap[hashCode] = vbuffer;

			CreateInternal(vbuffer, data, usage);
		}
		else
		{
			vbuffer = iter->second;
		}
	}

	return vbuffer;
}


void VertexBuffer::CreateInternal(std::shared_ptr<VertexBuffer> vbuffer, const BaseVertexBufferData& data, Usage usage)
{
	D3D12_RESOURCE_DESC resourceDesc = vbuffer->DescribeBuffer(data.GetNumElements(), data.GetElementSize());

	vbuffer->m_usageState = D3D12_RESOURCE_STATE_COMMON;

	D3D12_HEAP_PROPERTIES heapProps{};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	ThrowIfFailed(g_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		vbuffer->m_usageState,
		nullptr,
		IID_PPV_ARGS(&vbuffer->m_resource)));

	vbuffer->m_gpuVirtualAddress = vbuffer->m_resource->GetGPUVirtualAddress();

	if (data.GetData())
	{
		CommandList::InitializeBuffer(*vbuffer, data.GetData(), data.GetDataSize());
	}

	// Set debug name
	const auto& debugName = data.GetDebugName();
	if (!debugName.empty())
	{
		wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
		wstring wide = converter.from_bytes(debugName);

		vbuffer->m_resource->SetName(wide.c_str());
	}

	// Create the VBV
	vbuffer->m_vbv.BufferLocation = vbuffer->m_gpuVirtualAddress;
	vbuffer->m_vbv.StrideInBytes = (UINT)data.GetStride();
	vbuffer->m_vbv.SizeInBytes = (UINT)data.GetDataSize();
}


D3D12_RESOURCE_DESC VertexBuffer::DescribeBuffer(size_t numElements, size_t elementSize)
{
	m_bufferSize = numElements * elementSize;
	m_elementCount = numElements;
	m_elementSize = elementSize;

	D3D12_RESOURCE_DESC desc{};
	desc.Alignment = 0;
	desc.DepthOrArraySize = 1;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.Height = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Width = (UINT64)(numElements * elementSize);

	return desc;
}