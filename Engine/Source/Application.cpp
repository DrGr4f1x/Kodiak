// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "Application.h"

#include "CommandList.h"
#include "DeviceManager.h"
#include "InputState.h"
#include "Log.h"
#include "Profile.h"
#include "Renderer.h"
#include "StepTimer.h"

#include <shellapi.h>
#include <WindowsX.h>


using namespace Kodiak;
using namespace std;

Application::Application(uint32_t width, uint32_t height, const std::wstring& name)
{
	InitializeProfiling();
	InitializeLogging();

	m_width = width;
	m_height = height;
	m_title = name;

	m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	
	// Start up renderer
	Renderer::GetInstance().Initialize();

	// Create step timer
	m_timer = make_unique<StepTimer>();
}


Application::~Application()
{
	m_inputState->Shutdown();
	ShutdownLogging();
	ShutdownProfiling();
}


int Application::Run(HINSTANCE hInstance, int nCmdShow)
{
	// Initialize the window class.
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = L"WindowClass1";
	RegisterClassEx(&windowClass);

	RECT windowRect = { 0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	// Create the window and store a handle to it.
	m_hwnd = CreateWindowEx(NULL,
		L"WindowClass1",
		m_title.c_str(),
		WS_OVERLAPPEDWINDOW,
		100,
		100,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,		// We have no parent window, NULL.
		NULL,		// We aren't using menus, NULL.
		hInstance,
		NULL);		// We aren't using multiple windows, NULL.

	ShowWindow(m_hwnd, nCmdShow);

	m_inputState = make_shared<InputState>();
	m_inputState->Initialize(m_hwnd);

	// Initialize the sample. OnInit is defined in each child-implementation of Application.
	OnInit();

	// Main sample loop.
	MSG msg = { 0 };
	while (true)
	{
		// Process any messages in the queue.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				break;

			// Pass events into our sample.
			OnEvent(msg);
		}

		Update();
		Render();
	}

	OnDestroy();

	// Return this part of the WM_QUIT message to Windows.
	return static_cast<char>(msg.wParam);
}


// Message handler for the application
bool Application::OnEvent(MSG msg)
{
	switch (msg.message)
	{
	// WM_ACTIVATE is sent when the window is activated or deactivated.  
	// We pause the game when the window is deactivated and unpause it 
	// when it becomes active.  
	case WM_ACTIVATE:
		if (LOWORD(msg.wParam) == WA_INACTIVE)
		{
			m_paused = true;
		}
		else
		{
			m_paused = false;
		}
		return true;
		break;

	// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		// Save the new client area dimensions
		m_width = LOWORD(msg.lParam);
		m_height = HIWORD(msg.lParam);

		if (msg.wParam == SIZE_MINIMIZED)
		{
			m_paused = true;
			m_minimized = true;
			m_maximized = false;
		}
		else if (msg.wParam == SIZE_MAXIMIZED)
		{
			m_paused = false;
			m_minimized = false;
			m_maximized = true;
		}
		else if (msg.wParam == SIZE_RESTORED)
		{
			// Restoring from minimized state?
			if (m_minimized)
			{
				m_paused = false;
				m_minimized = false;
				OnResize();
			}

			// Restoring from maximized state?
			else if (m_maximized)
			{
				m_paused = false;
				m_maximized = false;
				OnResize();
			}
			else if (m_resizing)
			{
				// If user is dragging the resize bars, we do not resize 
				// the buffers here because as the user continuously 
				// drags the resize bars, a stream of WM_SIZE messages are
				// sent to the window, and it would be pointless (and slow)
				// to resize for each WM_SIZE message received from dragging
				// the resize bars.  So instead, we reset after the user is 
				// done resizing the window and releases the resize bars, which 
				// sends a WM_EXITSIZEMOVE message.
			}
			else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
			{
				OnResize();
			}
		}
		return true;
		break;

	// WM_ENTERSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		m_paused = true;
		m_resizing = true;
		return true;
		break;

	// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
	// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		m_paused = false;
		m_resizing = false;
		OnResize();
		return true;
		break;

	// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)msg.lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)msg.lParam)->ptMinTrackSize.y = 200;
		return true;
		break;
	}
	return false;
}


// Handle window resize events
void Application::OnResize()
{
	Renderer::GetInstance().SetWindowSize(m_width, m_height);
}


// Main message handler for the sample.
LRESULT CALLBACK Application::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Handle destroy/shutdown messages.
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	// Handle any messages the switch statement didn't.
	return DefWindowProc(hWnd, message, wParam, lParam);
}


// Helper function for parsing any supplied command line args.
void Application::ParseCommandLineArgs()
{
	int argc;
	LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	for (int i = 1; i < argc; ++i)
	{
		OnCommandlineArgument(argv[i], wcslen(argv[i]));
	}
	LocalFree(argv);
}


void Application::Update()
{
	PROFILE(application_Update);

	// Update scene objects
	m_timer->Tick([this]()
	{
		// Update the input state
		m_inputState->Update(static_cast<float>(m_timer->GetElapsedSeconds()));

		// Subclasses implement OnUpdate to supply their own scene update logic
		OnUpdate(m_timer.get());
	});
}


bool Application::Render()
{
	PROFILE(application_Render);

	// Don't try to render until we've run one Update tick
	if (m_timer->GetFrameCount() == 0)
	{
		return false;
	}

	Renderer::GetInstance().Render();

	return true;
}