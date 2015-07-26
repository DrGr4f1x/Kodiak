#pragma once

#include "Engine\Source\Application.h"

namespace Kodiak
{
    class DeviceResources;
} // namespace Kodiak

class BasicApplication : public Kodiak::Application
{
public:
    BasicApplication(const std::shared_ptr<Kodiak::DeviceResources>& deviceResources);
    ~BasicApplication();

    void Initialize();
    
protected:
    void Update() override;

private:
    void InitializeScene();
    void InitializeRenderer();
};