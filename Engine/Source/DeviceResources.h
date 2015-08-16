#pragma once

#if defined(DX12)
#include "DeviceResources12.h"
#elif defined(DX11)
#include "DeviceResources11.h"
#elif defined(VULKAN)
#include "DeviceResourcesVk.h"
#else
#error No graphics API defined!
#endif