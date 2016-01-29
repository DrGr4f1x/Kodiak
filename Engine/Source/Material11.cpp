// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "Material.h"


using namespace Kodiak;
using namespace std;

#if 0
namespace Kodiak
{

size_t ComputeHash(const MaterialDesc& desc)
{
	return ComputeBaseHash(desc);
}

} // namespace Kodiak


Material::Material(const string& name)
	: m_name(name)
{}


void Material::BindParameters(const ShaderState& shaderState)
{}


void Material::SetupPSO(const MaterialDesc& desc)
{
	m_pso->SetBlendState(desc.blendStateDesc);
	m_pso->SetDepthStencilState(desc.depthStencilStateDesc);
	m_pso->SetRasterizerState(desc.rasterizerStateDesc);
}
#endif