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
	ID3D11Buffer* GetBuffer() { return m_buffer.Get(); }
		
	void Create(size_t size, Usage usage);

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;
};

} // namespace Kodiak