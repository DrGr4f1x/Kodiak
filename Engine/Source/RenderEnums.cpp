// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "RenderEnums.h"

namespace Kodiak
{

bool IsSRVType(ShaderResourceType type)
{
	return type == ShaderResourceType::Texture ||
		type == ShaderResourceType::TBuffer ||
		type == ShaderResourceType::Structured ||
		type == ShaderResourceType::ByteAddress;
}


bool IsUAVType(ShaderResourceType type)
{
	return type == ShaderResourceType::UAVAppendStructured ||
		type == ShaderResourceType::UAVConsumeStructured ||
		type == ShaderResourceType::UAVRWByteAddress ||
		type == ShaderResourceType::UAVRWStructured ||
		type == ShaderResourceType::UAVRWStructuredWithCounter ||
		type == ShaderResourceType::UAVRWTyped;
}

} // namespace Kodiak