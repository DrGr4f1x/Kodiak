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
class BaseIndexBufferData;
enum class Usage;


class IndexBuffer : public GpuResource
{
public:
	IndexBuffer() = default;
	~IndexBuffer() { Destroy(); }

	void Destroy();

	void Create(std::shared_ptr<BaseIndexBufferData> data, Usage usage, const std::string& debugName);

	const D3D12_INDEX_BUFFER_VIEW& GetIBV() const { return m_ibv; }
	
public:
	concurrency::task<void> loadTask;

private:
	D3D12_RESOURCE_DESC DescribeBuffer(size_t numElements, size_t elementSize);

	D3D12_INDEX_BUFFER_VIEW		m_ibv;
	size_t						m_bufferSize;
	size_t						m_elementCount;
	size_t						m_elementSize;
};


} // namespace Kodiak