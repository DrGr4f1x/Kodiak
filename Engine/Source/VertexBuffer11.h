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
class BaseVertexBufferData;
enum class Usage;

class VertexBuffer
{
public:
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	uint32_t stride{ 0 };

	static std::shared_ptr<VertexBuffer> Create(std::shared_ptr<BaseVertexBufferData> data, Usage usage);

private:
	static void CreateInternal(std::shared_ptr<VertexBuffer> buffer, std::shared_ptr<BaseVertexBufferData> data, Usage usage);
};

} // namespace Kodiak