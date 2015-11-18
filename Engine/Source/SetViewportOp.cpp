// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "SetViewportOp.h"

#include "CommandList.h"

using namespace Kodiak;
using namespace std;


SetViewportOperation::SetViewportOperation(float topLeftX, float topLeftY, float width, float height, float minDepth, float maxDepth)
	: m_viewport(topLeftX, topLeftY, width, height, minDepth, maxDepth)
{}


void SetViewportOperation::PopulateCommandList(GraphicsCommandList& commandList)
{
	commandList.SetViewport(m_viewport);
}