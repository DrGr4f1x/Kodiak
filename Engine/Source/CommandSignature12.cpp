// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from CommandSignature.cpp in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#include "Stdafx.h"

#include "CommandSignature12.h"

#include "DeviceManager12.h"
#include "RenderUtils.h"


using namespace Kodiak;
using namespace std;


namespace Kodiak
{
CommandSignature DispatchIndirectCommandSignature(1);
CommandSignature DrawIndirectCommandSignature(1);
}


void CommandSignature::Finalize(const RootSignature* rootSignature)
{
	if (m_finalized)
	{
		return;
	}

	UINT byteStride = 0;
	bool requiresRootSignature = false;

	for (UINT i = 0; i < m_numParameters; ++i)
	{
		switch (m_paramArray[i].GetType())
		{
		case D3D12_INDIRECT_ARGUMENT_TYPE_DRAW:
			byteStride += sizeof(D3D12_DRAW_ARGUMENTS);
			break;
		case D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED:
			byteStride += sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
			break;
		case D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH:
			byteStride += sizeof(D3D12_DISPATCH_ARGUMENTS);
			break;
		case D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT:
			requiresRootSignature = true;
			byteStride += 4;
			break;
		case D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW:
			byteStride += sizeof(D3D12_VERTEX_BUFFER_VIEW);
			break;
		case D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW:
			byteStride += sizeof(D3D12_INDEX_BUFFER_VIEW);
			break;
		case D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW:
		case D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW:
		case D3D12_INDIRECT_ARGUMENT_TYPE_UNORDERED_ACCESS_VIEW:
			byteStride += 8;
			requiresRootSignature = true;
			break;
		}
	}

	D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc;
	commandSignatureDesc.ByteStride = byteStride;
	commandSignatureDesc.NumArgumentDescs = m_numParameters;
	commandSignatureDesc.pArgumentDescs = (const D3D12_INDIRECT_ARGUMENT_DESC*)m_paramArray.get();
	commandSignatureDesc.NodeMask = 1;

	Microsoft::WRL::ComPtr<ID3DBlob> pOutBlob, pErrorBlob;

	ID3D12RootSignature* pRootSig = rootSignature ? rootSignature->GetSignature() : nullptr;
	if (requiresRootSignature)
	{
		assert(pRootSig != nullptr);
	}
	else
	{
		pRootSig = nullptr;
	}

	ThrowIfFailed(g_device->CreateCommandSignature(&commandSignatureDesc, pRootSig,	IID_PPV_ARGS(&m_signature)));

	m_signature->SetName(L"CommandSignature");

	m_finalized = TRUE;
}