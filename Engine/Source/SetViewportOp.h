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

#include "Viewport.h"

namespace Kodiak
{

class SetViewportOperation : public IRenderOperation
{
public:
	SetViewportOperation(float topLeftX, float topLeftY, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f);

	void PopulateCommandList(GraphicsCommandList& commandList) override;

private:
	Viewport m_viewport;
};

} // namespace Kodiak