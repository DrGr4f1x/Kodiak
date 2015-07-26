#pragma once

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#define ENGINE_API_TEMPLATE
#else
#define ENGINE_API __declspec(dllimport)
#define ENGINE_API_TEMPLATE extern
#endif