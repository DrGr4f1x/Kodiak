// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "DeviceManager11.h"

#include "ColorBuffer11.h"
#include "CommandList11.h"
#include "CommandListManager11.h"
#include "DepthBuffer11.h"
#include "Format.h"
#include "Profile.h"
#include "RenderEnums11.h"
#include "RenderUtils.h"
#include "Shader.h"
#include "ShaderManager.h"


using namespace Kodiak;
using namespace Microsoft::WRL;
using namespace std;


namespace Kodiak
{
ID3D11Device* g_device = nullptr;
}

namespace
{

#if defined(_DEBUG)
	// Check for SDK Layer support.
	inline bool SdkLayersAvailable()
	{
		HRESULT hr = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_NULL,       // There is no need to create a real hardware device.
			0,
			D3D11_CREATE_DEVICE_DEBUG,  // Check for the SDK layers.
			nullptr,                    // Any feature level will do.
			0,
			D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Store apps.
			nullptr,                    // No need to keep the D3D device reference.
			nullptr,                    // No need to know the feature level.
			nullptr                     // No need to keep the D3D device context reference.
			);

		return SUCCEEDED(hr);
	}
#endif

} // anonymous namespace


DeviceManager::DeviceManager()
{
	CreateDeviceIndependentResources();
	CreateDeviceResources();
}


void DeviceManager::SetWindow(uint32_t width, uint32_t height, HWND hwnd)
{
	m_hwnd = hwnd;
	m_width = width;
	m_height = height;

	CreateWindowSizeDependentResources();
}


void DeviceManager::SetWindowSize(uint32_t width, uint32_t height)
{
	if (m_width != width || m_height != height)
	{
		m_width = width;
		m_height = height;

		CreateWindowSizeDependentResources();
	}
}


void DeviceManager::BeginFrame()
{
	if (m_d3dContext1)
	{
		// Discard the contents of the render target.
		// This is a valid operation only when the existing contents will be entirely
		// overwritten. If dirty or scroll rects are used, this call should be removed.
		//m_d3dContext1->DiscardView(m_backBuffer->GetRTV());
	}
}


void DeviceManager::Present(shared_ptr<ColorBuffer> presentSource)
{
	PROFILE_BEGIN(itt_present);

	auto commandList = GraphicsCommandList::Begin();

	PreparePresent(commandList, presentSource);

	commandList->CloseAndExecute();
	commandList = nullptr;

	// The first argument instructs DXGI to block until VSync, putting the application
	// to sleep until the next VSync. This ensures we don't waste any cycles rendering
	// frames that will never be displayed to the screen.
	HRESULT hr = m_swapChain->Present(1, 0);

	// If the device was removed either by a disconnection or a driver upgrade, we 
	// must recreate all device resources.
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		m_deviceRemoved = true;
	}
	else
	{
		ThrowIfFailed(hr);
	}

	PROFILE_END();
}


// Configures resources that don't depend on the Direct3D device.
void DeviceManager::CreateDeviceIndependentResources()
{
	// Initialize Direct2D resources.
	D2D1_FACTORY_OPTIONS options;
	ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));

#if defined(_DEBUG)
	// If the project is in a debug build, enable Direct2D debugging via SDK Layers.
	options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

	// Initialize the Direct2D Factory.
	ThrowIfFailed(
		D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED,
			__uuidof(ID2D1Factory2),
			&options,
			&m_d2dFactory
			)
		);

	// Initialize the DirectWrite Factory.
	ThrowIfFailed(
		DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory2),
			&m_dwriteFactory
			)
		);

	// Initialize the Windows Imaging Component (WIC) Factory.
	ThrowIfFailed(
		CoCreateInstance(
			CLSID_WICImagingFactory2,
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&m_wicFactory)
			)
		);
}


void DeviceManager::CreateDeviceResources()
{
	// This flag adds support for surfaces with a different color channel ordering
	// than the API default. It is required for compatibility with Direct2D.
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
	if (SdkLayersAvailable())
	{
		// If the project is in a debug build, enable debugging via SDK Layers with this flag.
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
	}
#endif

	// This array defines the set of DirectX hardware feature levels this app will support.
	// Note the ordering should be preserved.
	// Don't forget to declare your application's minimum required feature level in its
	// description.  All applications are assumed to support 9.1 unless otherwise stated.
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	// Create the Direct3D 11 API device object and a corresponding context.

	HRESULT hr = D3D11CreateDevice(
		nullptr,					// Specify nullptr to use the default adapter.
		D3D_DRIVER_TYPE_HARDWARE,	// Create a device using the hardware graphics driver.
		0,							// Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
		creationFlags,				// Set debug and Direct2D compatibility flags.
		featureLevels,				// List of feature levels this app can support.
		ARRAYSIZE(featureLevels),	// Size of the list above.
		D3D11_SDK_VERSION,			// Always set this to D3D11_SDK_VERSION for Windows Store apps.
		&m_d3dDevice,					// Returns the Direct3D device created.
		&m_d3dFeatureLevel,			// Returns feature level of device created.
		&m_d3dContext					// Returns the device immediate context.
		);

	if (FAILED(hr))
	{
		// If the initialization fails, fall back to the WARP device.
		// For more information on WARP, see: 
		// http://go.microsoft.com/fwlink/?LinkId=286690
		ThrowIfFailed(
			D3D11CreateDevice(
				nullptr,
				D3D_DRIVER_TYPE_WARP, // Create a WARP device instead of a hardware device.
				0,
				creationFlags,
				featureLevels,
				ARRAYSIZE(featureLevels),
				D3D11_SDK_VERSION,
				&m_d3dDevice,
				&m_d3dFeatureLevel,
				&m_d3dContext
				)
			);
	}

	g_device = m_d3dDevice.Get();

	// Initialize the command list manager
	CommandListManager::GetInstance().Create(m_d3dDevice.Get());

	// Cast pointers to DX11.1, 11.2, 11.3
	m_d3dDevice.As(&m_d3dDevice1);
	m_d3dContext.As(&m_d3dContext1);
	m_d3dDevice.As(&m_d3dDevice2);
	m_d3dContext.As(&m_d3dContext2);
	m_d3dDevice.As(&m_d3dDevice3);
	m_d3dContext.As(&m_d3dContext3);

	// Create state for present
	CreatePresentState();

	// Create the Direct2D device object and a corresponding context.
	ComPtr<IDXGIDevice3> dxgiDevice;
	ThrowIfFailed(
		m_d3dDevice.As(&dxgiDevice)
		);

	ThrowIfFailed(
		m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice)
		);

	ThrowIfFailed(
		m_d2dDevice->CreateDeviceContext(
			D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
			&m_d2dContext
			)
		);
}


void DeviceManager::CreateWindowSizeDependentResources()
{
	// Clear the previous window size specific context.
	ID3D11RenderTargetView* nullViews[] = { nullptr };
	m_d3dContext->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
	m_backBuffer = nullptr;
	m_d2dContext->SetTarget(nullptr);
	m_d2dTargetBitmap = nullptr;
	m_d3dContext->Flush();

	if (m_swapChain != nullptr)
	{
		// If the swap chain already exists, resize it.
		HRESULT hr = m_swapChain->ResizeBuffers(
			2, // Double-buffered swap chain.
			m_width,
			m_height,
			DXGI_FORMAT_B8G8R8A8_UNORM,
			0
			);

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			// If the device was removed for any reason, a new device and swap chain will need to be created.
			m_deviceRemoved = true;

			// Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this method 
			// and correctly set up the new device.
			return;
		}
		else
		{
			ThrowIfFailed(hr);
		}
	}
	else
	{
		// Otherwise, create a new one using the same adapter as the existing Direct3D device.
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };

		swapChainDesc.Width = m_width; // Match the size of the window.
		swapChainDesc.Height = m_height;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // This is the most common swap chain format.
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1; // Don't use multi-sampling.
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2; // Use double-buffering to minimize latency.
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // All Windows Store apps must use this SwapEffect.
		swapChainDesc.Flags = 0;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

		// This sequence obtains the DXGI factory that was used to create the Direct3D device above.
		ComPtr<IDXGIDevice3> dxgiDevice;
		ThrowIfFailed(
			m_d3dDevice.As(&dxgiDevice)
			);

		ComPtr<IDXGIAdapter> dxgiAdapter;
		ThrowIfFailed(
			dxgiDevice->GetAdapter(&dxgiAdapter)
			);

		ComPtr<IDXGIFactory2> dxgiFactory;
		ThrowIfFailed(
			dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory))
			);

		ThrowIfFailed(
			dxgiFactory->CreateSwapChainForHwnd(
				m_d3dDevice.Get(),
				m_hwnd,
				&swapChainDesc,
				nullptr,
				nullptr,
				&m_swapChain)
			);

		ThrowIfFailed(
			dxgiDevice->SetMaximumFrameLatency(1)
			);
	}

	// Create a render target view of the swap chain back buffer.
	ComPtr<ID3D11Texture2D> backBuffer;
	ThrowIfFailed(
		m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))
		);

	m_backBuffer = make_shared<ColorBuffer>();
	m_backBuffer->CreateFromSwapChain(this, "BackBuffer", backBuffer.Get());

#if 0
	// Create a Direct2D target bitmap associated with the
	// swap chain back buffer and set it as the current target.
	D2D1_BITMAP_PROPERTIES1 bitmapProperties =
		D2D1::BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
			96,
			96
			);

	ComPtr<IDXGISurface2> dxgiBackBuffer;
	ThrowIfFailed(
		m_swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer))
		);

	ThrowIfFailed(
		m_d2dContext->CreateBitmapFromDxgiSurface(
			dxgiBackBuffer.Get(),
			&bitmapProperties,
			&m_d2dTargetBitmap
			)
		);

	m_d2dContext->SetTarget(m_d2dTargetBitmap.Get());

	// Grayscale text anti-aliasing is recommended for all Windows Store apps.
	m_d2dContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
#endif
}


void DeviceManager::CreatePresentState()
{
	// Default blend state - no blend
	BlendStateDesc defaultBlendState;

	// Depth-stencil state - no depth test, no depth write
	DepthStencilStateDesc depthStencilState(false, false);

	// Rasterizer state - two-sided
	RasterizerStateDesc rasterizerState(CullMode::None, FillMode::Solid);

	// Load shaders
	auto vs = ShaderManager::GetInstance().LoadVertexShader(ShaderPath("Engine", "ScreenQuadVS.cso"));
	auto ps = ShaderManager::GetInstance().LoadPixelShader(ShaderPath("Engine", "ConvertLDRToDisplayPS.cso"));
	(vs->loadTask && ps->loadTask).wait();
	
	// Configure PSO
	m_convertLDRToDisplayPSO.SetBlendState(defaultBlendState);
	m_convertLDRToDisplayPSO.SetRasterizerState(rasterizerState);
	m_convertLDRToDisplayPSO.SetDepthStencilState(depthStencilState);
	m_convertLDRToDisplayPSO.SetVertexShader(vs.get());
	m_convertLDRToDisplayPSO.SetPixelShader(ps.get());

	m_convertLDRToDisplayPSO.Finalize();
}


void DeviceManager::PreparePresent(GraphicsCommandList* commandList, shared_ptr<ColorBuffer> presentSource)
{
	commandList->PIXBeginEvent("PreparePresent");
	commandList->UnbindRenderTargets();

	commandList->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetPipelineState(m_convertLDRToDisplayPSO);

	// Copy and convert the LDR present source to the current back buffer
	commandList->SetRenderTarget(*m_backBuffer);
	commandList->SetPixelShaderResource(0, presentSource->GetSRV());

	commandList->SetViewportAndScissor(0, 0, m_width, m_height);

	commandList->Draw(3);

	commandList->SetPixelShaderResource(0, nullptr);
	commandList->PIXEndEvent();
}