#pragma once

#if defined(DX12)
#include "RenderTargetView12.h"
#elif defined(DX11)
#include "RenderTargetView11.h"
#elif defined(VULKAN)
#include "RenderTargetViewVk.h"
#else
#error No graphics API defined!
#endif