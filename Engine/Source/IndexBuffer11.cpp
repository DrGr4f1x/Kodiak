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

#include "DeviceManager11.h"
#include "RenderUtils.h"

using namespace Kodiak;
using namespace std;
using namespace Microsoft::WRL;


namespace Kodiak
{
atomic_size_t IndexBufferData16::s_baseId = 1;
atomic_size_t IndexBufferData32::s_baseId = 1;
} // namespace Kodiak


namespace
{
map<size_t, shared_ptr<IndexBuffer>>	s_indexBufferMap;
} // anonymous namespace


shared_ptr<IndexBuffer> IndexBuffer::Create(const BaseIndexBufferData& data, Usage usage)
{
	const auto hashCode = data.GetId();

	shared_ptr<IndexBuffer> ibuffer;

	{
		static mutex indexBufferMutex;
		lock_guard<mutex> CS(indexBufferMutex);

		auto iter = s_indexBufferMap.find(hashCode);

		if (iter == s_indexBufferMap.end())
		{
			ibuffer = make_shared<IndexBuffer>();
			s_indexBufferMap[hashCode] = ibuffer;

			CreateInternal(ibuffer, data, usage);
		}
		else
		{
			ibuffer = iter->second;
		}
	}

	return ibuffer;
}


void IndexBuffer::CreateInternal(std::shared_ptr<IndexBuffer>ibuffer, const BaseIndexBufferData& data, Usage usage)
{
	// Fill in a buffer description
	D3D11_BUFFER_DESC desc{};
	
	desc.ByteWidth = static_cast<UINT>(data.GetDataSize());
	desc.Usage = (D3D11_USAGE)usage;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	// Fill in the subresource data
	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = data.GetData();
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	// Create the buffer
	ComPtr<ID3D11Buffer> buffer;
	ThrowIfFailed(g_device->CreateBuffer(&desc, (data.GetData() ? &initData : nullptr), &buffer));

	const auto& debugName = data.GetDebugName();
	if (!debugName.empty())
	{
		buffer->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(debugName.size()), debugName.c_str());
	}

	// Setup our data
	ibuffer->indexBuffer = buffer;
	ibuffer->format = data.GetFormat();
}