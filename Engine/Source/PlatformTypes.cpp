// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#if defined(DX12)

const ShaderResourceView	g_nullSRV = ShaderResourceView{ ~0ull };
const DepthStencilView		g_nullDSV = DepthStencilView{ ~0ull };
const UnorderedAccessView	g_nullUAV = UnorderedAccessView{ ~0ull };

#elif defined(DX11)

const ShaderResourceView	g_nullSRV = nullptr;
const DepthStencilView		g_nullDSV = nullptr;
const UnorderedAccessView	g_nullUAV = nullptr;

#elif defined(VK)

#else
#error  No graphics API defined!
#endif