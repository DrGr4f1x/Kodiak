// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "Shader.h"

#include "InputLayout12.h"
#include "Log.h"
#include "Material.h"
#include "Paths.h"
#include "RenderEnums.h"
#include "RenderUtils.h"

#include <algorithm>
#include <d3d12shader.h>

using namespace Kodiak;
using namespace std;
using namespace Microsoft::WRL;


ShaderPath::ShaderPath(const string& shaderFile)
	: m_shaderPath()
	, m_shaderFile(shaderFile)
	, m_shaderFullPath()
{
	m_shaderFullPath = Paths::GetInstance().ShaderDir() + "SM5.1\\" + m_shaderFile;
}


ShaderPath::ShaderPath(const string& shaderPath, const string& shaderFile)
	: m_shaderPath(shaderPath)
	, m_shaderFile(shaderFile)
	, m_shaderFullPath()
{
	m_shaderFullPath = Paths::GetInstance().ShaderDir() + m_shaderPath + "\\SM5.1\\" + m_shaderFile;
}


void Shader::Finalize()
{
	ComPtr<ID3D12ShaderReflection> reflector;
	ThrowIfFailed(D3DReflect(m_byteCode.get(), m_byteCodeSize, IID_ID3D12ShaderReflection, &reflector));

	Introspect(reflector.Get(), m_signature);
}


void VertexShader::Finalize()
{
	ComPtr<ID3D12ShaderReflection> reflector;
	ThrowIfFailed(D3DReflect(m_byteCode.get(), m_byteCodeSize, IID_ID3D12ShaderReflection, &reflector));

	Introspect(reflector.Get(), m_signature);
	CreateInputLayout(reflector.Get());
}


void VertexShader::CreateInputLayout(ID3D12ShaderReflection* reflector)
{
	uint32_t byteOffset = 0;
	
	// Get shader info
	D3D12_SHADER_DESC shaderDesc;
	reflector->GetDesc(&shaderDesc);

	m_inputLayout.elements.reserve(shaderDesc.InputParameters);
	m_inputLayout.semantics.reserve(shaderDesc.InputParameters);

	for (uint32_t i = 0; i < shaderDesc.InputParameters; ++i)
	{
		D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
		reflector->GetInputParameterDesc(i, &paramDesc);

		// Fill out input element desc
		D3D12_INPUT_ELEMENT_DESC elementDesc;
		
		// NOTE: if we simply do:
		//   elementDesc.SemanticName = paramDesc.SemanticName;
		// then we have a lifetime problem with the string on elementDesc.  Store a copy of the string
		// and point elementDesc.SemanticName to c_str().  This is not in general safe, but we know
		// our list of strings has the correct size, so it won't reallocate/move the strings out
		// from under us.
		m_inputLayout.semantics.push_back(string(paramDesc.SemanticName));
		elementDesc.SemanticName = m_inputLayout.semantics.back().c_str();

		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.InputSlot = 0;
		elementDesc.AlignedByteOffset = byteOffset;
		elementDesc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		// Skip certain system value semantics
		string upper = elementDesc.SemanticName;
		transform(begin(upper), end(upper), begin(upper), ::toupper);

		if (upper == "SV_INSTANCEID" || upper == "SV_VERTEXID" || upper == "SV_PRIMITIVEID")
		{
			continue;
		}

		// Determine DXGI format
		if (paramDesc.Mask == 1)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
			{
				elementDesc.Format = DXGI_FORMAT_R32_UINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
			{
				elementDesc.Format = DXGI_FORMAT_R32_SINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
			{
				elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
			}
			byteOffset += 4;
		}
		else if (paramDesc.Mask <= 3)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
			{
				elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
			{
				elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
			{
				elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
			}
			byteOffset += 8;
		}
		else if (paramDesc.Mask <= 7)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
			{
				elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
			{
				elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
			{
				elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			}
			byteOffset += 12;
		}
		else if (paramDesc.Mask <= 15)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
			{
				elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
			{
				elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
			{
				elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			}
			byteOffset += 16;
		}

		// Save element desc
		m_inputLayout.elements.push_back(elementDesc);
	}
}