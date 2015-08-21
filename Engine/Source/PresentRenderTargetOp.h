#pragma once

#include "IRenderOperation.h"

namespace Kodiak
{

// Forward declarations
class CommandList;
class RenderTargetView;

class PresentRenderTargetOperation : public IRenderOperation
{
public:
	PresentRenderTargetOperation(const std::shared_ptr<RenderTargetView>& rtv);

	void PopulateCommandList(const std::shared_ptr<CommandList>& commandList) override;

private:
	std::shared_ptr<RenderTargetView> m_rtv;
};

} // namespace Kodiak
