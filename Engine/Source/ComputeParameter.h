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
namespace RenderThread { struct ComputeData; }

class ComputeParameter : public std::enable_shared_from_this<ComputeParameter>
{
public:
	ComputeParameter(const std::string& name);

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

	void SetValueImmediate(bool value);
	void SetValueImmediate(int32_t value);
	void SetValueImmediate(DirectX::XMINT2 value);
	void SetValueImmediate(DirectX::XMINT3 value);
	void SetValueImmediate(DirectX::XMINT4 value);
	void SetValueImmediate(uint32_t value);
	void SetValueImmediate(DirectX::XMUINT2 value);
	void SetValueImmediate(DirectX::XMUINT3 value);
	void SetValueImmediate(DirectX::XMUINT4 value);
	void SetValueImmediate(float value);
	void SetValueImmediate(DirectX::XMFLOAT2 value);
	void SetValueImmediate(Math::Vector3 value);
	void SetValueImmediate(Math::Vector4 value);
	void SetValueImmediate(const Math::Matrix4& value);

	void CreateRenderThreadData(std::shared_ptr<RenderThread::ComputeData> computeData, const ShaderReflection::Parameter<1>& parameter);

private:
	void UpdateParameterOnRenderThread(const std::array<byte, 64>& data);
	void SubmitToRenderThread();

private:
	const std::string		m_name;

	ShaderVariableType		m_type;
	std::array<byte, 64>	m_data;
	size_t					m_size{ kInvalid };

	// Render thread data
	std::weak_ptr<RenderThread::ComputeData>	m_renderThreadData;
	byte*										m_binding{ nullptr };
};

} // namespace Kodiak