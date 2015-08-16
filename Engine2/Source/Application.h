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
	virtual void OnInit() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnRender() = 0;
	virtual void OnDestroy() = 0;
	virtual bool OnEvent(MSG msg) = 0;
	virtual void OnCommandlineArgument(const wchar_t* argv, size_t length) {}

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

protected:
	// Rendering engine components
	std::unique_ptr<class Renderer> m_renderer;

	// Window dimensions.
	uint32_t m_width;
	uint32_t m_height;
	float m_aspectRatio;

	// Window handle.
	HWND m_hwnd;

	// Window title.
	std::wstring m_title;

private:
	void ParseCommandLineArgs();		
};

} // namespace Application