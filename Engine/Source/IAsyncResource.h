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
enum class LoadState;


class IAsyncResource
{
public:
	IAsyncResource();
	virtual ~IAsyncResource() = default;

	// Implement this in subclasses
	virtual bool DoLoad() = 0;

	void AcquireThreadResult(std::future<void>&& threadResult);
	void Wait();

	void SetResourcePath(const std::string& path) { m_resourcePath = path; }
	const std::string& GetResourcePath() const { return m_resourcePath; }

	bool IsReady() const;
	bool IsLoadFinished() const;
	LoadState GetLoadState() const { return m_loadState; }

	void AddPostLoadCallback(std::function<void()> callback);
	void ExecutePostLoadCallbacks();

protected:
	std::string							m_resourcePath;
	std::future<void>					m_threadResult;
	std::atomic<LoadState>				m_loadState;
	std::vector<std::function<void()>>	m_callbacks;
};

} // namespace Kodiak