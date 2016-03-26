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
#include "InputLayout11.h"
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


shared_ptr<InputLayout> VertexShader::GetInputLayout()
{
	return m_inputLayout;
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
		m_inputLayout = make_shared<InputLayout>();

		ThrowIfFailed(g_device->CreateInputLayout(
			&inputLayoutDesc[0],
			static_cast<UINT>(inputLayoutDesc.size()),
			data.get(),
			dataSize,
			&m_inputLayout->inputLayout));
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

	if (type == D3D_SVT_UINT)
	{
		if (rows != 1)
		{
			LOG_ERROR << "Unsigned integer matrix shader variable not supported yet";
			return ShaderVariableType::Unsupported;
		}
		switch (columns)
		{
		case 1: return ShaderVariableType::UInt;
		case 2: return ShaderVariableType::UInt2;
		case 3: return ShaderVariableType::UInt3;
		default: return ShaderVariableType::UInt4;
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
	case D3D11_SRV_DIMENSION_BUFFER:			return ShaderResourceDimension::Buffer;
	case D3D11_SRV_DIMENSION_TEXTURE1D:			return ShaderResourceDimension::Texture1d;
	case D3D11_SRV_DIMENSION_TEXTURE1DARRAY:	return ShaderResourceDimension::Texture1dArray;
	case D3D11_SRV_DIMENSION_TEXTURE2D:			return ShaderResourceDimension::Texture2d;
	case D3D11_SRV_DIMENSION_TEXTURE2DARRAY:	return ShaderResourceDimension::Texture2dArray;
	case D3D11_SRV_DIMENSION_TEXTURE2DMS:		return ShaderResourceDimension::Texture2dMS;
	case D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY:	return ShaderResourceDimension::Texture2dMSArray;
	case D3D11_SRV_DIMENSION_TEXTURE3D:			return ShaderResourceDimension::Texture3d;
	case D3D11_SRV_DIMENSION_TEXTURECUBE:		return ShaderResourceDimension::TextureCube;
	case D3D11_SRV_DIMENSION_TEXTURECUBEARRAY:	return ShaderResourceDimension::TextureCubeArray;
	default:
		LOG_ERROR << "Shader resource with dimension " << dim << " are not supported yet!";
		return ShaderResourceDimension::Unsupported;
	}
}


} // anonymous namespace


namespace Kodiak
{

void IntrospectCBuffer(ID3D11ShaderReflection* reflector, const D3D11_SHADER_INPUT_BIND_DESC& inputDesc, Shader::Signature& signature)
{
	// Grab the D3D11 constant buffer description
	auto reflCBuffer = reflector->GetConstantBufferByName(inputDesc.Name);
	assert(reflCBuffer);

	D3D11_SHADER_BUFFER_DESC bufferDesc;
	reflCBuffer->GetDesc(&bufferDesc);

	const std::string cbufferName(bufferDesc.Name);

	// If this cbuffer holds per-view or per-object constants, record the size for validation against other shaders
	if (cbufferName == GetPerViewConstantsName())
	{
		signature.cbvPerViewData.sizeInBytes = bufferDesc.Size;
		signature.cbvPerViewData.shaderRegister = inputDesc.BindPoint;
		return; // Don't create a record for per-view constant buffer
	}
	else if (cbufferName == GetPerObjectConstantsName())
	{
		signature.cbvPerObjectData.sizeInBytes = bufferDesc.Size;
		signature.cbvPerObjectData.shaderRegister = inputDesc.BindPoint;
		return; // Don't create a record for per-object constant buffer
	}

	// Create our own description for the constant buffer
	ShaderReflection::CBVLayout cbvLayout;
	cbvLayout.name = cbufferName;
	cbvLayout.byteOffset = 0;
	cbvLayout.sizeInBytes = bufferDesc.Size;
	cbvLayout.shaderRegister = inputDesc.BindPoint;

	signature.cbvTable.push_back(cbvLayout);
	
	// Variables in constant buffer
	for (uint32_t j = 0; j < bufferDesc.Variables; ++j)
	{
		// Grab the D3D11 variable & type descriptions
		auto variable = reflCBuffer->GetVariableByIndex(j);
		D3D11_SHADER_VARIABLE_DESC varDesc;
		variable->GetDesc(&varDesc);
		auto reflType = variable->GetType();
		D3D11_SHADER_TYPE_DESC typeDesc;
		reflType->GetDesc(&typeDesc);

		// Create our own description for the variable
		auto varType = ConvertToEngine(typeDesc.Type, typeDesc.Rows, typeDesc.Columns);

		ShaderReflection::Parameter<1> parameter;
		parameter.name = varDesc.Name;
		parameter.type = varType;
		parameter.sizeInBytes = varDesc.Size;
		parameter.cbvShaderRegister[0] = inputDesc.BindPoint;
		parameter.byteOffset[0] = varDesc.StartOffset;

		signature.parameters.push_back(parameter);
	}
}


void IntrospectResourceSRV(ShaderResourceType type, const D3D11_SHADER_INPUT_BIND_DESC& inputDesc, Shader::Signature& signature)
{
	ShaderReflection::ResourceSRV<1> resourceSRV;
	resourceSRV.name = inputDesc.Name;
	resourceSRV.type = type;
	resourceSRV.dimension = ConvertToEngine(inputDesc.Dimension);
	resourceSRV.binding[0].tableIndex = inputDesc.BindPoint; // Overload binding[0].tableIndex to temporarily hold the shader register
	signature.resources.push_back(resourceSRV);
}


void IntrospectResourceUAV(ShaderResourceType type, const D3D11_SHADER_INPUT_BIND_DESC& inputDesc, Shader::Signature& signature)
{
	ShaderReflection::ResourceUAV<1> resourceUAV;
	resourceUAV.name = inputDesc.Name;
	resourceUAV.type = type;
	resourceUAV.binding[0].tableIndex = inputDesc.BindPoint; // Overload binding[0].tableIndex to temporarily hold the shader register
	signature.uavs.push_back(resourceUAV);
}


void IntrospectSampler(const D3D11_SHADER_INPUT_BIND_DESC& inputDesc, Shader::Signature& signature)
{
	ShaderReflection::Sampler<1> sampler;
	sampler.name = inputDesc.Name;
	sampler.binding[0].tableIndex = inputDesc.BindPoint; // Overload binding[0].tableIndex to temporarily hold the shader register
	signature.samplers.push_back(sampler);
}


void ResetSignature(Shader::Signature& signature)
{
	// Reset per-view and per-object CBV bindings
	signature.cbvPerViewData.byteOffset = kInvalid;
	signature.cbvPerViewData.sizeInBytes = kInvalid;
	signature.cbvPerViewData.shaderRegister = kInvalid;

	signature.cbvPerObjectData.byteOffset = kInvalid;
	signature.cbvPerObjectData.sizeInBytes = kInvalid;
	signature.cbvPerObjectData.shaderRegister = kInvalid;

	// Clear tables
	signature.cbvTable.clear();
	signature.srvTable.clear();
	signature.uavTable.clear();
	signature.samplerTable.clear();

	// Clear parameters
	signature.parameters.clear();
	signature.resources.clear();
	signature.uavs.clear();
	signature.samplers.clear();
}


void Introspect(ID3D11ShaderReflection* reflector, Shader::Signature& signature)
{
	ResetSignature(signature);

	D3D11_SHADER_DESC desc;
	reflector->GetDesc(&desc);

	// Bound resources
	for (uint32_t i = 0; i < desc.BoundResources; ++i)
	{
		// Grab the D3D11 bound resource description
		D3D11_SHADER_INPUT_BIND_DESC inputDesc;
		ThrowIfFailed(reflector->GetResourceBindingDesc(i, &inputDesc));

		switch (inputDesc.Type)
		{
		case D3D_SIT_CBUFFER:
			IntrospectCBuffer(reflector, inputDesc, signature);
			break;

		case D3D_SIT_TBUFFER:
			IntrospectResourceSRV(ShaderResourceType::TBuffer, inputDesc, signature);
			break;

		case D3D_SIT_TEXTURE:
			IntrospectResourceSRV(ShaderResourceType::Texture, inputDesc, signature);
			break;

		case D3D_SIT_UAV_RWTYPED:
			IntrospectResourceUAV(ShaderResourceType::UAVRWTyped, inputDesc, signature);
			break;

		case D3D_SIT_STRUCTURED:
			IntrospectResourceUAV(ShaderResourceType::Structured, inputDesc, signature);
			break;

		case D3D_SIT_UAV_RWSTRUCTURED:
			IntrospectResourceUAV(ShaderResourceType::UAVRWStructured, inputDesc, signature);
			break;

		case D3D_SIT_BYTEADDRESS:
			IntrospectResourceUAV(ShaderResourceType::ByteAddress, inputDesc, signature);
			break;

		case D3D_SIT_UAV_RWBYTEADDRESS:
			IntrospectResourceUAV(ShaderResourceType::UAVRWByteAddress, inputDesc, signature);
			break;

		case D3D_SIT_UAV_APPEND_STRUCTURED:
			IntrospectResourceUAV(ShaderResourceType::UAVAppendStructured, inputDesc, signature);
			break;

		case D3D_SIT_UAV_CONSUME_STRUCTURED:
			IntrospectResourceUAV(ShaderResourceType::UAVConsumeStructured, inputDesc, signature);
			break;

		case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
			IntrospectResourceUAV(ShaderResourceType::UAVRWStructuredWithCounter, inputDesc, signature);
			break;

		case D3D_SIT_SAMPLER:
			IntrospectSampler(inputDesc, signature);
			break;

		default:
			assert(false);
			break;
		}
	}

	// Compute new cbuffer offsets, cbuffers are mapped consecutively to a contiguous memory block
	const uint32_t numCBuffers = static_cast<uint32_t>(signature.cbvTable.size());
	uint32_t currentOffset = 0;
	for (uint32_t i = 0; i < numCBuffers; ++i)
	{
		signature.cbvTable[i].byteOffset = currentOffset;
		currentOffset += signature.cbvTable[i].sizeInBytes;
	}

	// Merge SRVs into tables
	if (!signature.resources.empty())
	{
		ShaderReflection::TableLayout currentLayout;
		currentLayout.shaderRegister = signature.resources[0].binding[0].tableIndex;
		currentLayout.numItems = 1;

		const uint32_t numSRVs = static_cast<uint32_t>(signature.resources.size());
		for (uint32_t i = 1; i < numSRVs; ++i)
		{
			const auto shaderRegister = signature.resources[i].binding[0].tableIndex;
			if (shaderRegister == currentLayout.shaderRegister + currentLayout.numItems)
			{
				++currentLayout.numItems;
			}
			else
			{
				signature.srvTable.push_back(currentLayout);
				currentLayout.shaderRegister = shaderRegister;
				currentLayout.numItems = 1;
			}
		}
		signature.srvTable.push_back(currentLayout);
	}

	// Merge UAVs into tables
	if (!signature.uavs.empty())
	{
		ShaderReflection::TableLayout currentLayout;
		currentLayout.shaderRegister = signature.uavs[0].binding[0].tableIndex;
		currentLayout.numItems = 1;

		const uint32_t numUAVs = static_cast<uint32_t>(signature.uavs.size());
		for (uint32_t i = 1; i < numUAVs; ++i)
		{
			const auto shaderRegister = signature.uavs[i].binding[0].tableIndex;
			if (shaderRegister == currentLayout.shaderRegister + currentLayout.numItems)
			{
				++currentLayout.numItems;
			}
			else
			{
				signature.uavTable.push_back(currentLayout);
				currentLayout.shaderRegister = shaderRegister;
				currentLayout.numItems = 1;
			}
		}
		signature.uavTable.push_back(currentLayout);
	}

	// Merge samplers into tables
	if (!signature.samplers.empty())
	{
		ShaderReflection::TableLayout currentLayout;
		currentLayout.shaderRegister = signature.samplers[0].binding[0].tableIndex;
		currentLayout.numItems = 1;

		const uint32_t numSamplers = static_cast<uint32_t>(signature.samplers.size());
		for (uint32_t i = 1; i < numSamplers; ++i)
		{
			const auto shaderRegister = signature.samplers[i].binding[0].tableIndex;
			if (shaderRegister == currentLayout.shaderRegister + currentLayout.numItems)
			{
				++currentLayout.numItems;
			}
			else
			{
				signature.samplerTable.push_back(currentLayout);
				currentLayout.shaderRegister = shaderRegister;
				currentLayout.numItems = 1;
			}
		}
		signature.samplerTable.push_back(currentLayout);
	}

	// Remap SRVs into table ranges
	for (auto& resource : signature.resources)
	{
		bool found = false;
		uint32_t tableIndex = 0;
		const uint32_t shaderRegister = resource.binding[0].tableIndex;
		for (const auto& table : signature.srvTable)
		{
			if (shaderRegister >= table.shaderRegister && shaderRegister < (table.shaderRegister + table.numItems))
			{
				resource.binding[0].tableIndex = tableIndex;
				resource.binding[0].tableSlot = shaderRegister - table.shaderRegister;
				
				found = true;
				break;
			}
			++tableIndex;
		}
		assert(found);
	}

	// Remap UAVs into table ranges
	for (auto& uav : signature.uavs)
	{
		bool found = false;
		uint32_t tableIndex = 0;
		const uint32_t shaderRegister = uav.binding[0].tableIndex;
		for (const auto& table : signature.srvTable)
		{
			if (shaderRegister >= table.shaderRegister && shaderRegister < (table.shaderRegister + table.numItems))
			{
				uav.binding[0].tableIndex = tableIndex;
				uav.binding[0].tableSlot = shaderRegister - table.shaderRegister;

				found = true;
				break;
			}
			++tableIndex;
		}
		assert(found);
	}

	// Remap samplers into table ranges
	for (auto& sampler : signature.samplers)
	{
		bool found = false;
		uint32_t tableIndex = 0;
		const uint32_t shaderRegister = sampler.binding[0].tableIndex;
		for (const auto& table : signature.srvTable)
		{
			if (shaderRegister >= table.shaderRegister && shaderRegister < (table.shaderRegister + table.numItems))
			{
				sampler.binding[0].tableIndex = tableIndex;
				sampler.binding[0].tableSlot = shaderRegister - table.shaderRegister;

				found = true;
				break;
			}
			++tableIndex;
		}
		assert(found);
	}
}

} // namespace Kodiak