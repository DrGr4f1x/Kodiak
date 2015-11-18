// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include <ppltasks.h>

namespace Kodiak
{ 

// Forward declarations
class BaseIndexBufferData;
enum class Usage;

class IndexBuffer
{
public:
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	DXGI_FORMAT format;
	
	concurrency::task<void> loadTask;

	void Create(std::shared_ptr<BaseIndexBufferData> data, Usage usage, const std::string& debugName);
};


} // namespace Kodiak