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
enum class PrimitiveTopology;

struct DrawIndexedParameters
{
	PrimitiveTopology		topology;
	uint32_t				indexCount;
	uint32_t				startIndex;
	int32_t					baseVertexOffset;
};

} // namespace Kodiak