// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#if defined(DX12)
#include "DeviceManager12.h"
#elif defined(DX11)
#include "DeviceManager11.h"
#elif defined(VK)
#include "DeviceManagerVk.h"
#else
#error No graphics API defined!
#endif