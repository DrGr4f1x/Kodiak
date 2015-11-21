// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

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


class VertexShader
{
	friend class ShaderManager;

public:
	const uint8_t* GetByteCode() const { return m_byteCode.get(); }
	size_t GetByteCodeSize() const { return m_byteCodeSize; }

	std::shared_ptr<InputLayout> GetInputLayout() { return m_inputLayout; }

	bool IsReady() const { return m_isReady; }

	ShaderType GetType() const { return ShaderType::Vertex; }

	concurrency::task<void> loadTask;

private:
	void Finalize();
	void CreateInputLayout(ID3D12ShaderReflection* reflector);

private:
	std::unique_ptr<uint8_t[]>				m_byteCode;
	size_t									m_byteCodeSize;
	std::shared_ptr<InputLayout>			m_inputLayout;
	std::vector<ShaderConstantBufferDesc>	m_constantBuffers;
	std::vector<ShaderResourceDesc>			m_resources;
	bool									m_isReady{ false };
};



class PixelShader
{
	friend class ShaderManager;

public:
	const uint8_t* GetByteCode() const { return m_byteCode.get(); }
	size_t GetByteCodeSize() const { return m_byteCodeSize; }

	bool IsReady() const { return m_isReady; }

	ShaderType GetType() const { return ShaderType::Pixel; }

	concurrency::task<void> loadTask;

private:
	void Finalize();

private:
	std::unique_ptr<uint8_t[]>				m_byteCode;
	size_t									m_byteCodeSize;
	std::vector<ShaderConstantBufferDesc>	m_constantBuffers;
	std::vector<ShaderResourceDesc>			m_resources;
	bool									m_isReady{ false };
};


class GeometryShader
{
	friend class ShaderManager;

public:
	const uint8_t* GetByteCode() const { return m_byteCode.get(); }
	size_t GetByteCodeSize() const { return m_byteCodeSize; }

	bool IsReady() const { return m_isReady; }

	ShaderType GetType() const { return ShaderType::Geometry; }

	concurrency::task<void> loadTask;

private:
	void Finalize();

private:
	std::unique_ptr<uint8_t[]>				m_byteCode;
	size_t									m_byteCodeSize;
	std::vector<ShaderConstantBufferDesc>	m_constantBuffers;
	std::vector<ShaderResourceDesc>			m_resources;
	bool									m_isReady{ false };
};


class DomainShader
{
	friend class ShaderManager;

public:
	const uint8_t* GetByteCode() const { return m_byteCode.get(); }
	size_t GetByteCodeSize() const { return m_byteCodeSize; }

	bool IsReady() const { return m_isReady; }

	ShaderType GetType() const { return ShaderType::Domain; }

	concurrency::task<void> loadTask;

private:
	void Finalize();

private:
	std::unique_ptr<uint8_t[]>				m_byteCode;
	size_t									m_byteCodeSize;
	std::vector<ShaderConstantBufferDesc>	m_constantBuffers;
	std::vector<ShaderResourceDesc>			m_resources;
	bool									m_isReady{ false };
};


class HullShader
{
	friend class ShaderManager;

public:
	const uint8_t* GetByteCode() const { return m_byteCode.get(); }
	size_t GetByteCodeSize() const { return m_byteCodeSize; }

	bool IsReady() const { return m_isReady; }

	ShaderType GetType() const { return ShaderType::Hull; }

	concurrency::task<void> loadTask;

private:
	void Finalize();

private:
	std::unique_ptr<uint8_t[]>				m_byteCode;
	size_t									m_byteCodeSize;
	std::vector<ShaderConstantBufferDesc>	m_constantBuffers;
	std::vector<ShaderResourceDesc>			m_resources;
	bool									m_isReady{ false };
};


class ComputeShader
{
	friend class ShaderManager;

public:
	const uint8_t* GetByteCode() const { return m_byteCode.get(); }
	size_t GetByteCodeSize() const { return m_byteCodeSize; }

	bool IsReady() const { return m_isReady; }

	ShaderType GetType() const { return ShaderType::Compute; }

	concurrency::task<void> loadTask;

private:
	void Finalize();

private:
	std::unique_ptr<uint8_t[]>				m_byteCode;
	size_t									m_byteCodeSize;
	std::vector<ShaderConstantBufferDesc>	m_constantBuffers;
	std::vector<ShaderResourceDesc>			m_resources;
	bool									m_isReady{ false };
};


// TODO: this shit
struct ShaderConstantBufferDesc {};
struct ShaderResourceDesc {};
struct ShaderVariableDesc {};
void Introspect(ID3D12ShaderReflection* reflector, std::vector<ShaderConstantBufferDesc>& constantBuffers,
	std::vector<ShaderResourceDesc>& resources);


} // namespace Kodiak