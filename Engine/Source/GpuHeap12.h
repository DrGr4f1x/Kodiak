#pragma once

namespace Kodiak
{

class GpuHeap
{
public:
	GpuHeap(uint32_t frameCount);

private:
	std::vector<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>>	m_commandLists;
	std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>>		m_commandAllocators;
	uint32_t	m_currentFrame{ 0 };
	HANDLE		m_fenceEvent;
};

} // namespace Kodiak