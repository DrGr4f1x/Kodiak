// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from GpuResource.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#pragma once

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL 0ull
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN ~0ull

namespace Kodiak
{

class GpuResource
{
	friend class CommandList;
	friend class GraphicsCommandList;
	friend class ComputeCommandList;

public:
	GpuResource() :
		m_gpuVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL),
		m_usageState(D3D12_RESOURCE_STATE_COMMON),
		m_transitioningState((D3D12_RESOURCE_STATES)-1)
	{}

	GpuResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES currentState) :
		m_resource(resource),
		m_usageState(currentState),
		m_transitioningState((D3D12_RESOURCE_STATES)-1)
	{
		m_gpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
	}

	void Destroy()
	{
		m_resource = nullptr;
	}

	ID3D12Resource* operator->() { return m_resource.Get(); }
	const ID3D12Resource* operator->() const { return m_resource.Get(); }

	ID3D12Resource* GetResource() { return m_resource.Get(); }
	const ID3D12Resource* GetResource() const { return m_resource.Get(); }

	D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return m_gpuVirtualAddress; }

protected:
	Microsoft::WRL::ComPtr<ID3D12Resource>		m_resource;
	D3D12_RESOURCE_STATES						m_usageState;
	D3D12_RESOURCE_STATES						m_transitioningState;
	D3D12_GPU_VIRTUAL_ADDRESS					m_gpuVirtualAddress;
};

} // namespace Kodiak