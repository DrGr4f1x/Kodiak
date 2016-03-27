// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from RootSignature.cpp in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#include "Stdafx.h"

#include "RootSignature12.h"

#include "DebugUtility.h"
#include "DeviceManager12.h"
#include "RenderUtils.h"


using namespace Kodiak;
using namespace std;
using namespace Microsoft::WRL;


static std::map<size_t, ComPtr<ID3D12RootSignature>> s_rootSignatureHashMap;


void RootSignature::DestroyAll(void)
{
	s_rootSignatureHashMap.clear();
}


void RootSignature::InitStaticSampler(
	uint32_t _register,
	const D3D12_SAMPLER_DESC& nonStaticSamplerDesc,
	D3D12_SHADER_VISIBILITY visibility)
{
	assert(m_numInitializedStaticSamplers < m_numSamplers);
	D3D12_STATIC_SAMPLER_DESC& staticSamplerDesc = m_samplerArray[m_numInitializedStaticSamplers++];

	staticSamplerDesc.Filter = nonStaticSamplerDesc.Filter;
	staticSamplerDesc.AddressU = nonStaticSamplerDesc.AddressU;
	staticSamplerDesc.AddressV = nonStaticSamplerDesc.AddressV;
	staticSamplerDesc.AddressW = nonStaticSamplerDesc.AddressW;
	staticSamplerDesc.MipLODBias = nonStaticSamplerDesc.MipLODBias;
	staticSamplerDesc.MaxAnisotropy = nonStaticSamplerDesc.MaxAnisotropy;
	staticSamplerDesc.ComparisonFunc = nonStaticSamplerDesc.ComparisonFunc;
	staticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	staticSamplerDesc.MinLOD = nonStaticSamplerDesc.MinLOD;
	staticSamplerDesc.MaxLOD = nonStaticSamplerDesc.MaxLOD;
	staticSamplerDesc.ShaderRegister = _register;
	staticSamplerDesc.RegisterSpace = 0;
	staticSamplerDesc.ShaderVisibility = visibility;

	if (staticSamplerDesc.AddressU == D3D12_TEXTURE_ADDRESS_MODE_BORDER ||
		staticSamplerDesc.AddressV == D3D12_TEXTURE_ADDRESS_MODE_BORDER ||
		staticSamplerDesc.AddressW == D3D12_TEXTURE_ADDRESS_MODE_BORDER)
	{
		warn_once_if_not(
			// Transparent Black
			nonStaticSamplerDesc.BorderColor[0] == 0.0f &&
			nonStaticSamplerDesc.BorderColor[1] == 0.0f &&
			nonStaticSamplerDesc.BorderColor[2] == 0.0f &&
			nonStaticSamplerDesc.BorderColor[3] == 0.0f ||
			// Opaque Black
			nonStaticSamplerDesc.BorderColor[0] == 0.0f &&
			nonStaticSamplerDesc.BorderColor[1] == 0.0f &&
			nonStaticSamplerDesc.BorderColor[2] == 0.0f &&
			nonStaticSamplerDesc.BorderColor[3] == 1.0f ||
			// Opaque White
			nonStaticSamplerDesc.BorderColor[0] == 1.0f &&
			nonStaticSamplerDesc.BorderColor[1] == 1.0f &&
			nonStaticSamplerDesc.BorderColor[2] == 1.0f &&
			nonStaticSamplerDesc.BorderColor[3] == 1.0f,
			"Sampler border color does not match static sampler limitations");

		if(nonStaticSamplerDesc.BorderColor[3] == 1.0f)
		{
			if(nonStaticSamplerDesc.BorderColor[0] == 1.0f)
			{
				staticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
			}
			else
			{
				staticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
			}
		}
	}
}


void RootSignature::Finalize(D3D12_ROOT_SIGNATURE_FLAGS flags)
{
	if(m_finalized)
	{
		return;
	}

	assert(m_numInitializedStaticSamplers == m_numSamplers);

	D3D12_ROOT_SIGNATURE_DESC rootDesc;
	rootDesc.NumParameters = m_numParameters;
	rootDesc.pParameters = (const D3D12_ROOT_PARAMETER*)m_paramArray.get();
	rootDesc.NumStaticSamplers = m_numSamplers;
	rootDesc.pStaticSamplers = (const D3D12_STATIC_SAMPLER_DESC*)m_samplerArray.get();
	rootDesc.Flags = flags;

	m_descriptorTableBitMap = 0;
	m_maxDescriptorCacheHandleCount = 0;

	size_t hashCode = HashStateArray(rootDesc.pStaticSamplers, m_numSamplers);

	for(uint32_t param = 0; param < m_numParameters; ++param)
	{
		const D3D12_ROOT_PARAMETER& rootParam = rootDesc.pParameters[param];
		m_descriptorTableSize[param] = 0;

		if(rootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
		{
			assert(rootParam.DescriptorTable.pDescriptorRanges != nullptr);

			hashCode = HashStateArray(rootParam.DescriptorTable.pDescriptorRanges,
				rootParam.DescriptorTable.NumDescriptorRanges, hashCode);

			// We don't care about sampler descriptor tables.  We don't manage them in DescriptorCache
			if(rootParam.DescriptorTable.pDescriptorRanges->RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
			{
				continue;
			}

			m_descriptorTableBitMap |= (1 << param);
			for (uint32_t tableRange = 0; tableRange < rootParam.DescriptorTable.NumDescriptorRanges; ++tableRange)
			{
				m_descriptorTableSize[param] += rootParam.DescriptorTable.pDescriptorRanges[tableRange].NumDescriptors;
			}

			m_maxDescriptorCacheHandleCount += m_descriptorTableSize[param];
		}
		else
		{
			hashCode = HashState(&rootParam, hashCode);
		}
	}

	ID3D12RootSignature** rsRef = nullptr;
	bool firstCompile = false;
	{
		static mutex s_hashMapMutex;
		lock_guard<mutex> CS(s_hashMapMutex);

		auto iter = s_rootSignatureHashMap.find(hashCode);

		// Reserve space so the next inquiry will find that someone got here first.
		if(iter == s_rootSignatureHashMap.end())
		{
			rsRef = s_rootSignatureHashMap[hashCode].GetAddressOf();
			firstCompile = true;
		}
		else
		{
			rsRef = iter->second.GetAddressOf();
		}
	}

	if(firstCompile)
	{
		ComPtr<ID3DBlob> pOutBlob, pErrorBlob;

		HRESULT hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			pOutBlob.GetAddressOf(), pErrorBlob.GetAddressOf());
		if(FAILED(hr))
		{ 
			if (pErrorBlob)
			{
				OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			}
		}

		ThrowIfFailed(g_device->CreateRootSignature(1, pOutBlob->GetBufferPointer(), pOutBlob->GetBufferSize(),
			IID_PPV_ARGS(&m_signature)));

		s_rootSignatureHashMap[hashCode].Attach(m_signature);
		assert(*rsRef == m_signature);
	}
	else
	{
		while (*rsRef == nullptr)
		{
			this_thread::yield();
		}
		m_signature = *rsRef;
	}

	m_finalized = true;
}