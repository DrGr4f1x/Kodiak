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
namespace RenderThread { struct ComputeData; }


class ComputeConstantBuffer
{
public:
	ComputeConstantBuffer(const std::string& name);

	const std::string& GetName() const { return m_name; }

	void SetDataImmediate(size_t sizeInBytes, const byte* data);

	void CreateRenderThreadData(std::shared_ptr<RenderThread::ComputeData> computeData, const ShaderReflection::CBVLayout& cbv);

private:
	const std::string	m_name;

	size_t				m_size{ kInvalid };

	// Render thread data
	std::weak_ptr<RenderThread::ComputeData>	m_renderThreadData;
	byte*										m_binding{ nullptr };
};

} // namespace Kodiak