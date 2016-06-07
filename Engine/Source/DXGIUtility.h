// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "Format.h"

namespace DXGIUtility
{

DXGI_FORMAT ConvertToDXGI(Kodiak::ColorFormat format);
DXGI_FORMAT ConvertToDXGI(Kodiak::DepthFormat format);

DXGI_FORMAT GetBaseFormat(DXGI_FORMAT defaultFormat);
DXGI_FORMAT GetUAVFormat(DXGI_FORMAT defaultFormat);
DXGI_FORMAT GetDSVFormat(DXGI_FORMAT defaultFormat);
DXGI_FORMAT GetDepthFormat(DXGI_FORMAT defaultFormat);
DXGI_FORMAT GetStencilFormat(DXGI_FORMAT defaultFormat);

size_t BitsPerPixel(DXGI_FORMAT fmt);
size_t BytesPerPixel(DXGI_FORMAT fmt);

} // namespace DXGIUtility