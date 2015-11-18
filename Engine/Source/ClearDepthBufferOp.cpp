// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "ClearDepthBufferOp.h"

#include "DepthBuffer.h"
#include "CommandList.h"


using namespace Kodiak;
using namespace DirectX;
using namespace std;


ClearDepthBufferOperation::ClearDepthBufferOperation(const shared_ptr<DepthBuffer>& depthBuffer, float depth)
	: m_depthBuffer(depthBuffer)
	, m_depth(depth)
{}


void ClearDepthBufferOperation::PopulateCommandList(GraphicsCommandList& commandList)
{
	commandList.ClearDepth(*m_depthBuffer, m_depth);
}