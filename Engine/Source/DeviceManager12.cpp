// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "DeviceManager12.h"

#include "CommandList12.h"
#include "CommandListManager12.h"
#include "CommandSignature12.h"
#include "DXGIUtility.h"
#include "Format.h"
#include "Renderer.h"
#include "RenderEnums12.h"
#include "RenderUtils.h"
#include "Shader.h"


using namespace Kodiak;
using namespace Microsoft::WRL;
using namespace std;


static uint32_t g_currentFrame = 0;

namespace Kodiak
{
ID3D12Device* g_device = nullptr;
} // namespace Kodiak


DeviceManager& DeviceManager::GetInstance()
{
	static DeviceManager instance;
	return instance;
}


DeviceManager::DeviceManager()
	: m_descriptorAllocator{D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,	D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,	D3D12_DESCRIPTOR_HEAP_TYPE_RTV,	D3D12_DESCRIPTOR_HEAP_TYPE_DSV }
{
	m_swapChainFormat = ColorFormat::R10G10B10A2_UNorm;

	CreateDeviceIndependentResources();
	CreateDeviceResources();
	m_currentFrame = 0;
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


void DeviceManager::Finalize()
{
	// Wait until all previous GPU work is complete.
	CommandListManager::GetInstance().IdleGpu();
}


void DeviceManager::Present(shared_ptr<ColorBuffer> presentSource, bool bHDRPresent, const PresentParameters& params)
{
	auto& commandList = GraphicsCommandList::Begin();

	if(bHDRPresent)
	{ 
		PreparePresentHDR(commandList, presentSource, params);
	}
	else
	{
		PreparePresentLDR(commandList, presentSource);
	}

	commandList.CloseAndExecute();
	
	// TODO: better vsync logic here
	m_swapChain->Present(1, 0);

	// Switch to the next frame & back buffer
	g_currentFrame = (g_currentFrame + 1) % SWAP_CHAIN_BUFFER_COUNT;
}


D3D12_CPU_DESCRIPTOR_HANDLE DeviceManager::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t count)
{
	return m_descriptorAllocator[type].Allocate(this, count);
}


void DeviceManager::CreateDeviceIndependentResources()
{
	D2D1_FACTORY_OPTIONS d2dFactoryOptions = {};
#ifdef _DEBUG
	// Enable the D2D debug layer.
	d2dFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

	// Initialize the Direct2D Factory.
	ThrowIfFailed(
		D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED,
			__uuidof(ID2D1Factory3),
			&d2dFactoryOptions,
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
	UINT d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	// Enable the D3D11 debug layer.
	d3d11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;

	// Enable the D3D12 debug layer.
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
	}
#endif

	// Create DXGI device.
	ComPtr<IDXGIFactory4> factory;
	ThrowIfFailed(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)));

	ComPtr<IDXGIAdapter1> adapter;
	ComPtr<IDXGIAdapter1> bestAdapter;
	size_t maxDedicatedVideoMemory = 0;
	for (uint32_t i = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(i, &adapter); ++i)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);
		if(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}

		if (desc.DedicatedVideoMemory > maxDedicatedVideoMemory)
		{
			maxDedicatedVideoMemory = desc.DedicatedVideoMemory;
			bestAdapter = adapter;
		}
	}

	// Create DirectX 12 device.
	ThrowIfFailed(D3D12CreateDevice(bestAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));

	// We like to do read-modify-write operations on UAVs during post processing.  To support that, we
	// need to either have the hardware do typed UAV loads of R11G11B10_FLOAT or we need to manually
	// decode an R32_UINT representation of the same buffer.  This code determines if we get the hardware
	// load support.
	D3D12_FEATURE_DATA_D3D12_OPTIONS featureData = {};
	if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &featureData, sizeof(featureData))))
	{
		if (featureData.TypedUAVLoadAdditionalFormats)
		{
			D3D12_FEATURE_DATA_FORMAT_SUPPORT support =
			{
				DXGI_FORMAT_R11G11B10_FLOAT, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE
			};

			if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &support, sizeof(support))) &&
				(support.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) != 0)
			{
				m_supportsTypedUAVLoad_R11G11B10_FLOAT = true;
			}
		}
	}

	g_device = m_device.Get();

	// Initialize the command list manager
	CommandListManager::GetInstance().Create(m_device.Get());

	// Initalize graphics state for present
	CreatePresentState();

	DispatchIndirectCommandSignature[0].Dispatch();
	DispatchIndirectCommandSignature.Finalize();

	DrawIndirectCommandSignature[0].Draw();
	DrawIndirectCommandSignature.Finalize();

#if 0
	// Create an 11 device wrapped around the 12 device and share
	// 12's command queue.
	ComPtr<ID3D11Device> d3d11Device;
	ThrowIfFailed(D3D11On12CreateDevice(
		m_device.Get(),
		d3d11DeviceFlags,
		nullptr,
		0,
		reinterpret_cast<IUnknown**>(m_commandQueue.GetAddressOf()),
		1,
		0,
		&d3d11Device,
		&m_d3d11DeviceContext,
		nullptr
		));

	// Query the 11On12 device from the 11 device.
	ThrowIfFailed(d3d11Device.As(&m_d3d11On12Device));

	// Create the Direct2D device object and a corresponding context.
	ComPtr<IDXGIDevice> dxgiDevice;
	ThrowIfFailed(
		m_d3d11On12Device.As(&dxgiDevice)
		);

	ThrowIfFailed(
		m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice)
		);

	D2D1_DEVICE_CONTEXT_OPTIONS deviceOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
	ThrowIfFailed(
		m_d2dDevice->CreateDeviceContext(
			deviceOptions,
			&m_d2dDeviceContext
			)
		);
#endif
}


void DeviceManager::CreateWindowSizeDependentResources()
{
	// Wait until all previous GPU work is complete.
	CommandListManager::GetInstance().IdleGpu();

	if (m_swapChain != nullptr)
	{
		// If the swap chain already exists, resize it.
		HRESULT hr = m_swapChain->ResizeBuffers(
			SWAP_CHAIN_BUFFER_COUNT,
			m_width,
			m_height,
			DXGIUtility::ConvertToDXGI(m_swapChainFormat),
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
		ComPtr<IDXGIFactory4> factory;
		ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));

		// Describe and create the swap chain.
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
		swapChainDesc.BufferDesc.Width = m_width;
		swapChainDesc.BufferDesc.Height = m_height;
		swapChainDesc.BufferDesc.Format = DXGIUtility::ConvertToDXGI(m_swapChainFormat);
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		swapChainDesc.OutputWindow = m_hwnd;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.Windowed = TRUE;

		ComPtr<IDXGISwapChain> swapChain;
		auto commandQueue = CommandListManager::GetInstance().GetCommandQueue();
		ThrowIfFailed(factory->CreateSwapChain(
			commandQueue,		// Swap chain needs the queue so that it can force a flush on it.
			&swapChainDesc,
			&swapChain
			));

		ThrowIfFailed(swapChain.As(&m_swapChain));
	}

	// Create ColorBuffer views of the swap chain's display planes
	for (uint32_t i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
	{
		ComPtr<ID3D12Resource> displayPlane;
		ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&displayPlane)));
		m_backbuffers[i].CreateFromSwapChain(this, "Primary SwapChain Buffer", displayPlane.Detach());
	}
}


void DeviceManager::CreatePresentState()
{
	// TODO: put this sampler desc in some factory class for standard samplers
	D3D12_SAMPLER_DESC samplerLinearClampDesc;
	ZeroMemory(&samplerLinearClampDesc, sizeof(D3D12_SAMPLER_DESC));
	samplerLinearClampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerLinearClampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerLinearClampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerLinearClampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerLinearClampDesc.MinLOD = -FLT_MAX;
	samplerLinearClampDesc.MaxLOD = FLT_MAX;
	samplerLinearClampDesc.MaxAnisotropy = 16;
	samplerLinearClampDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;

	D3D12_SAMPLER_DESC samplerPointClampDesc;
	ZeroMemory(&samplerPointClampDesc, sizeof(D3D12_SAMPLER_DESC));
	samplerPointClampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	samplerPointClampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerPointClampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerPointClampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerPointClampDesc.MinLOD = -FLT_MAX;
	samplerPointClampDesc.MaxLOD = FLT_MAX;
	samplerPointClampDesc.MaxAnisotropy = 16;
	samplerPointClampDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;

	// Configure the root signature for a single input SRV and sampler
	m_presentRS.Reset(3, 2);
	m_presentRS[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 2, D3D12_SHADER_VISIBILITY_PIXEL);
	m_presentRS[1].InitAsConstants(0, 6, D3D12_SHADER_VISIBILITY_PIXEL);
	m_presentRS[2].InitAsBufferSRV(2, D3D12_SHADER_VISIBILITY_PIXEL);
	m_presentRS.InitStaticSampler(0, samplerLinearClampDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_presentRS.InitStaticSampler(1, samplerPointClampDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_presentRS.Finalize();

	// Default blend state - no blend
	BlendStateDesc defaultBlendState;

	// Depth-stencil state - no depth test, no depth write
	DepthStencilStateDesc depthStencilState(false, false);

	// Rasterizer state - two-sided
	RasterizerStateDesc rasterizerState(CullMode::None, FillMode::Solid);

	// Load shaders
	auto screenQuadVs = VertexShader::LoadImmediate("Engine\\ScreenQuadVS.dx.cso");
	auto convertLDRToDisplayPs = PixelShader::LoadImmediate("Engine\\ConvertLDRToDisplayPS.dx.cso");
	auto convertHDRToDisplayPs = PixelShader::LoadImmediate("Engine\\ConvertHDRToDisplayPS.dx.cso");
	
	// Configure PSO
	m_convertLDRToDisplayPSO.SetRootSignature(m_presentRS);
	m_convertLDRToDisplayPSO.SetBlendState(defaultBlendState);
	m_convertLDRToDisplayPSO.SetRasterizerState(rasterizerState);
	m_convertLDRToDisplayPSO.SetDepthStencilState(depthStencilState);
	m_convertLDRToDisplayPSO.SetVertexShader(screenQuadVs.get());
	m_convertLDRToDisplayPSO.SetPixelShader(convertLDRToDisplayPs.get());
	//m_convertLDRToDisplayPSO.SetInputLayout(0, nullptr);
	m_convertLDRToDisplayPSO.SetSampleMask(0xFFFFFFFF);
	m_convertLDRToDisplayPSO.SetPrimitiveTopology(PrimitiveTopologyType::Triangle);
	m_convertLDRToDisplayPSO.SetRenderTargetFormat(m_swapChainFormat, DepthFormat::Unknown);
	m_convertLDRToDisplayPSO.Finalize();

	m_convertHDRToDisplayPSO.SetRootSignature(m_presentRS);
	m_convertHDRToDisplayPSO.SetBlendState(defaultBlendState);
	m_convertHDRToDisplayPSO.SetRasterizerState(rasterizerState);
	m_convertHDRToDisplayPSO.SetDepthStencilState(depthStencilState);
	m_convertHDRToDisplayPSO.SetVertexShader(screenQuadVs.get());
	m_convertHDRToDisplayPSO.SetPixelShader(convertHDRToDisplayPs.get());
	//m_convertLDRToDisplayPSO.SetInputLayout(0, nullptr);
	m_convertHDRToDisplayPSO.SetSampleMask(0xFFFFFFFF);
	m_convertHDRToDisplayPSO.SetPrimitiveTopology(PrimitiveTopologyType::Triangle);
	m_convertHDRToDisplayPSO.SetRenderTargetFormat(m_swapChainFormat, DepthFormat::Unknown);
	m_convertHDRToDisplayPSO.Finalize();
}


void DeviceManager::PreparePresentLDR(GraphicsCommandList& commandList, shared_ptr<ColorBuffer> presentSource)
{
	// Transition the present source so we can read from it
	commandList.TransitionResource(*presentSource, ResourceState::PixelShaderResource);

	commandList.SetRootSignature(m_presentRS);
	commandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Copy and convert the LDR present source to the current back buffer
	commandList.SetRenderTarget(m_backbuffers[g_currentFrame]);
	commandList.SetDynamicDescriptor(0, 0, presentSource->GetSRV());

	commandList.SetPipelineState(m_convertLDRToDisplayPSO);
	commandList.SetViewportAndScissor(0, 0, m_width, m_height);

	commandList.Draw(3);

	// Transition the current back buffer to present mode
	commandList.TransitionResource(m_backbuffers[g_currentFrame], ResourceState::Present);
}


void DeviceManager::PreparePresentHDR(GraphicsCommandList& commandList, shared_ptr<ColorBuffer> presentSource, const PresentParameters& params)
{
	commandList.TransitionResource(*presentSource, ResourceState::PixelShaderResource);

	commandList.SetRootSignature(m_presentRS);
	commandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Copy and convert the HDR present source to the current back buffer
	commandList.SetRenderTarget(m_backbuffers[g_currentFrame]);
	commandList.SetDynamicDescriptor(0, 0, presentSource->GetSRV());

	float toeStrength = params.ToeStrength < 1e-6f ? 1e32f : 1.0f / params.ToeStrength;

	commandList.SetConstants(1, (float)params.PaperWhite, (float)params.MaxBrightness, (float)toeStrength, (int)params.DebugMode);

	commandList.SetPipelineState(m_convertHDRToDisplayPSO);
	commandList.SetViewportAndScissor(0, 0, m_width, m_height);

	commandList.Draw(3);

	// Transition the current back buffer to present mode
	commandList.TransitionResource(m_backbuffers[g_currentFrame], ResourceState::Present);
}