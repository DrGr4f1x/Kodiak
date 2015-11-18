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

#include "DeviceManager11.h"
#include "RenderEnums11.h"
#include "RenderUtils.h"

using namespace Kodiak;
using namespace std;
using namespace Microsoft::WRL;


void VertexBuffer::Create(std::shared_ptr<BaseVertexBufferData> data, Usage usage, const std::string& debugName)
{
	loadTask = concurrency::create_task([this, data, usage, debugName]()
	{
		// Fill in a buffer description
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
		desc.ByteWidth = static_cast<UINT>(data->GetDataSize());
		desc.Usage = (D3D11_USAGE)usage;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = (usage == Usage::Dynamic) ? D3D11_CPU_ACCESS_WRITE : 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;

		// Fill in the subresource data
		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = data->GetData();
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		// Create the buffer
		ComPtr<ID3D11Buffer> buffer;
		ThrowIfFailed(g_device->CreateBuffer(&desc, (data->GetData() ? &initData : nullptr), &buffer));

		buffer->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(debugName.size()), debugName.c_str());

		// Setup our data
		vertexBuffer = buffer;
		stride = static_cast<uint32_t>(data->GetStride());
	});
}