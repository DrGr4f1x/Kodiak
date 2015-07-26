#pragma once

namespace Kodiak
{

// Forward declarations
class DeviceResources;

class RenderingEngine
{
public:
    RenderingEngine(const std::shared_ptr<DeviceResources>& deviceResources);

private:
    // Cached pointer to device resources.
    std::shared_ptr<DeviceResources> m_deviceResources;
};

} // namespace Kodiak