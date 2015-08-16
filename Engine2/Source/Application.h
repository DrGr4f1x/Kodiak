#pragma once

namespace Kodiak
{

class Application
{
public:
	Application(uint32_t width, uint32_t height, const std::wstring& name);
	virtual ~Application();

	int Run(HINSTANCE hInstance, int nCmdShow);

protected:
	// Application event handlers
	virtual void OnInit() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnRender() = 0;
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
	// Rendering engine components
	std::unique_ptr<class Renderer> m_renderer{ nullptr };

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
};

} // namespace Application