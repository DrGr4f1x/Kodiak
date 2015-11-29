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

#include <ppltasks.h>

namespace Kodiak
{

// Forward declarations
class InputLayout;
struct ShaderConstantBufferDesc;
struct ShaderResourceDesc;
struct ShaderVariableDesc;
enum class ShaderResourceDimension;
enum class ShaderVariableType;


enum class ShaderType
{
	Compute,
	Domain,
	Geometry,
	Hull,
	Pixel,
	Vertex
};


class VertexShader
{
	friend class ShaderManager;

public:
	ID3D11VertexShader* GetShader() { return m_shader.Get(); }
	std::shared_ptr<InputLayout> GetInputLayout();

	bool IsReady() const { return m_isReady; }

	ShaderType GetType() const { return ShaderType::Vertex; }

	concurrency::task<void> loadTask;

private:
	void Create(std::unique_ptr<uint8_t[]>& data, size_t dataSize);
	void CreateInputLayout(ID3D11ShaderReflection* reflector, std::unique_ptr<uint8_t[]>& data, size_t dataSize);

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader>		m_shader;
	std::shared_ptr<InputLayout>					m_inputLayout;
	std::vector<ShaderConstantBufferDesc>			m_constantBuffers;
	std::vector<ShaderResourceDesc>					m_resources;
	bool											m_isReady{ false };
};


class PixelShader
{
	friend class ShaderManager;

public:
	ID3D11PixelShader* GetShader() { return m_shader.Get(); }

	bool IsReady() const { return m_isReady; }

	ShaderType GetType() const { return ShaderType::Pixel; }

	concurrency::task<void> loadTask;

private:
	void Create(std::unique_ptr<uint8_t[]>& data, size_t dataSize);

private:
	Microsoft::WRL::ComPtr<ID3D11PixelShader>		m_shader;
	std::vector<ShaderConstantBufferDesc>			m_constantBuffers;
	std::vector<ShaderResourceDesc>					m_resources;
	bool											m_isReady{ false };
};


class GeometryShader
{
	friend class ShaderManager;

public:
	ID3D11GeometryShader* GetShader() { return m_shader.Get(); }

	bool IsReady() const { return m_isReady; }

	ShaderType GetType() const { return ShaderType::Geometry; }

	concurrency::task<void> loadTask;

private:
	void Create(std::unique_ptr<uint8_t[]>& data, size_t dataSize);

private:
	Microsoft::WRL::ComPtr<ID3D11GeometryShader>	m_shader;
	std::vector<ShaderConstantBufferDesc>			m_constantBuffers;
	std::vector<ShaderResourceDesc>					m_resources;
	bool											m_isReady{ false };
};


class DomainShader
{
	friend class ShaderManager;

public:
	ID3D11DomainShader* GetShader() { return m_shader.Get(); }

	bool IsReady() const { return m_isReady; }

	ShaderType GetType() const { return ShaderType::Domain; }

	concurrency::task<void> loadTask;

private:
	void Create(std::unique_ptr<uint8_t[]>& data, size_t dataSize);

private:
	Microsoft::WRL::ComPtr<ID3D11DomainShader>		m_shader;
	std::vector<ShaderConstantBufferDesc>			m_constantBuffers;
	std::vector<ShaderResourceDesc>					m_resources;
	bool											m_isReady{ false };
};


class HullShader
{
	friend class ShaderManager;

public:
	ID3D11HullShader* GetShader() { return m_shader.Get(); }

	bool IsReady() const { return m_isReady; }

	ShaderType GetType() const { return ShaderType::Hull; }

	concurrency::task<void> loadTask;

private:
	void Create(std::unique_ptr<uint8_t[]>& data, size_t dataSize);

private:
	Microsoft::WRL::ComPtr<ID3D11HullShader>		m_shader;
	std::vector<ShaderConstantBufferDesc>			m_constantBuffers;
	std::vector<ShaderResourceDesc>					m_resources;
	bool											m_isReady{ false };
};


class ComputeShader
{
	friend class ShaderManager;

public:
	ID3D11ComputeShader* GetShader() { return m_shader.Get(); }

	bool IsReady() const { return m_isReady; }

	ShaderType GetType() const { return ShaderType::Compute; }

	concurrency::task<void> loadTask;

private:
	void Create(std::unique_ptr<uint8_t[]>& data, size_t dataSize);

private:
	Microsoft::WRL::ComPtr<ID3D11ComputeShader>		m_shader;
	std::vector<ShaderConstantBufferDesc>			m_constantBuffers;
	std::vector<ShaderResourceDesc>					m_resources;
	bool											m_isReady{ false };
};


struct ShaderConstantBufferDesc
{
	std::string name;
	uint32_t registerSlot;
	uint32_t size;
	std::vector<ShaderVariableDesc> variables;
};


struct ShaderResourceDesc
{
	std::string name;
	uint32_t slot;
	ShaderResourceDimension dimension;
};


struct ShaderVariableDesc
{
	std::string name;
	uint32_t constantBuffer;
	uint32_t startOffset;
	uint32_t size;
	ShaderVariableType type;
};


void Introspect(ID3D11ShaderReflection* reflector, std::vector<ShaderConstantBufferDesc>& constantBuffers,
	std::vector<ShaderResourceDesc>& resources);


} // namespace Kodiak