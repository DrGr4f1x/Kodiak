// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

namespace Kodiak
{

// Forward declarations
struct RenderTaskEnvironment;

class IAsyncRenderTask
{
public:
	virtual ~IAsyncRenderTask() {}

	virtual void Execute(RenderTaskEnvironment& environment) = 0;
};

} // namespace Kodiak
