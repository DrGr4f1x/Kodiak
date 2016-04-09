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

template <class T>
class ThreadParameter
{
public:
	ThreadParameter(T& target);

	operator const T&() const { return m_target; }
	void operator=(T& other);

	std::function<void()> postAssignment;

private:
	T& m_target;
};


template <class T>
ThreadParameter<T>::ThreadParameter(T& target)
	: m_target(target)
{
	postAssignment = [] {};
}


template <class T>
void ThreadParameter<T>::operator=(T& other)
{
	Renderer::GetInstance().EnqueueTask([this, &other](RenderTaskEnvironment& rte)
	{
		m_target = other;
		postAssignment();
	});
}


} // namespace Kodiak