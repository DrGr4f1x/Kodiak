// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#if defined(PROFILING) && (PROFILING == 1)

#define RMT_ENABLED 1
#include "Remotery\lib\Remotery.h"

#define PROFILE(str) rmt_ScopedCPUSample(str)

namespace Kodiak
{
void InitializeProfiling();
void ShutdownProfiling();
} // namespace Kodiak

#else

#define PROFILE(str) __noop

namespace Kodiak
{
inline void InitializeProfiling() {}
inline void ShutdownProfiling() {}
}

#endif