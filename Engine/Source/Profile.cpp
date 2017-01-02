// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "Profile.h"

#if defined(PROFILING) && (PROFILING == 1)

__itt_domain* domain = nullptr;
__itt_string_handle* itt_update = nullptr;
__itt_string_handle* itt_draw_mesh = nullptr;
__itt_string_handle* itt_draw_model = nullptr;
__itt_string_handle* itt_scene_update = nullptr;
__itt_string_handle* itt_present = nullptr;

namespace Kodiak
{

void InitializeProfiling()
{
	domain = __itt_domain_create(L"GPA_DOMAIN");
	itt_update = __itt_string_handle_create(L"Update");
	itt_draw_mesh = __itt_string_handle_create(L"Draw mesh");
	itt_draw_model = __itt_string_handle_create(L"Draw model");
	itt_scene_update = __itt_string_handle_create(L"Scene update");
	itt_present = __itt_string_handle_create(L"Present");
}


void ShutdownProfiling()
{
	domain = nullptr;
}

} // namespace Kodiak

#endif