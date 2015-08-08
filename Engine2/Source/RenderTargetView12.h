#pragma once


namespace Kodiak
{

class RenderTargetView
{
public:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_rtv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_rtvHandle;
};

} // namespace Kodiak