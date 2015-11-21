// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "ClearColorBufferOp.h"

#include "ColorBuffer.h"
#include "CommandList.h"


using namespace Kodiak;
using namespace DirectX;
using namespace std;


ClearColorBufferOperation::ClearColorBufferOperation(const shared_ptr<ColorBuffer>& colorBuffer, const XMVECTORF32& color)
	: m_colorBuffer(colorBuffer)
	, m_color(color)
{}


void ClearColorBufferOperation::PopulateCommandList(GraphicsCommandList& commandList)
{
	commandList.ClearColor(*m_colorBuffer, m_color);
}