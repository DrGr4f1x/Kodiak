// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "StepTimer.h"

namespace Kodiak
{

// Forward declarations
class StepTimer;

class Application
{
public:
	Application(uint32_t width, uint32_t height, const std::wstring& name);
	virtual ~Application();

	int Run(HINSTANCE hInstance, int nCmdShow);

protected:
	// Application event handlers
	virtual void OnInit() = 0;
	virtual void OnUpdate(StepTimer* timer) = 0;
	virtual void OnDestroy() = 0;
	virtual void OnCommandlineArgument(const wchar_t* argv, size_t length) {}

	// Top-level Windows message dispatcher
	virtual bool OnEvent(MSG msg);

	// Windows message handlers
	virtual void OnResize();
	virtual void OnMouseDown(WPARAM btnState, int x, int y) { }
	virtual void OnMouseUp(WPARAM btnState, int x, int y) { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y) { }

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

protected:
	// Window dimensions.
	uint32_t m_width{ 1 };
	uint32_t m_height{ 1 };
	float m_aspectRatio{ 0.0f };

	// Window handle.
	HWND m_hwnd{ 0 };

	// Window title.
	std::wstring m_title{ L"Application" };

	// Application state
	bool m_paused{ false };
	bool m_minimized{ false };
	bool m_maximized{ false };
	bool m_resizing{ false };

private:
	void ParseCommandLineArgs();
	void Update();
	bool Render();

private:
	// Rendering loop timer
	std::unique_ptr<StepTimer> m_timer;
};

} // namespace Application