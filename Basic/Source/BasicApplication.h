#pragma once

#include "Engine\Source\Application.h"
#include "Engine\Source\Renderer.h"

namespace Kodiak
{

// Forward declarations
class CommandList;
class Model;

class BasicApplication : public Application
{
public:
	BasicApplication(uint32_t width, uint32_t height, const std::wstring& name);

protected:
	void OnInit() override;
	void OnUpdate() override;
	void OnDestroy() override;
	bool OnEvent(MSG msg) override;

private:
	std::shared_ptr<Model> m_boxModel;
};

} // namespace Kodiak