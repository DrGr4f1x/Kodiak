// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "CommandListManager11.h"

#include "RenderUtils.h"


using namespace Kodiak;
using namespace Microsoft::WRL;
using namespace std;


void CommandListManager::Create(ID3D11Device* device)
{
	assert(nullptr != device);

	m_device = device;
}


void CommandListManager::CreateNewDeferredContext(ID3D11DeviceContext** context)
{
	ThrowIfFailed(m_device->CreateDeferredContext(0, context));
}


uint64_t CommandListManager::ExecuteCommandList(ID3D11CommandList* commandList)
{
	ComPtr<ID3D11DeviceContext> immediateContext;
	
	m_device->GetImmediateContext(&immediateContext);
	immediateContext->ExecuteCommandList(commandList, FALSE);

	commandList->Release();

	return 0;
}