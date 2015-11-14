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
class GraphicsCommandList;

class IRenderOperation
{
public:
	virtual ~IRenderOperation() = default;

	virtual void PopulateCommandList(GraphicsCommandList& commandList) = 0;
};

} // namespace Kodiak