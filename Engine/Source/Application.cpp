#include "pch.h"

#include "Application.h"

#include "RenderingEngine.h"

using namespace std;

namespace Kodiak
{

Application::Application(const std::shared_ptr<DeviceResources>& deviceResources)
    : m_renderingEngine(std::make_unique<RenderingEngine>(deviceResources))
{}


Application::~Application()
{
    Shutdown();
}


void Application::Initialize()
{}


void Application::Run()
{}


void Application::Shutdown()
{}


} // namespace Kodiak