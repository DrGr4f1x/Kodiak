// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "IRenderOperation.h"

#include "Rectangle.h"

namespace Kodiak
{

class SetScissorOperation : public IRenderOperation
{
public:
	SetScissorOperation(uint32_t topLeftX, uint32_t topLeftY, uint32_t width, uint32_t height);

	void PopulateCommandList(GraphicsCommandList& commandList) override;

private:
	Kodiak::Rectangle m_rect;
};

} // namespace Kodiak