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

class GpuResource
{
public:
	void Destroy()
	{
		m_resource = nullptr;
	}

	ID3D11Resource* operator->() { return m_resource.Get(); }
	const ID3D11Resource* operator->() const { return m_resource.Get(); }

	ID3D11Resource* GetResource() { return m_resource.Get(); }
	const ID3D11Resource* GetResource() const { return m_resource.Get(); }

protected:
	Microsoft::WRL::ComPtr<ID3D11Resource> m_resource{ nullptr };
};

} // namespace Kodiak