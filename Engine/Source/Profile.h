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

#include "IntelITT\include\ittnotify.h"

extern __itt_domain* domain;
extern __itt_string_handle* itt_update;
extern __itt_string_handle* itt_draw_mesh;
extern __itt_string_handle* itt_draw_model;
extern __itt_string_handle* itt_scene_update;
extern __itt_string_handle* itt_present;

#define PROFILE_BEGIN(name) __itt_task_begin(domain, __itt_null, __itt_null, name)
#define PROFILE_END() __itt_task_end(domain)

namespace Kodiak
{
void InitializeProfiling();
void ShutdownProfiling();
} // namespace Kodiak

#else

#define PROFILE_BEGIN(name) __noop
#define PROFILE_END() __noop

namespace Kodiak
{
inline void InitializeProfiling() {}
inline void ShutdownProfiling() {}
}

#endif