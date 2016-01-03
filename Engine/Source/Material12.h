// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "ConstantBuffer12.h"
#include "PipelineState12.h"
#include "Shader.h"

#include <ppltasks.h>

namespace Kodiak
{

// Forward declarations
class ConstantBuffer;
class RootSignature;
struct ShaderConstantBufferDesc;


class Material
{
	friend class MaterialManager;

public:
	const std::string& GetName() const { return m_name; }

private:
	// To be called by MaterialManager (any thread)
	Material(const std::string& name);
	void BindParameters(const ShaderState& shaderState);
	void SetupPSO(const MaterialDesc& desc);

	// To be called internally by Material
	void ConfigureRootSignature(const ShaderState& shaderState);
	void ConfigureRootConstantBuffers(const ShaderState& shaderState, uint32_t& totalConstantBuffers);
	bool ValidateConstantBuffers(const ShaderState& shaderState, const std::string& cbufferName, uint32_t cbufferSlot);

private:
	// Graphics objects
	std::shared_ptr<RootSignature>	m_rootSig;
	std::shared_ptr<GraphicsPSO>	m_pso;

	// Material constant buffer
	struct MaterialConstantBuffer
	{
		explicit MaterialConstantBuffer(D3D12_SHADER_VISIBILITY _shaderVisibility)
			: buffer(), dataMirror(), bufferDesc(nullptr), rootSignatureSlot(0xffffffff), shaderVisibilityFlag(_shaderVisibility), isDirty(false)
		{}

		MaterialConstantBuffer(MaterialConstantBuffer&& other)
			: buffer(std::move(other.buffer))
			, dataMirror(std::move(other.dataMirror))
			, bufferDesc(other.bufferDesc)
			, rootSignatureSlot(other.rootSignatureSlot)
			, shaderVisibilityFlag(other.shaderVisibilityFlag)
			, isDirty(other.isDirty)
		{}

		MaterialConstantBuffer& operator=(const MaterialConstantBuffer& other) = default;

		std::unique_ptr<ConstantBuffer> buffer;
		std::unique_ptr<uint8_t[]>		dataMirror;
		const ShaderConstantBufferDesc*	bufferDesc;
		uint32_t						rootSignatureSlot;
		D3D12_SHADER_VISIBILITY			shaderVisibilityFlag;
		bool							isDirty;
	};
	std::vector<MaterialConstantBuffer>	m_constantBuffers;

	// Material properties
	std::string						m_name;
	bool							m_isReady{ false };
};


} // namespace Kodiak