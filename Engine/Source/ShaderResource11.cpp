// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "ShaderResource11.h"

#include "BinaryReader.h"
#include "DeviceManager11.h"
#include "Filesystem.h"
#include "LoaderEnums.h"
#include "RenderUtils.h"


using namespace std;
using namespace Kodiak;
using namespace Microsoft::WRL;


namespace
{

bool LoadShaderFile(const string& path, unique_ptr<byte[]>& data, size_t& dataSize)
{
	auto& filesystem = Filesystem::GetInstance();
	string fullpath = filesystem.GetFullPath(path);
	auto res = BinaryReader::ReadEntireFile(fullpath, data, &dataSize);
		
	return (res == S_OK);
}

} // anonymous namespace



bool ShaderResource::DoLoad()
{
	m_loadState = LoadState::Loading;

	// Load the compiled shader file
	unique_ptr<byte[]> data;
	size_t dataSize;

	auto res = LoadShaderFile(m_resourcePath, data, dataSize);
	if (res)
	{
		Create(data, dataSize);

		m_loadState = LoadState::LoadSucceeded;
		return true;
	}
	else
	{
		m_loadState = LoadState::LoadFailed;
		return false;
	}
}


void VertexShaderResource::Create(unique_ptr<byte[]>& data, size_t dataSize)
{
	ThrowIfFailed(g_device->CreateVertexShader(data.get(), dataSize, nullptr, &m_shader));

	ComPtr<ID3D11ShaderReflection> reflector;
	ThrowIfFailed(D3DReflect(data.get(), dataSize, IID_ID3D11ShaderReflection, &reflector));

	Introspect(reflector.Get(), m_signature);
	CreateInputLayout(reflector.Get(), data, dataSize);
}


void VertexShaderResource::CreateInputLayout(ID3D11ShaderReflection* reflector, unique_ptr<byte[]>& data, size_t dataSize)
{
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
		elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
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



void HullShaderResource::Create(unique_ptr<byte[]>& data, size_t dataSize)
{
	ThrowIfFailed(g_device->CreateHullShader(data.get(), dataSize, nullptr, &m_shader));

	ComPtr<ID3D11ShaderReflection> reflector;
	ThrowIfFailed(D3DReflect(data.get(), dataSize, IID_ID3D11ShaderReflection, &reflector));

	Introspect(reflector.Get(), m_signature);
}


void DomainShaderResource::Create(unique_ptr<byte[]>& data, size_t dataSize)
{
	ThrowIfFailed(g_device->CreateDomainShader(data.get(), dataSize, nullptr, &m_shader));

	ComPtr<ID3D11ShaderReflection> reflector;
	ThrowIfFailed(D3DReflect(data.get(), dataSize, IID_ID3D11ShaderReflection, &reflector));

	Introspect(reflector.Get(), m_signature);
}


void GeometryShaderResource::Create(unique_ptr<byte[]>& data, size_t dataSize)
{
	ThrowIfFailed(g_device->CreateGeometryShader(data.get(), dataSize, nullptr, &m_shader));

	ComPtr<ID3D11ShaderReflection> reflector;
	ThrowIfFailed(D3DReflect(data.get(), dataSize, IID_ID3D11ShaderReflection, &reflector));

	Introspect(reflector.Get(), m_signature);
}


void PixelShaderResource::Create(unique_ptr<byte[]>& data, size_t dataSize)
{
	ThrowIfFailed(g_device->CreatePixelShader(data.get(), dataSize, nullptr, &m_shader));

	ComPtr<ID3D11ShaderReflection> reflector;
	ThrowIfFailed(D3DReflect(data.get(), dataSize, IID_ID3D11ShaderReflection, &reflector));

	Introspect(reflector.Get(), m_signature);
}


void ComputeShaderResource::Create(unique_ptr<byte[]>& data, size_t dataSize)
{
	ThrowIfFailed(g_device->CreateComputeShader(data.get(), dataSize, nullptr, &m_shader));

	ComPtr<ID3D11ShaderReflection> reflector;
	ThrowIfFailed(D3DReflect(data.get(), dataSize, IID_ID3D11ShaderReflection, &reflector));

	Introspect(reflector.Get(), m_signature);
}