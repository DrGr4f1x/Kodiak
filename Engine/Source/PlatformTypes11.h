// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once


template <typename T>
class PlatformCpuHandle
{
public:
	using Type = T*;

	PlatformResource() : m_handle(nullptr) {}
	PlatformResource(Type handle) : m_handle(handle) {}
	PlatformResource(const PlatformResource& other) : m_handle(other.m_handle) {}
	PlatformResource(PlatformResource&& other) : m_handle(std::move(other.m_handle)) {}

	PlatformCpuHandle& operator=(decltype(__nullptr)) throw()
	{
		m_handle.Reset();
		return *this;
	}

	PlatformCpuHandle& operator=(Type other) throw()
	{
		if (ptr_ != other)
		{
			Microsoft::WRL::ComPtr<T>(other).Swap(m_handle);
		}
		return *this;
	}

	template <typename U>
	PlatformCpuHandle& operator=(U* other) throw()
	{
		Microsoft::WRL::ComPtr(other).Swap(m_handle);
		return *this;
	}

	PlatformCpuHandle& operator=(const ComPtr& other) throw()
	{
		if (ptr_ != other.ptr_)
		{
			ComPtr(other).Swap(*this);
		}
		return *this;
	}

	template<class U>
	ComPtr& operator=(const ComPtr<U>& other) throw()
	{
		ComPtr(other).Swap(*this);
		return *this;
	}

	ComPtr& operator=(_Inout_ ComPtr &&other) throw()
	{
		ComPtr(static_cast<ComPtr&&>(other)).Swap(*this);
		return *this;
	}

	template<class U>
	ComPtr& operator=(_Inout_ ComPtr<U>&& other) throw()
	{
		ComPtr(static_cast<ComPtr<U>&&>(other)).Swap(*this);
		return *this;
	}

	Type Get()
	{
		return m_handle.Get();
	}

	operator bool() const
	{
		return Get() != nullptr;
	}

private:
	Microsoft::WRL::ComPtr<T> m_handle;
};