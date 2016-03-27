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

#include "DeviceManager11.h"
#include "Log.h"
#include "Material.h"
#include "MathUtil.h"
#include "Paths.h"
#include "RenderEnums.h"
#include "RenderUtils.h"

#include <algorithm>
#include <d3d11shader.h>

using namespace Kodiak;
using namespace std;
using namespace Microsoft::WRL;


ShaderPath::ShaderPath(const string& shaderFile)
	: m_shaderPath()
	, m_shaderFile(shaderFile)
	, m_shaderFullPath()
{
	m_shaderFullPath = Paths::GetInstance().ShaderDir() + "SM5\\" + m_shaderFile;
}


ShaderPath::ShaderPath(const string& shaderPath, const string& shaderFile)
	: m_shaderPath(shaderPath)
	, m_shaderFile(shaderFile)
	, m_shaderFullPath()
{
	m_shaderFullPath = Paths::GetInstance().ShaderDir() + m_shaderPath + "\\SM5\\" + m_shaderFile;
}


void VertexShader::Create(unique_ptr<uint8_t[]>& data, size_t dataSize)
{
	ThrowIfFailed(g_device->CreateVertexShader(data.get(), dataSize, nullptr, &m_shader));
	
	ComPtr<ID3D11ShaderReflection> reflector;
	ThrowIfFailed(D3DReflect(data.get(), dataSize, IID_ID3D11ShaderReflection, &reflector));

	Introspect(reflector.Get(), m_signature);
	CreateInputLayout(reflector.Get(), data, dataSize);
}


void VertexShader::CreateInputLayout(ID3D11ShaderReflection* reflector, unique_ptr<uint8_t[]>& data, size_t dataSize)
{
	uint32_t byteOffset = 0;
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;

	// Get shader info
	D3D11_SHADER_DESC shaderDesc;
	reflector->GetDesc(&shaderDesc);

	for (uint32_t i = 0; i < shaderDesc.InputParameters; ++i)
	{
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
		reflector->GetInputParameterDesc(i, &paramDesc);

		// Fill out input element desc
		D3D11_INPUT_ELEMENT_DESC elementDesc;
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.InputSlot = 0;
		elementDesc.AlignedByteOffset = byteOffset;
		elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		// Skip certain system value semantics
		string upper = elementDesc.SemanticName;
		transform(begin(upper), end(upper), begin(upper), ::toupper);

		if(upper == "SV_INSTANCEID" || upper == "SV_VERTEXID" || upper == "SV_PRIMITIVEID")
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
		inputLayoutDesc.push_back(elementDesc);
	}

	// Try to create input layout
	if (!inputLayoutDesc.empty())
	{
		ThrowIfFailed(g_device->CreateInputLayout(
			&inputLayoutDesc[0],
			static_cast<UINT>(inputLayoutDesc.size()),
			data.get(),
			dataSize,
			&m_inputLayout));
	}
}


void PixelShader::Create(unique_ptr<uint8_t[]>& data, size_t dataSize)
{
	ThrowIfFailed(g_device->CreatePixelShader(data.get(), dataSize, nullptr, &m_shader));

	ComPtr<ID3D11ShaderReflection> reflector;
	ThrowIfFailed(D3DReflect(data.get(), dataSize, IID_ID3D11ShaderReflection, &reflector));

	Introspect(reflector.Get(), m_signature);
}


void GeometryShader::Create(unique_ptr<uint8_t[]>& data, size_t dataSize)
{
	ThrowIfFailed(g_device->CreateGeometryShader(data.get(), dataSize, nullptr, &m_shader));

	ComPtr<ID3D11ShaderReflection> reflector;
	ThrowIfFailed(D3DReflect(data.get(), dataSize, IID_ID3D11ShaderReflection, &reflector));

	Introspect(reflector.Get(), m_signature);
}


void DomainShader::Create(unique_ptr<uint8_t[]>& data, size_t dataSize)
{
	ThrowIfFailed(g_device->CreateDomainShader(data.get(), dataSize, nullptr, &m_shader));

	ComPtr<ID3D11ShaderReflection> reflector;
	ThrowIfFailed(D3DReflect(data.get(), dataSize, IID_ID3D11ShaderReflection, &reflector));

	Introspect(reflector.Get(), m_signature);
}


void HullShader::Create(unique_ptr<uint8_t[]>& data, size_t dataSize)
{
	ThrowIfFailed(g_device->CreateHullShader(data.get(), dataSize, nullptr, &m_shader));

	ComPtr<ID3D11ShaderReflection> reflector;
	ThrowIfFailed(D3DReflect(data.get(), dataSize, IID_ID3D11ShaderReflection, &reflector));

	Introspect(reflector.Get(), m_signature);
}


void ComputeShader::Create(unique_ptr<uint8_t[]>& data, size_t dataSize)
{
	ThrowIfFailed(g_device->CreateComputeShader(data.get(), dataSize, nullptr, &m_shader));

	ComPtr<ID3D11ShaderReflection> reflector;
	ThrowIfFailed(D3DReflect(data.get(), dataSize, IID_ID3D11ShaderReflection, &reflector));

	Introspect(reflector.Get(), m_signature);
}