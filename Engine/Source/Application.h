#pragma once

namespace Kodiak
{

// Forward declarations
class DeviceResources;
class RenderingEngine;

ENGINE_API_TEMPLATE template class ENGINE_API std::unique_ptr<class RenderingEngine>;

class ENGINE_API Application
{
public:
    Application(const std::shared_ptr<DeviceResources>& deviceResources);
    virtual ~Application();

    virtual void Initialize();
    void Run();

protected:
    virtual void Shutdown();
    virtual void Update() = 0;

protected:
    // Pointer to the rendering engine
    std::unique_ptr<class RenderingEngine> m_renderingEngine;
};

} // namespace Kodiak