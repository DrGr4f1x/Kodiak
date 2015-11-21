// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "Shader12.h"

#include "InputLayout12.h"
#include "RenderUtils.h"

#include <algorithm>
#include <d3d12shader.h>

using namespace Kodiak;
using namespace std;
using namespace Microsoft::WRL;


void VertexShader::Finalize()
{
	ComPtr<ID3D12ShaderReflection> reflector;
	ThrowIfFailed(D3DReflect(m_byteCode.get(), m_byteCodeSize, IID_ID3D12ShaderReflection, &reflector));

	Introspect(reflector.Get(), m_constantBuffers, m_resources);
	CreateInputLayout(reflector.Get());
}


void VertexShader::CreateInputLayout(ID3D12ShaderReflection* reflector)
{
	m_inputLayout = make_shared<InputLayout>();

	uint32_t byteOffset = 0;
	
	// Get shader info
	D3D12_SHADER_DESC shaderDesc;
	reflector->GetDesc(&shaderDesc);

	m_inputLayout->elements.reserve(shaderDesc.InputParameters);

	for (uint32_t i = 0; i < shaderDesc.InputParameters; ++i)
	{
		D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
		reflector->GetInputParameterDesc(i, &paramDesc);

		// Fill out input element desc
		D3D12_INPUT_ELEMENT_DESC elementDesc;
		elementDesc.SemanticName = paramDesc.SemanticName;
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
		m_inputLayout->elements.emplace_back(elementDesc);
	}
}


void PixelShader::Finalize()
{
	ComPtr<ID3D12ShaderReflection> reflector;
	ThrowIfFailed(D3DReflect(m_byteCode.get(), m_byteCodeSize, IID_ID3D12ShaderReflection, &reflector));

	Introspect(reflector.Get(), m_constantBuffers, m_resources);
}


void GeometryShader::Finalize()
{
	ComPtr<ID3D12ShaderReflection> reflector;
	ThrowIfFailed(D3DReflect(m_byteCode.get(), m_byteCodeSize, IID_ID3D12ShaderReflection, &reflector));

	Introspect(reflector.Get(), m_constantBuffers, m_resources);
}


void DomainShader::Finalize()
{
	ComPtr<ID3D12ShaderReflection> reflector;
	ThrowIfFailed(D3DReflect(m_byteCode.get(), m_byteCodeSize, IID_ID3D12ShaderReflection, &reflector));

	Introspect(reflector.Get(), m_constantBuffers, m_resources);
}


void HullShader::Finalize()
{
	ComPtr<ID3D12ShaderReflection> reflector;
	ThrowIfFailed(D3DReflect(m_byteCode.get(), m_byteCodeSize, IID_ID3D12ShaderReflection, &reflector));

	Introspect(reflector.Get(), m_constantBuffers, m_resources);
}


void ComputeShader::Finalize()
{
	ComPtr<ID3D12ShaderReflection> reflector;
	ThrowIfFailed(D3DReflect(m_byteCode.get(), m_byteCodeSize, IID_ID3D12ShaderReflection, &reflector));

	Introspect(reflector.Get(), m_constantBuffers, m_resources);
}


namespace Kodiak
{

void Introspect(ID3D12ShaderReflection* reflector, vector<ShaderConstantBufferDesc>& constantBuffer,
	vector<ShaderResourceDesc>& resources)
{

}

} // namespace Kodiak