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
	friend class ShaderManager;

public:
	const uint8_t* GetByteCode() const { return m_byteCode.get(); }
	size_t GetByteCodeSize() const { return m_byteCodeSize; }

	bool IsReady() const { return m_isReady; }
	virtual ShaderType GetType() const = 0;

	// Reflection info
	uint32_t GetPerViewDataSize() const { return m_signature.cbvPerViewData.sizeInBytes; }
	uint32_t GetPerObjectDataSize() const { return m_signature.cbvPerObjectData.sizeInBytes; }
	const struct ShaderReflection::Signature& GetSignature() const { return m_signature; }

	concurrency::task<void> loadTask;

protected:
	virtual void Finalize();

protected:
	std::unique_ptr<uint8_t[]>		m_byteCode;
	size_t							m_byteCodeSize;
	
	ShaderReflection::Signature		m_signature;
	bool							m_isReady{ false };
};


class VertexShader : public Shader
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


class PixelShader : public Shader
{
public:
	ShaderType GetType() const override { return ShaderType::Pixel; }
};


class DomainShader : public Shader
{
public:
	ShaderType GetType() const override { return ShaderType::Domain; }
};


class HullShader : public Shader
{
public:
	ShaderType GetType() const override { return ShaderType::Hull; }
};


class GeometryShader : public Shader
{
public:
	ShaderType GetType() const override { return ShaderType::Geometry; }
};


class ComputeShader : public Shader
{
public:
	ShaderType GetType() const override { return ShaderType::Compute; }
};


} // namespace Kodiak