// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "ConstantBuffer11.h"

#include "DeviceManager11.h"
#include "RenderEnums11.h"
#include "RenderUtils.h"


using namespace Kodiak;
using namespace std;


void ConstantBuffer::Create(size_t size, Usage usage)
{
	size = Math::AlignUp(size, 16);

	// Fill in a buffer description
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.ByteWidth = static_cast<UINT>(size);
	desc.Usage = (D3D11_USAGE)usage;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	ThrowIfFailed(g_device->CreateBuffer(&desc, nullptr, &constantBuffer));
}