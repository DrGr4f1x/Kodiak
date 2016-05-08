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

	void SetValue(bool value, int32_t index = 0);
	void SetValue(int32_t value, int32_t index = 0);
	void SetValue(DirectX::XMINT2 value, int32_t index = 0);
	void SetValue(DirectX::XMINT3 value, int32_t index = 0);
	void SetValue(DirectX::XMINT4 value, int32_t index = 0);
	void SetValue(uint32_t value, int32_t index = 0);
	void SetValue(DirectX::XMUINT2 value, int32_t index = 0);
	void SetValue(DirectX::XMUINT3 value, int32_t index = 0);
	void SetValue(DirectX::XMUINT4 value, int32_t index = 0);
	void SetValue(float value, int32_t index = 0);
	void SetValue(DirectX::XMFLOAT2 value, int32_t index = 0);
	void SetValue(Math::Vector3 value, int32_t index = 0);
	void SetValue(Math::Vector4 value, int32_t index = 0);
	void SetValue(const Math::Matrix4& value, int32_t index = 0);

	void SetValueImmediate(bool value, int32_t index = 0);
	void SetValueImmediate(int32_t value, int32_t index = 0);
	void SetValueImmediate(DirectX::XMINT2 value, int32_t index = 0);
	void SetValueImmediate(DirectX::XMINT3 value, int32_t index = 0);
	void SetValueImmediate(DirectX::XMINT4 value, int32_t index = 0);
	void SetValueImmediate(uint32_t value, int32_t index = 0);
	void SetValueImmediate(DirectX::XMUINT2 value, int32_t index = 0);
	void SetValueImmediate(DirectX::XMUINT3 value, int32_t index = 0);
	void SetValueImmediate(DirectX::XMUINT4 value, int32_t index = 0);
	void SetValueImmediate(float value, int32_t index = 0);
	void SetValueImmediate(DirectX::XMFLOAT2 value, int32_t index = 0);
	void SetValueImmediate(Math::Vector3 value, int32_t index = 0);
	void SetValueImmediate(Math::Vector4 value, int32_t index = 0);
	void SetValueImmediate(const Math::Matrix4& value, int32_t index = 0);

	void CreateRenderThreadData(std::shared_ptr<RenderThread::ComputeData> computeData, const ShaderReflection::Parameter<1>& parameter);

private:
	void UpdateParameterOnRenderThread(const std::array<byte, 64>& data, int32_t index);
	void SubmitToRenderThread(std::array<byte, 64> data, int32_t index);
	void FlushPendingUpdates();

private:
	const std::string		m_name;

	ShaderVariableType		m_type;
	size_t					m_size{ kInvalid };
	uint32_t				m_numElements;

	// Render thread data
	std::weak_ptr<RenderThread::ComputeData>	m_renderThreadData;
	byte*										m_binding{ nullptr };

	// Pending updates
	std::vector<std::pair<std::array<byte, 64>, int32_t>> m_pendingUpdates;
};

} // namespace Kodiak