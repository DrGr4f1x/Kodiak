#pragma once

namespace Kodiak
{

class RenderTargetView
{
public:
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;
};

} // namespace Kodiak