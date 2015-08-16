#pragma once


namespace Kodiak
{

class RenderTargetView
{
public:
	RenderTargetView();

public:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_rtv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_rtvHandle;
	D3D12_RESOURCE_STATES m_currentResourceState;
};

} // namespace Kodiak