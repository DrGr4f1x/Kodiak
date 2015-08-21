#include "Stdafx.h"

#include "CommandList.h"

#include "DeviceResources.h"

using namespace Kodiak;
using namespace std;


CommandListManager::CommandListManager(DeviceResources* deviceResources)
	: m_deviceResources(deviceResources)
	, m_commandLists()
{}


shared_ptr<CommandList> CommandListManager::CreateCommandList()
{
	auto commandList = m_deviceResources->CreateCommandList();

	weak_ptr<CommandList> weak(commandList);
	m_commandLists.push_back(weak);

	return commandList;
}


void CommandListManager::UpdateCommandLists()
{
	auto currentFrame = m_deviceResources->GetCurrentFrame();

	auto it = m_commandLists.begin();
	auto end = m_commandLists.end();

	while (it != end)
	{
		auto locked = it->lock();
		if (locked)
		{
			locked->SetCurrentFrame(currentFrame);
			++it;
		}
		else
		{
			it = m_commandLists.erase(it);
		}
	}
}