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

class CommandListManager
{
	friend class CommandList;

public:
	static CommandListManager& GetInstance()
	{
		static CommandListManager instance;
		return instance;
	}

	void Create(ID3D11Device* device);

private:
	void CreateNewDeferredContext(ID3D11DeviceContext** context);
	uint64_t ExecuteCommandList(ID3D11CommandList* commandList);

private:
	ID3D11Device* m_device{ nullptr };
};

} // namespace Kodiak
