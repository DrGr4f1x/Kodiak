// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//


#include "Stdafx.h"

#include "SetScissorOp.h"

#include "CommandList.h"

using namespace Kodiak;
using namespace std;


SetScissorOperation::SetScissorOperation(uint32_t topLeftX, uint32_t topLeftY, uint32_t width, uint32_t height)
	: m_rect(topLeftX, topLeftY, topLeftX + width, topLeftY + height)
{}


void SetScissorOperation::PopulateCommandList(GraphicsCommandList& commandList)
{
	commandList.SetScissor(m_rect);
}