#pragma once

namespace Kodiak
{

// Forward declarations
class CommandList;

class IRenderOperation
{
public:
	virtual ~IRenderOperation() = default;

	virtual void PopulateCommandList(const std::shared_ptr<CommandList>& commandList) = 0;
};

} // namespace Kodiak