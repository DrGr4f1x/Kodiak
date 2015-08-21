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


namespace Kodiak
{

// Forward declarations
class DeviceResources;

class CommandListManager
{
public:
	CommandListManager(DeviceResources* deviceResources);

	std::shared_ptr<CommandList> CreateCommandList();
	void UpdateCommandLists();

private:
	DeviceResources*						m_deviceResources{ nullptr };
	std::list<std::weak_ptr<CommandList>>	m_commandLists;
};

} // namespace Kodiak