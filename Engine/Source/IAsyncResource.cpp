// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "IAsyncResource.h"

#include "LoaderEnums.h"

using namespace Kodiak;
using namespace std;


IAsyncResource::IAsyncResource()
	: m_loadState(LoadState::LoadNotStarted)
{}


void IAsyncResource::AcquireThreadResult(future<void>&& threadResult)
{
	m_threadResult = move(threadResult);
}


void IAsyncResource::Wait()
{
	if (m_threadResult.valid())
	{
		m_threadResult.get();
	}
}


bool IAsyncResource::IsReady() const
{
	return m_loadState == LoadState::LoadSucceeded;
}


bool IAsyncResource::IsLoadFinished() const
{
	LoadState curState = m_loadState;
	return (curState == LoadState::LoadFailed) || (curState == LoadState::LoadSucceeded);
}


void IAsyncResource::AddPostLoadCallback(function<void()> callback)
{
	LoadState curState = m_loadState;
	if (curState == LoadState::LoadSucceeded)
	{
		callback();
	}
	else if (curState != LoadState::LoadFailed)
	{
		m_callbacks.push_back(callback);
	}
}


void IAsyncResource::ExecutePostLoadCallbacks()
{
	for (auto& callback : m_callbacks)
	{
		callback();
	}
	m_callbacks.clear();
}