// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "RenderEnums.h"
#include "ShaderReflection.h"

#include <ppltasks.h>

namespace Kodiak
{

// Forward declarations
class InputLayout;


class Shader
{
public:
	bool IsReady() const { return m_isReady; }
	virtual ShaderType GetType() const = 0;

	// Reflection info
	struct Signature;
	uint32_t GetPerViewDataSize() const { return m_signature.cbvPerViewData.sizeInBytes; }
	uint32_t GetPerObjectDataSize() const { return m_signature.cbvPerObjectData.sizeInBytes; }
	const struct Signature& GetSignature() const { return m_signature; }

	concurrency::task<void> loadTask;

	struct Signature
	{
		// DX11 API inputs
		ShaderReflection::CBVLayout						cbvPerViewData;
		ShaderReflection::CBVLayout						cbvPerObjectData;
		std::vector<ShaderReflection::CBVLayout>		cbvTable;
		std::vector<ShaderReflection::TableLayout>		srvTable;
		std::vector<ShaderReflection::TableLayout>		uavTable;
		std::vector<ShaderReflection::TableLayout>		samplerTable;

		// Application inputs
		std::vector<ShaderReflection::Parameter<1>>		parameters;
		std::vector<ShaderReflection::ResourceSRV<1>>	resources;
		std::vector<ShaderReflection::ResourceUAV<1>>	uavs;
		std::vector<ShaderReflection::Sampler<1>>		samplers;
	};

protected:
	Signature	m_signature;
	bool		m_isReady{ false };
};


class VertexShader : public Shader
{
	friend class ShaderManager;

public:
	ID3D11VertexShader* GetShader() { return m_shader.Get(); }
	std::shared_ptr<InputLayout> GetInputLayout();

	ShaderType GetType() const override { return ShaderType::Vertex; }

private:
	void Create(std::unique_ptr<uint8_t[]>& data, size_t dataSize);
	void CreateInputLayout(ID3D11ShaderReflection* reflector, std::unique_ptr<uint8_t[]>& data, size_t dataSize);

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader>		m_shader;
	std::shared_ptr<InputLayout>					m_inputLayout;
};


class PixelShader : public Shader
{
	friend class ShaderManager;

public:
	ID3D11PixelShader* GetShader() { return m_shader.Get(); }

	ShaderType GetType() const override { return ShaderType::Pixel; }

private:
	void Create(std::unique_ptr<uint8_t[]>& data, size_t dataSize);

private:
	Microsoft::WRL::ComPtr<ID3D11PixelShader>		m_shader;
};


class GeometryShader : public Shader
{
	friend class ShaderManager;

public:
	ID3D11GeometryShader* GetShader() { return m_shader.Get(); }

	ShaderType GetType() const override { return ShaderType::Geometry; }

private:
	void Create(std::unique_ptr<uint8_t[]>& data, size_t dataSize);

private:
	Microsoft::WRL::ComPtr<ID3D11GeometryShader>	m_shader;
};


class DomainShader : public Shader
{
	friend class ShaderManager;

public:
	ID3D11DomainShader* GetShader() { return m_shader.Get(); }

	ShaderType GetType() const override { return ShaderType::Domain; }

private:
	void Create(std::unique_ptr<uint8_t[]>& data, size_t dataSize);

private:
	Microsoft::WRL::ComPtr<ID3D11DomainShader>		m_shader;
};


class HullShader : public Shader
{
	friend class ShaderManager;

public:
	ID3D11HullShader* GetShader() { return m_shader.Get(); }

	ShaderType GetType() const override { return ShaderType::Hull; }

private:
	void Create(std::unique_ptr<uint8_t[]>& data, size_t dataSize);

private:
	Microsoft::WRL::ComPtr<ID3D11HullShader>		m_shader;
};


class ComputeShader : public Shader
{
	friend class ShaderManager;

public:
	ID3D11ComputeShader* GetShader() { return m_shader.Get(); }

	ShaderType GetType() const override { return ShaderType::Compute; }

private:
	void Create(std::unique_ptr<uint8_t[]>& data, size_t dataSize);

private:
	Microsoft::WRL::ComPtr<ID3D11ComputeShader>		m_shader;
};


void Introspect(ID3D11ShaderReflection* reflector, Shader::Signature& signature);


} // namespace Kodiak