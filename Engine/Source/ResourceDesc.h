// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

namespace Kodiak
{

// Forward declarations
enum class Usage;

struct SampleDesc
{
	uint32_t Count;
	uint32_t Quality;
};


struct Texture2dDesc
{
	uint32_t Width;
	uint32_t Height;
	uint32_t MipLevels;
	uint32_t ArraySize;
	DXGI_FORMAT Format;
	SampleDesc SampleDesc;
	Usage Usage;
	uint32_t BindFlags;
	uint32_t CPUAccessFlags;
	uint32_t MiscFlags;
};


struct ClearValue
{

};


struct ShaderResourceViewDesc
{

};


struct DepthStencilViewDesc
{

};


} // namespace Kodiak