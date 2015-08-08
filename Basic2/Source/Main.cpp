#include "Stdafx.h"

#include "BasicApplication.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	
	Kodiak::BasicApplication app(1280, 720, L"Basic Application");
    auto ret = app.Run(hInstance, nCmdShow);

	return ret;
}