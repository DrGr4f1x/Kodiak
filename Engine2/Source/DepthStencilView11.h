#pragma once

namespace Kodiak
{

class DepthStencilView
{
public:
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsv;
};

} // namespace Kodiak