// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "IAsyncResource.h"
#include "InputLayout12.h"
#include "RenderEnums.h"
#include "ShaderReflection.h"

namespace Kodiak
{

class ShaderResource : public IAsyncResource
{
public:
	virtual ShaderType GetType() const = 0;

	// Resource loader interface
	bool DoLoad() final override;

	const byte* GetByteCode() const { return m_byteCode.get(); }
	size_t GetByteCodeSize() const { return m_byteCodeSize; }

	// Reflection info
	uint32_t GetPerViewDataSize() const { return m_signature.cbvPerViewData.sizeInBytes; }
	uint32_t GetPerObjectDataSize() const { return m_signature.cbvPerObjectData.sizeInBytes; }
	const struct ShaderReflection::Signature& GetSignature() const { return m_signature; }

protected:
	virtual void Finalize();

protected:
	std::unique_ptr<byte[]>			m_byteCode;
	size_t							m_byteCodeSize;

	ShaderReflection::Signature		m_signature;
};


class VertexShaderResource : public ShaderResource
{
	friend class ShaderManager;

public:
	const InputLayout& GetInputLayout() const { return m_inputLayout; }

	ShaderType GetType() const override { return ShaderType::Vertex; }

private:
	void Finalize() final override;
	void CreateInputLayout(ID3D12ShaderReflection* reflector);

private:
	InputLayout			m_inputLayout;
};


class PixelShaderResource : public ShaderResource
{
public:
	ShaderType GetType() const override { return ShaderType::Pixel; }
};


class DomainShaderResource : public ShaderResource
{
public:
	ShaderType GetType() const override { return ShaderType::Domain; }
};


class HullShaderResource : public ShaderResource
{
public:
	ShaderType GetType() const override { return ShaderType::Hull; }
};


class GeometryShaderResource : public ShaderResource
{
public:
	ShaderType GetType() const override { return ShaderType::Geometry; }
};


class ComputeShaderResource : public ShaderResource
{
public:
	ShaderType GetType() const override { return ShaderType::Compute; }
};

} // namespace Kodiak
