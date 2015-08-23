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
