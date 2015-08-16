#include "Stdafx.h"

#include "BasicApplication.h"

static const wchar_t* appname = L"Basic Application " GRAPHICS_API;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	
	Kodiak::BasicApplication app(1280, 720, appname);
    auto ret = app.Run(hInstance, nCmdShow);

	return ret;
}