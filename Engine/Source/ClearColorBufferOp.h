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
class ColorBuffer;

class ClearColorBufferOperation : public IRenderOperation
{
public:
	ClearColorBufferOperation(const std::shared_ptr<ColorBuffer>& colorBuffer, const DirectX::XMVECTORF32& color);

	void PopulateCommandList(GraphicsCommandList& commandList) override;

private:
	const std::shared_ptr<ColorBuffer>		m_colorBuffer;
	const DirectX::XMVECTORF32				m_color;
};

} // namespace Kodiak
