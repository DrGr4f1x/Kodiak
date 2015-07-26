#include "pch.h"

#include "Application.h"

#include "RenderingEngine.h"


namespace Kodiak
{

Application::Application(const std::shared_ptr<DeviceResources>& deviceResources)
    : m_renderingEngine(std::make_unique<RenderingEngine>(deviceResources))
{}


Application::~Application()
{}


void Application::Run()
{}

} // namespace Kodiak