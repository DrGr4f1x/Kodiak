// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "Filesystem.h"

namespace Kodiak
{


template <class TResource>
class ResourceLoader
{
public:
	static ResourceLoader& GetInstance()
	{
		static ResourceLoader instance;
		return instance;
	}

	bool IsAsyncLoadEnabled() const { return m_asyncLoadEnabled; }
	void SetAsyncLoadEnabled(bool enable) { m_asyncLoadEnabled = enable; }

	template<class... U>
	std::shared_ptr<TResource> Load(const std::string& path, U&&... u)
	{
		// See if we already have a resource with the given path
		auto resource = FindObject(path);
		if (resource)
		{
			return resource;
		}

		// Verify that the path to the resource is valid
		auto& filesystem = Filesystem::GetInstance();
		if (!filesystem.Exists(path))
		{
			return nullptr;
		}

		// Didn't find one, so queue up an async load (or load immediately)
		resource = make_shared<TResource>(std::forward<U>(u)...);
		resource->SetResourcePath(path);

		if (m_asyncLoadEnabled)
		{
			Queue(path, resource);
		}
		else
		{
			resource->DoLoad();
		}
		return resource;
	}


	void Update()
	{
		auto it = std::begin(m_pendingQueue);
		while (it != std::end(m_pendingQueue))
		{
			auto weak = it->second;
			if (!weak.expired())
			{
				auto resource = weak.lock();
				if (resource)
				{
					if (resource->IsLoadFinished())
					{
						if (resource->IsReady())
						{
							resource->ExecutePostLoadCallbacks();
						}
						m_readyQueue[it->first] = resource;
						it = m_pendingQueue.erase(it);
					}
				}
				else
				{
					++it;
				}
			}
			else
			{
				it = m_pendingQueue.erase(it);
			}
		}

		it = std::begin(m_readyQueue);
		while (it != std::end(m_readyQueue))
		{
			if (it->second.expired())
			{
				it = m_readyQueue.erase(it);
			}
			else
			{
				++it;
			}
		}
	}


private:
	void Queue(const std::string& path, std::shared_ptr<TResource> resource)
	{
		weak_ptr<TResource> weak = resource;
		
		// Add to pending work queue
		m_pendingQueue[path] = weak;

		// Launch async background task
		std::async(std::launch::async, [resource]() { resource->DoLoad(); });
	}


	std::shared_ptr<TResource> FindObject(const std::string& path)
	{
		// Check ready queue first
		{
			auto res = m_readyQueue.find(path);
			if (res != end(m_readyQueue))
			{
				return res->second.lock();
			}
		}

		// Check pending queue next
		{
			auto res = m_pendingQueue.find(path);
			if (res != end(m_pendingQueue))
			{
				return res->second.lock();
			}
		}

		return nullptr;
	}


private:
	bool m_asyncLoadEnabled{ true };
	std::unordered_map<std::string, std::weak_ptr<TResource>> m_pendingQueue;
	std::unordered_map<std::string, std::weak_ptr<TResource>> m_readyQueue;
};

} // namespace Kodiak