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

namespace Kodiak
{

static Remotery* s_rmt = nullptr;

void InitializeProfiling()
{
	rmt_CreateGlobalInstance(&s_rmt);
}


void ShutdownProfiling()
{
	rmt_DestroyGlobalInstance(s_rmt);
	s_rmt = nullptr;
}

} // namespace Kodiak

#endif