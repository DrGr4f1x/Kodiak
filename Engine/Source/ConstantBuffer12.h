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

class ConstantBuffer
{
public:
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
	size_t size;

	void Create(size_t size, Usage usage);
};

} // namespace Kodiak