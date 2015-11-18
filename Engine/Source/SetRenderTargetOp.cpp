// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "SetRenderTargetOp.h"

#include "ColorBuffer.h"
#include "DepthBuffer.h"
#include "CommandList.h"


using namespace Kodiak;
using namespace std;


SetRenderTargetOperation::SetRenderTargetOperation(shared_ptr<ColorBuffer> colorBuffer, shared_ptr<DepthBuffer> depthBuffer)
	: m_colorBuffer(colorBuffer)
	, m_depthBuffer(depthBuffer)
{}


void SetRenderTargetOperation::PopulateCommandList(GraphicsCommandList& commandList)
{
	commandList.SetRenderTarget(*m_colorBuffer, *m_depthBuffer);
}