// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "RenderThread.h"

namespace Kodiak
{

thread_local ThreadRole tl_threadRole = ThreadRole::Unknown;


ThreadRole GetThreadRole()
{
	return tl_threadRole;
}


void SetThreadRole(ThreadRole role)
{
	tl_threadRole = role;
}


bool IsRenderThread()
{
	return tl_threadRole == ThreadRole::RenderMain || tl_threadRole == ThreadRole::RenderWorker;
}

} // namespace Kodiak