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


namespace
{

ShaderVariableType ConvertToEngine(D3D_SHADER_VARIABLE_TYPE type, uint32_t rows, uint32_t columns)
{
	if (type == D3D_SVT_INT)
	{
		if (rows != 1)
		{
			LOG_ERROR << "Integer matrix shader variable not supported yet";
			return ShaderVariableType::Unsupported;
		}
		switch (columns)
		{
		case 1: return ShaderVariableType::Int;
		case 2: return ShaderVariableType::Int2;
		case 3: return ShaderVariableType::Int3;
		default: return ShaderVariableType::Int4;
		}
	}

	if (type == D3D_SVT_FLOAT)
	{
		if (rows == 1)
		{
			switch (columns)
			{
			case 1: return ShaderVariableType::Float;
			case 2: return ShaderVariableType::Float2;
			case 3: return ShaderVariableType::Float3;
			default: return ShaderVariableType::Float4;
			}
		}
		else if (rows == 4 && columns == 4)
		{
			return ShaderVariableType::Float4x4;
		}
		else
		{
			LOG_ERROR << "Shader variables of type float" << rows << "x" << columns << " are not supported yet";
			return ShaderVariableType::Unsupported;
		}
	}

	LOG_ERROR << "Shader type " << (int32_t)type << " not supported";
	return ShaderVariableType::Unsupported;
}


ShaderResourceDimension ConvertToEngine(D3D_SRV_DIMENSION dim)
{
	switch (dim)
	{
	case D3D12_SRV_DIMENSION_BUFFER:			return ShaderResourceDimension::Buffer;
	case D3D12_SRV_DIMENSION_TEXTURE1D:			return ShaderResourceDimension::Texture1d;
	case D3D12_SRV_DIMENSION_TEXTURE1DARRAY:	return ShaderResourceDimension::Texture1dArray;
	case D3D12_SRV_DIMENSION_TEXTURE2D:			return ShaderResourceDimension::Texture2d;
	case D3D12_SRV_DIMENSION_TEXTURE2DARRAY:	return ShaderResourceDimension::Texture2dArray;
	case D3D12_SRV_DIMENSION_TEXTURE2DMS:		return ShaderResourceDimension::Texture2dMS;
	case D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY:	return ShaderResourceDimension::Texture2dMSArray;
	case D3D12_SRV_DIMENSION_TEXTURE3D:			return ShaderResourceDimension::Texture3d;
	case D3D12_SRV_DIMENSION_TEXTURECUBE:		return ShaderResourceDimension::TextureCube;
	case D3D12_SRV_DIMENSION_TEXTURECUBEARRAY:	return ShaderResourceDimension::TextureCubeArray;
	default:
		LOG_ERROR << "Shader resource with dimension " << dim << " are not supported yet!";
		return ShaderResourceDimension::Unsupported;
	}
}


} // anonymous namespace


namespace Kodiak
{

void IntrospectCBuffer(vector<ShaderConstantBufferDesc>& constantBuffers, ID3D12ShaderReflection* reflector,
	const D3D12_SHADER_INPUT_BIND_DESC& inputDesc)
{
	// Grab the D3D11 constant buffer description
	auto reflCBuffer = reflector->GetConstantBufferByName(inputDesc.Name);
	assert(reflCBuffer);

	D3D12_SHADER_BUFFER_DESC bufferDesc;
	reflCBuffer->GetDesc(&bufferDesc);

	uint32_t constantBufferIndex = static_cast<uint32_t>(constantBuffers.size());

	// Create our own description for the constant buffer
	constantBuffers.emplace_back(bufferDesc.Name, inputDesc.BindPoint, bufferDesc.Size);
	auto& shaderBufferDesc = constantBuffers[constantBufferIndex];

	// Variables in constant buffer
	for (uint32_t j = 0; j < bufferDesc.Variables; ++j)
	{
		// Grab the D3D11 variable & type descriptions
		auto variable = reflCBuffer->GetVariableByIndex(j);
		D3D12_SHADER_VARIABLE_DESC varDesc;
		variable->GetDesc(&varDesc);
		auto reflType = variable->GetType();
		D3D12_SHADER_TYPE_DESC typeDesc;
		reflType->GetDesc(&typeDesc);

		// Create our own description for the variable
		auto varType = ConvertToEngine(typeDesc.Type, typeDesc.Rows, typeDesc.Columns);
		shaderBufferDesc.variables.emplace_back(varDesc.Name, constantBufferIndex, varDesc.StartOffset, varDesc.Size, varType);
	}
}


void IntrospectTexture(vector<ShaderResourceDesc>& resources, const D3D12_SHADER_INPUT_BIND_DESC& inputDesc)
{
	resources.emplace_back(inputDesc.Name, inputDesc.BindPoint, ConvertToEngine(inputDesc.Dimension));
}


void Introspect(ID3D12ShaderReflection* reflector, vector<ShaderConstantBufferDesc>& constantBuffers,
	vector<ShaderResourceDesc>& resources)
{
	D3D12_SHADER_DESC desc;
	reflector->GetDesc(&desc);

	// Allocate space for constant buffers and textures
	constantBuffers.clear();
	constantBuffers.reserve(desc.BoundResources);
	resources.clear();
	resources.reserve(desc.BoundResources);

	// Bound resources
	for (uint32_t i = 0; i < desc.BoundResources; ++i)
	{
		// Grab the D3D12 bound resource description
		D3D12_SHADER_INPUT_BIND_DESC inputDesc;
		ThrowIfFailed(reflector->GetResourceBindingDesc(i, &inputDesc));

		switch (inputDesc.Type)
		{
		case D3D_SIT_CBUFFER:
			IntrospectCBuffer(constantBuffers, reflector, inputDesc);
			break;

		case D3D_SIT_TEXTURE:
			IntrospectTexture(resources, inputDesc);
			break;

		default:
			assert(false);
			break;
		}
	}
}

} // namespace Kodiak