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
#include "RenderEnums.h"
#include "ShaderReflection.h"

namespace Kodiak
{


class ShaderResource : public IAsyncResource
{
public:
	virtual ShaderType GetType() const = 0;

	// Reflection info
	uint32_t GetPerViewDataSize() const { return m_signature.cbvPerViewData.sizeInBytes; }
	uint32_t GetPerObjectDataSize() const { return m_signature.cbvPerObjectData.sizeInBytes; }
	const struct ShaderReflection::Signature& GetSignature() const { return m_signature; }

	// Resource loader interface
	bool DoLoad() final override;

protected:
	virtual void Create(std::unique_ptr<uint8_t[]>& data, size_t dataSize) = 0;

protected:
	ShaderReflection::Signature			m_signature;
};


class VertexShaderResource : public ShaderResource
{
public:
	ID3D11VertexShader* GetShader() { return m_shader.Get(); }
	ID3D11InputLayout* GetInputLayout() { return m_inputLayout.Get(); }

	ShaderType GetType() const final override { return ShaderType::Vertex; }

private:
	void Create(std::unique_ptr<uint8_t[]>& data, size_t dataSize) final override;
	void CreateInputLayout(ID3D11ShaderReflection* reflector, std::unique_ptr<uint8_t[]>& data, size_t dataSize);

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_shader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
};


class HullShaderResource : public ShaderResource
{
public:
	ID3D11HullShader* GetShader() { return m_shader.Get(); }

	ShaderType GetType() const final override { return ShaderType::Hull; }

private:
	void Create(std::unique_ptr<uint8_t[]>& data, size_t dataSize) final override;

private:
	Microsoft::WRL::ComPtr<ID3D11HullShader>	m_shader;
};


class DomainShaderResource : public ShaderResource
{
public:
	ID3D11DomainShader* GetShader() { return m_shader.Get(); }

	ShaderType GetType() const final override { return ShaderType::Domain; }

private:
	void Create(std::unique_ptr<uint8_t[]>& data, size_t dataSize) final override;

private:
	Microsoft::WRL::ComPtr<ID3D11DomainShader>	m_shader;
};


class GeometryShaderResource : public ShaderResource
{
public:
	ID3D11GeometryShader* GetShader() { return m_shader.Get(); }

	ShaderType GetType() const final override { return ShaderType::Geometry; }

private:
	void Create(std::unique_ptr<uint8_t[]>& data, size_t dataSize) final override;

private:
	Microsoft::WRL::ComPtr<ID3D11GeometryShader>	m_shader;
};


class PixelShaderResource : public ShaderResource
{
public:
	ID3D11PixelShader* GetShader() { return m_shader.Get(); }

	ShaderType GetType() const final override { return ShaderType::Pixel; }

private:
	void Create(std::unique_ptr<uint8_t[]>& data, size_t dataSize) final override;

private:
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_shader;
};


class ComputeShaderResource : public ShaderResource
{
public:
	ID3D11ComputeShader* GetShader() { return m_shader.Get(); }

	ShaderType GetType() const final override { return ShaderType::Compute; }

private:
	void Create(std::unique_ptr<uint8_t[]>& data, size_t dataSize) final override;

private:
	Microsoft::WRL::ComPtr<ID3D11ComputeShader>	m_shader;
};

} // namespace Kodiak
