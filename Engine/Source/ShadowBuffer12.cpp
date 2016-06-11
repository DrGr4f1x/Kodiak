// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from ShadowBuffer.cpp in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#include "Stdafx.h"

#include "ShadowBuffer12.h"

#include "CommandList12.h"
#include "Format.h"
#include "RenderEnums.h"


using namespace Kodiak;
using namespace std;


void ShadowBuffer::Create(const std::string& name, uint32_t width, uint32_t height)
{
	DepthBuffer::Create(name, width, height, DepthFormat::D16);

	m_viewport.topLeftX = 0.0f;
	m_viewport.topLeftY = 0.0f;
	m_viewport.width = (float)width;
	m_viewport.height = (float)height;
	m_viewport.minDepth = 0.0f;
	m_viewport.maxDepth = 1.0f;

	// Prevent drawing to the boundary pixels so that we don't have to worry about shadows stretching
	m_scissor.left = 1;
	m_scissor.top = 1;
	m_scissor.right = (LONG)width - 2;
	m_scissor.bottom = (LONG)height - 2;
}


void ShadowBuffer::BeginRendering(GraphicsCommandList& commandList)
{
	commandList.TransitionResource(*this, ResourceState::DepthWrite, true);
	commandList.ClearDepth(*this);
	commandList.SetDepthStencilTarget(*this);
	commandList.SetViewportAndScissor(m_viewport, m_scissor);
}


void ShadowBuffer::EndRendering(GraphicsCommandList& commandList)
{
	commandList.TransitionResource(*this, ResourceState::PixelShaderResource);
}