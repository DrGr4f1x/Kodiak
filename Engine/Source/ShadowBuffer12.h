// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from ShadowBuffer.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#pragma once

#include "DepthBuffer.h"
#include "Rectangle.h"
#include "Viewport.h"

namespace Kodiak
{

class GraphicsCommandList;

class ShadowBuffer : public DepthBuffer
{
public:
	ShadowBuffer() {}

	void Create(const std::string& name, uint32_t width, uint32_t height);
	
	ShaderResourceView GetSRV() { return GetDepthSRV(); }

	void BeginRendering(GraphicsCommandList& context);
	void EndRendering(GraphicsCommandList& context);

private:
	Viewport m_viewport;
	Rectangle m_scissor;
};

} // namespace Kodiak