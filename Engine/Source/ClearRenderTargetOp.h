#pragma once

#include "IRenderOperation.h"

namespace Kodiak
{

// Forward declarations
class RenderTargetView;

class ClearRenderTargetOperation : public IRenderOperation
{
public:
	ClearRenderTargetOperation(const std::shared_ptr<RenderTargetView>& rtv, const DirectX::XMVECTORF32& color);

	void PopulateCommandList(const std::shared_ptr<CommandList>& commandList) override;

private:
	const std::shared_ptr<RenderTargetView>		m_rtv;
	const DirectX::XMVECTORF32					m_color;
};

} // namespace Kodiak
