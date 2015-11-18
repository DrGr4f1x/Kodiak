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

namespace Kodiak
{

// Forward declarations
class DepthBuffer;

class ClearDepthBufferOperation : public IRenderOperation
{
public:
	ClearDepthBufferOperation(const std::shared_ptr<DepthBuffer>& colorBuffer, float depth);

	void PopulateCommandList(GraphicsCommandList& commandList) override;

private:
	const std::shared_ptr<DepthBuffer>		m_depthBuffer;
	float									m_depth;
};

} // namespace Kodiak