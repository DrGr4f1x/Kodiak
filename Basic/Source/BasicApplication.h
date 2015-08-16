#pragma once

#include "Engine\Source\Application.h"
#include "Engine\Source\Renderer.h"

namespace Kodiak
{
class CommandList;

class BasicApplication : public Application
{
public:
	BasicApplication(uint32_t width, uint32_t height, const std::wstring& name);

protected:
	void OnInit() override;
	void OnUpdate() override;
	void OnRender() override;
	void OnDestroy() override;
	bool OnEvent(MSG msg) override;

private:
	void PopulateCommandList();

private:
	std::shared_ptr<CommandList> m_commandList;
};

} // namespace Kodiak