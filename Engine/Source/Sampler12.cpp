// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "Sampler12.h"

using namespace Kodiak;

SamplerState::SamplerState(const std::string& name)
	: m_name(name)
{
	m_desc.Filter = D3D12_FILTER_ANISOTROPIC;
	m_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	m_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	m_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	m_desc.MipLODBias = 0.0f;
	m_desc.MaxAnisotropy = 16;
	m_desc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	m_desc.BorderColor[0] = 1.0f;
	m_desc.BorderColor[1] = 1.0f;
	m_desc.BorderColor[2] = 1.0f;
	m_desc.BorderColor[3] = 1.0f;
	m_desc.MinLOD = 0.0f;
	m_desc.MaxLOD = D3D12_FLOAT32_MAX;
}