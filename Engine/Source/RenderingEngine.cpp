#include "pch.h"

#include "RenderingEngine.h"

#include "DeviceResources.h"


namespace Kodiak
{

RenderingEngine::RenderingEngine(const std::shared_ptr<DeviceResources>& deviceResources)
    : m_deviceResources(deviceResources)
{}

} // namespace Kodiak