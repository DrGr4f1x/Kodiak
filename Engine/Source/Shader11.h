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

	ShaderBindingDesc								m_bindingDesc;
	std::vector<ShaderVariableDesc>					m_variables;

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

	ShaderBindingDesc								m_bindingDesc;
	std::vector<ShaderVariableDesc>					m_variables;

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

	ShaderBindingDesc								m_bindingDesc;
	std::vector<ShaderVariableDesc>					m_variables;

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

	ShaderBindingDesc								m_bindingDesc;
	std::vector<ShaderVariableDesc>					m_variables;

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

	ShaderBindingDesc								m_bindingDesc;
	std::vector<ShaderVariableDesc>					m_variables;

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

	ShaderBindingDesc								m_bindingDesc;
	std::vector<ShaderVariableDesc>					m_variables;

	bool											m_isReady{ false };
};


void Introspect(ID3D11ShaderReflection* reflector, ShaderBindingDesc& bindingDesc, std::vector<ShaderVariableDesc>& variables);


} // namespace Kodiak