// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from DepthBuffer.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#pragma once

#include "PixelBuffer12.h"

namespace Kodiak
{

// Forward declarations
class DeviceManager;
enum class DepthFormat;

class DepthBuffer : public PixelBuffer
{
public:
	DepthBuffer(float clearDepth = 0.0f, uint32_t clearStencil = 0)
		: m_clearDepth(clearDepth)
		, m_clearStencil(clearStencil)
	{
		m_dsv.ptr = ~0ull;
		m_dsvReadOnly.ptr = ~0ull;
		m_dsvReadOnlyDepth.ptr = ~0ull;
		m_dsvReadOnlyStencil.ptr = ~0ull;
		m_depthSRV.ptr = ~0ull;
		m_stencilSRV.ptr = ~0ull;
	}

	// Create a depth buffer.  If an address is supplied, memory will not be allocated.
	// The vmem address allows you to alias buffers (which can be especially useful for
	// reusing ESRAM across a frame.)
	void Create(const std::string& name, size_t width, size_t height, DepthFormat format);

	// Get pre-created CPU-visible descriptor handles
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSV() const					{ return m_dsv; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSVReadOnly() const			{ return m_dsvReadOnly; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSVReadOnlyDepth() const		{ return m_dsvReadOnlyDepth; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSVReadOnlyStencil() const	{ return m_dsvReadOnlyStencil; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDepthSRV() const				{ return m_depthSRV; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetStencilSRV() const			{ return m_stencilSRV; }

	float GetClearDepth() const { return m_clearDepth; }
	uint32_t GetClearStencil() const { return m_clearStencil; }

private:
	void CreateDerivedViews();

private:
	float		m_clearDepth{ 0.0f };
	uint32_t	m_clearStencil{ 0 };
	D3D12_CPU_DESCRIPTOR_HANDLE m_dsv;
	D3D12_CPU_DESCRIPTOR_HANDLE m_dsvReadOnly;
	D3D12_CPU_DESCRIPTOR_HANDLE m_dsvReadOnlyDepth;
	D3D12_CPU_DESCRIPTOR_HANDLE m_dsvReadOnlyStencil;
	D3D12_CPU_DESCRIPTOR_HANDLE m_depthSRV;
	D3D12_CPU_DESCRIPTOR_HANDLE m_stencilSRV;
};

} // namespace Kodiak