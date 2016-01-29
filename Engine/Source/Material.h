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

// Make sure the return values of these functions match the shaders!
uint32_t GetPerViewConstantsSlot();
uint32_t GetPerObjectConstantsSlot();
uint32_t GetPerMaterialConstantsSlot();
const std::string& GetPerViewConstantsName();
const std::string& GetPerObjectConstantsName();
const std::string& GetPerMaterialConstantsName();

} // namespace Kodiak


#if defined(DX12)
#include "Material12.h"
#elif defined(DX11)
#include "Material11.h"
#elif defined(VK)
#include "MaterialVk.h"
#else
#error No graphics API defined!
#endif