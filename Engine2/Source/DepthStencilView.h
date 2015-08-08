#pragma once

#if defined(DX12)
#include "DepthStencilView12.h"
#elif defined(DX11)
#include "DepthStencilView11.h"
#elif defined(VK)
#include "DepthStencilViewVk.h"
#else
#error No graphics API defined!
#endif