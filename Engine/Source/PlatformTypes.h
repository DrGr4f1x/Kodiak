// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#if defined(DX12)

// Raw types
using ShaderResourceView = D3D12_CPU_DESCRIPTOR_HANDLE;
using DepthStencilView = D3D12_CPU_DESCRIPTOR_HANDLE;
using UnorderedAccessView = D3D12_CPU_DESCRIPTOR_HANDLE;

// Ref-counted pointers
using ShaderResourceViewPtr = D3D12_CPU_DESCRIPTOR_HANDLE;
using DepthStencilViewPtr = D3D12_CPU_DESCRIPTOR_HANDLE;
using UnorderedAccessViewPtr = D3D12_CPU_DESCRIPTOR_HANDLE;

// Conversion functions
inline ShaderResourceView GetRawSRV(ShaderResourceViewPtr srv)
{
	return srv;
}

inline DepthStencilView GetRawDSV(DepthStencilViewPtr dsv)
{
	return dsv;
}

inline UnorderedAccessView GetRawUAV(UnorderedAccessViewPtr uav)
{
	return uav;
}

#elif defined(DX11)

// Raw types
struct ShaderResourceView
{
	ID3D11ShaderResourceView* Ptr;
};

struct DepthStencilView
{
	ID3D11DepthStencilView* Ptr;
};

struct UnorderedAccessView
{
	ID3D11UnorderedAccessView* Ptr;
};

// Ref-counted types
using ShaderResourceViewPtr = Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>;
using DepthStencilViewPtr = Microsoft::WRL::ComPtr<ID3D11DepthStencilView>;
using UnorderedAccessViewPtr = Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>;

// Conversion functions
inline ShaderResourceView GetRawSRV(ShaderResourceViewPtr srv)
{
	return ShaderResourceView{ srv.Get() };
}

inline DepthStencilView GetRawDSV(DepthStencilViewPtr dsv)
{
	return DepthStencilView{ dsv.Get() };
}

inline UnorderedAccessView GetRawUAV(UnorderedAccessViewPtr uav)
{
	return UnorderedAccessView{ uav.Get() };
}

#elif defined(VK)
#


#else
#error No graphics API defined!
#endif