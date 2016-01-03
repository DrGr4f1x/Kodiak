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
enum class ShaderVariable;


enum class ShaderType
{
	Compute,
	Domain,
	Geometry,
	Hull,
	Pixel,
	Vertex
};


class BaseShader
{
	friend class ShaderManager;

public:
	const uint8_t* GetByteCode() const { return m_byteCode.get(); }
	size_t GetByteCodeSize() const { return m_byteCodeSize; }

	bool IsReady() const { return m_isReady; }

	const std::vector<ShaderConstantBufferDesc>& GetConstantBuffers() const { return m_constantBuffers; }
	const std::vector<ShaderResourceDesc>& GetResources() const { return m_resources; }

	virtual ShaderType GetType() const = 0;

	concurrency::task<void> loadTask;

protected:
	virtual void Finalize();

protected:
	std::unique_ptr<uint8_t[]>				m_byteCode;
	size_t									m_byteCodeSize;
	
	std::vector<ShaderConstantBufferDesc>	m_constantBuffers;
	std::vector<ShaderResourceDesc>			m_resources;

	bool									m_isReady{ false };
};


class VertexShader : public BaseShader
{
	friend class ShaderManager;

public:
	std::shared_ptr<InputLayout> GetInputLayout() { return m_inputLayout; }

	ShaderType GetType() const override { return ShaderType::Vertex; }

private:
	void Finalize() override;
	void CreateInputLayout(ID3D12ShaderReflection* reflector);

private:
	std::shared_ptr<InputLayout>			m_inputLayout;
};


class PixelShader : public BaseShader
{
public:
	ShaderType GetType() const override { return ShaderType::Pixel; }
};


class DomainShader : public BaseShader
{
public:
	ShaderType GetType() const override { return ShaderType::Domain; }
};


class HullShader : public BaseShader
{
public:
	ShaderType GetType() const override { return ShaderType::Hull; }
};


class GeometryShader : public BaseShader
{
public:
	ShaderType GetType() const override { return ShaderType::Geometry; }
};


class ComputeShader : public BaseShader
{
public:
	ShaderType GetType() const override { return ShaderType::Compute; }
};


void Introspect(ID3D12ShaderReflection* reflector, std::vector<ShaderConstantBufferDesc>& constantBuffers,
	std::vector<ShaderResourceDesc>& resources);


} // namespace Kodiak