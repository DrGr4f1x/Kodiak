// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#if defined(DX11)
#include "MaterialResource11.h"
#elif defined(DX12)
#include "MaterialResource12.h"
#elif defined(VK)
#include "MaterialResourceVk.h"
#else
#error No graphics API defined!
#endif