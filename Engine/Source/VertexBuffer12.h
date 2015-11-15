// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "GpuResource12.h"

#include <ppltasks.h>

namespace Kodiak
{

// Forward declarations
class BaseVertexBufferData;
enum class Usage;


class VertexBuffer : public GpuResource
{
public:
	VertexBuffer() = default;
	~VertexBuffer() { Destroy(); }

	void Destroy();

	void Create(std::shared_ptr<BaseVertexBufferData> data, Usage usage, const std::string& debugName);

	const D3D12_VERTEX_BUFFER_VIEW& GetVBV() const { return m_vbv; }

public:
	concurrency::task<void> loadTask;

private:
	D3D12_RESOURCE_DESC DescribeBuffer(size_t numElements, size_t elementSize);

	D3D12_VERTEX_BUFFER_VIEW	m_vbv;
	size_t						m_bufferSize;
	size_t						m_elementCount;
	size_t						m_elementSize;
};


} // namespace Kodiak