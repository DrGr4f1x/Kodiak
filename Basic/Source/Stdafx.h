#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

// Windows
#include <windows.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <ppl.h>
#include <ppltasks.h>

// DirectX common
#if defined(DX11) || defined(DX12)
#include <D3Dcompiler.h>
#include <dxgi1_4.h>
#include <d2d1_3.h>
#include <dwrite_2.h>
#include <wincodec.h>
#endif

#if defined(DX12)
#include <d3d12.h>
#include <d3d11on12.h>
#include "Engine\Source\d3dx12.h"
#define GRAPHICS_API L"DX12"
#elif defined(DX11)
#include <d3d11_3.h>
#define GRAPHICS_API L"DX11"
#elif defined(VK)
#define GRAPHICS_API L"Vulkan"
#else
#error No graphics API defined!
#endif

// Standard library
#include <functional>
#include <list>
#include <memory>
#include <sstream>
#include <stdint.h>
#include <string>