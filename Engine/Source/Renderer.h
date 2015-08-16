#pragma once

#if defined(DX12)
#include "Renderer12.h"
#elif defined(DX11)
#include "Renderer11.h"
#elif defined(VULKAN)
#include "RendererVk.h"
#else
#error No graphics API defined!
#endif