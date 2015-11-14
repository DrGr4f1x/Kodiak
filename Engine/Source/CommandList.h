#pragma once

#if defined(DX12)
#include "CommandList12.h"
#elif defined(DX11)
#include "CommandList11.h"
#elif defined(VK)
#include "CommandListVk.h"
#else
#error No graphics API defined!
#endif