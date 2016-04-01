// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "ShaderReflection.h"

namespace Kodiak
{

// Forward declarations
enum class ShaderVariableType;
namespace RenderThread { struct MaterialData; }


class MaterialParameter : public std::enable_shared_from_this<MaterialParameter>
{
public:
	MaterialParameter(const std::string& name);

	const std::string& GetName() const { return m_name; }

	void SetValue(bool value);
	void SetValue(int32_t value);
	void SetValue(DirectX::XMINT2 value);
	void SetValue(DirectX::XMINT3 value);
	void SetValue(DirectX::XMINT4 value);
	void SetValue(uint32_t value);
	void SetValue(DirectX::XMUINT2 value);
	void SetValue(DirectX::XMUINT3 value);
	void SetValue(DirectX::XMUINT4 value);
	void SetValue(float value);
	void SetValue(DirectX::XMFLOAT2 value);
	void SetValue(Math::Vector3 value);
	void SetValue(Math::Vector4 value);
	void SetValue(const Math::Matrix4& value);

	void CreateRenderThreadData(std::shared_ptr<RenderThread::MaterialData> materialData, const ShaderReflection::Parameter<5>& parameter);
	
private:
	void UpdateParameterOnRenderThread(RenderThread::MaterialData* materialData, const std::array<byte, 64>& data);
	void SubmitToRenderThread();

private:
	const std::string		m_name;

	ShaderVariableType		m_type;
	std::array<byte, 64>	m_data;
	size_t					m_size;

	// Render thread data
	std::weak_ptr<RenderThread::MaterialData>	m_renderThreadData;
	std::array<byte*, 5>						m_bindings;
};


namespace RenderThread
{

class MaterialParameterData
{
	friend class MaterialParameter;

public:
	MaterialParameterData(std::shared_ptr<MaterialData> materialData);

	void SetValue(bool value);
	void SetValue(int32_t value);
	void SetValue(DirectX::XMINT2 value);
	void SetValue(DirectX::XMINT3 value);
	void SetValue(DirectX::XMINT4 value);
	void SetValue(uint32_t value);
	void SetValue(DirectX::XMUINT2 value);
	void SetValue(DirectX::XMUINT3 value);
	void SetValue(DirectX::XMUINT4 value);
	void SetValue(float value);
	void SetValue(DirectX::XMFLOAT2 value);
	void SetValue(DirectX::XMFLOAT3 value);
	void SetValue(DirectX::XMFLOAT4 value);
	void SetValue(DirectX::XMFLOAT4X4 value);

private:
	template <typename T>
	void InternalSetValue(T value);

private:
	ShaderVariableType		m_type;
	std::array<uint8_t, 64> m_data;
	size_t					m_size;

	std::weak_ptr<MaterialData> m_materialData;
	std::array<uint8_t*, 5>		m_bindings;
};


} // namespace RenderThread

} // namespace Kodiak
