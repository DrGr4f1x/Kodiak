// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from Utility.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#pragma once

namespace Utility
{
inline void Print(const char* msg) { printf(msg); }
inline void Print(const wchar_t* msg) { wprintf(msg); }

inline void Printf(const char* format, ...)
{
	char buffer[256];
	va_list ap;
	va_start(ap, format);
	vsprintf_s(buffer, 256, format, ap);
	Print(buffer);
}

inline void Printf(const wchar_t* format, ...)
{
	wchar_t buffer[256];
	va_list ap;
	va_start(ap, format);
	vswprintf(buffer, 256, format, ap);
	Print(buffer);
}

#ifndef RELEASE
inline void PrintSubMessage(const char* format, ...)
{
	Print("--> ");
	char buffer[256];
	va_list ap;
	va_start(ap, format);
	vsprintf_s(buffer, 256, format, ap);
	Print(buffer);
	Print("\n");
}
inline void PrintSubMessage(const wchar_t* format, ...)
{
	Print("--> ");
	wchar_t buffer[256];
	va_list ap;
	va_start(ap, format);
	vswprintf(buffer, 256, format, ap);
	Print(buffer);
	Print("\n");
}
inline void PrintSubMessage(void)
{
}
#endif
}

#define halt( ... ) ERROR( __VA_ARGS__ ) __debugbreak();

#ifdef RELEASE

#define assert_msg( isTrue, ... ) (void)(isTrue)
#define warn_once_if( isTrue, ... ) (void)(isTrue)
#define warn_once_if_not( isTrue, ... ) (void)(isTrue)
#define error( msg, ... )
#define debugprint( msg, ... ) do {} while(0)
#define assert_succeeded( hr, ... ) (void)(hr)

#else	// !RELEASE

#define STRINGIFY(x) #x
#define STRINGIFY_BUILTIN(x) STRINGIFY(x)
#define assert_msg( isFalse, ... ) \
		if (!(bool)(isFalse)) { \
			Utility::Print("\nAssertion failed in " STRINGIFY_BUILTIN(__FILE__) " @ " STRINGIFY_BUILTIN(__LINE__) "\n"); \
			Utility::PrintSubMessage("\'" #isFalse "\' is false"); \
			Utility::PrintSubMessage(__VA_ARGS__); \
			Utility::Print("\n"); \
			__debugbreak(); \
		}

#define assert_succeeded( hr, ... ) \
		if (FAILED(hr)) { \
			Utility::Print("\nHRESULT failed in " STRINGIFY_BUILTIN(__FILE__) " @ " STRINGIFY_BUILTIN(__LINE__) "\n"); \
			Utility::PrintSubMessage("hr = 0x%08X", hr); \
			Utility::PrintSubMessage(__VA_ARGS__); \
			Utility::Print("\n"); \
			__debugbreak(); \
		}


#define warn_once_if( isTrue, ... ) \
	{ \
		static bool s_TriggeredWarning = false; \
		if ((bool)(isTrue) && !s_TriggeredWarning) { \
			s_TriggeredWarning = true; \
			Utility::Print("\nWarning issued in " STRINGIFY_BUILTIN(__FILE__) " @ " STRINGIFY_BUILTIN(__LINE__) "\n"); \
			Utility::PrintSubMessage("\'" #isTrue "\' is true"); \
			Utility::PrintSubMessage(__VA_ARGS__); \
			Utility::Print("\n"); \
		} \
	}

#define warn_once_if_not( isTrue, ... ) warn_once_if(!(isTrue), __VA_ARGS__)

#define error( ... ) \
		Utility::Print("\nError reported in " STRINGIFY_BUILTIN(__FILE__) " @ " STRINGIFY_BUILTIN(__LINE__) "\n"); \
		Utility::PrintSubMessage(__VA_ARGS__); \
		Utility::Print("\n");

#define debugprint( msg, ... ) \
	Utility::Printf( msg "\n", ##__VA_ARGS__ );

#endif