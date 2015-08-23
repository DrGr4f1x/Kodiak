#include "Stdafx.h"

#include "DeviceResources12.h"

#include "CommandList.h"
#include "IndexBuffer.h"
#include "RenderEnums.h"
#include "RenderTargetView.h"
#include "RenderUtils.h"
#include "VertexBuffer.h"

#include <locale>
#include <codecvt>


using namespace Kodiak;
using namespace Microsoft::WRL;
using namespace std;


DeviceResources::DeviceResources()
{
	ZeroMemory(m_fenceValues, sizeof(m_fenceValues));
	CreateDeviceIndependentResources();
	CreateDeviceResources();
}


void DeviceResources::SetWindow(uint32_t width, uint32_t height, HWND hwnd)
{
	m_hwnd = hwnd;
	m_width = width;
	m_height = height;

	CreateWindowSizeDependentResources();
}


void DeviceResources::SetWindowSize(uint32_t width, uint32_t height)
{
	if (m_width != width || m_height != height)
	{
		m_width = width;
		m_height = height;

		CreateWindowSizeDependentResources();
	}
}


void DeviceResources::Finalize()
{
	WaitForGPU();

	CloseHandle(m_fenceEvent);
}


shared_ptr<RenderTargetView> DeviceResources::GetBackBuffer()
{
	return m_backbuffers[m_currentFrame];
}


void DeviceResources::Present()
{
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

		MoveToNextFrame();
	}
}


void DeviceResources::EndFrame()
{
	MoveToNextFrame();
}


shared_ptr<CommandList> DeviceResources::CreateCommandList()
{
	auto commandList = make_shared<CommandList>(FrameCount);

	for (int32_t n = 0; n < FrameCount; ++n)
	{
		// Create command allocator
		ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandList->m_commandAllocators[n])));

		// Create command list
		ThrowIfFailed(m_device->CreateCommandList(
			0, 
			D3D12_COMMAND_LIST_TYPE_DIRECT, 
			commandList->m_commandAllocators[n].Get(), 
			nullptr, 
			IID_PPV_ARGS(&commandList->m_commandLists[n])));

		// Command lists are created in the recording state, but there is nothing
		// to record yet. The main loop expects it to be closed, so close it now.
		ThrowIfFailed(commandList->m_commandLists[n]->Close());
	}

	commandList->m_currentFrame = GetCurrentFrame();

	return commandList;
}


void DeviceResources::ExecuteCommandList(const shared_ptr<CommandList>& commandList)
{
	ID3D12CommandList* ppCommandLists[] = { commandList->m_commandLists[m_currentFrame].Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}


ComPtr<ID3D12CommandAllocator> DeviceResources::GetCommandAllocator()
{
	return m_commandAllocators[m_currentFrame];
}


void DeviceResources::CreateIndexBuffer(
	shared_ptr<CommandList> commandList,
	shared_ptr<IndexBuffer> ibuffer,
	IIndexBufferData* data,
	Usage usage,
	const std::string& debugName)
{
	// Create the index buffer resource in the GPU's default heap and copy index data into it using the upload heap.
	// The upload resource must not be released until after the GPU has finished using it.
	ComPtr<ID3D12Resource> indexBufferUpload;

	CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);

	CD3DX12_RESOURCE_DESC indexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(data->GetSize());
	ThrowIfFailed(m_device->CreateCommittedResource(
		&defaultHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&indexBufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&ibuffer->buffer)));

	CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
	ThrowIfFailed(m_device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&indexBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBufferUpload)));

	wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
	wstring wide = converter.from_bytes(debugName);
	ibuffer->buffer->SetName(wide.c_str());
	indexBufferUpload->SetName(L"Index Buffer Upload Resource");

	// Upload the index buffer to the GPU.
	{
		D3D12_SUBRESOURCE_DATA indexData = {};
		indexData.pData = data->GetData();
		indexData.RowPitch = data->GetSize();
		indexData.SlicePitch = indexData.RowPitch;

		commandList->Begin();
		auto _commandList = commandList->GetCommandList();

		UpdateSubresources(_commandList, ibuffer->buffer.Get(), indexBufferUpload.Get(), 0, 0, 1, &indexData);

		CD3DX12_RESOURCE_BARRIER indexBufferResourceBarrier =
			CD3DX12_RESOURCE_BARRIER::Transition(ibuffer->buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		_commandList->ResourceBarrier(1, &indexBufferResourceBarrier);

		ThrowIfFailed(_commandList->Close());

		{
			lock_guard<mutex> guard(m_uploadMutex);

			// Execute the upload command list
			ID3D12CommandList* ppCommandLists[] = { _commandList };
			m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

			// Schedule a Signal command in the queue.
			ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_currentFrame]));

			// Wait until the fence has been crossed.
			ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_currentFrame], commandList->m_fenceEvent));
			
			// Increment the fence value for the current frame.
			m_fenceValues[m_currentFrame]++;
		}

		// Create vertex buffer view.
		ibuffer->indexBufferView.BufferLocation = ibuffer->buffer->GetGPUVirtualAddress();
		ibuffer->indexBufferView.SizeInBytes = static_cast<UINT>(data->GetSize());
		ibuffer->indexBufferView.Format = (DXGI_FORMAT)data->GetFormat();
	}
}


void DeviceResources::CreateVertexBuffer(
	std::shared_ptr<CommandList> commandList,
	std::shared_ptr<VertexBuffer> vbuffer,
	IVertexBufferData* data,
	Usage usage,
	const std::string& debugName)
{
	// Create the vertex buffer resource in the GPU's default heap and copy vertex data into it using the upload heap.
	// The upload resource must not be released until after the GPU has finished using it.
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferUpload;

	CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);

	CD3DX12_RESOURCE_DESC vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(data->GetSize());
	ThrowIfFailed(m_device->CreateCommittedResource(
		&defaultHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&vertexBufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&vbuffer->buffer)));

	CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
	ThrowIfFailed(m_device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&vertexBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBufferUpload)));

	wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
	wstring wide = converter.from_bytes(debugName);
	vbuffer->buffer->SetName(wide.c_str());
	vertexBufferUpload->SetName(L"Vertex Buffer Upload Resource");

	// Upload the vertex buffer to the GPU.
	{
		D3D12_SUBRESOURCE_DATA vertexData = {};
		vertexData.pData = data->GetData();
		vertexData.RowPitch = data->GetSize();
		vertexData.SlicePitch = vertexData.RowPitch;

		commandList->Begin();
		auto _commandList = commandList->GetCommandList();

		UpdateSubresources(_commandList, vbuffer->buffer.Get(), vertexBufferUpload.Get(), 0, 0, 1, &vertexData);

		CD3DX12_RESOURCE_BARRIER vertexBufferResourceBarrier =
			CD3DX12_RESOURCE_BARRIER::Transition(vbuffer->buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		_commandList->ResourceBarrier(1, &vertexBufferResourceBarrier);

		ThrowIfFailed(_commandList->Close());

		{
			lock_guard<mutex> guard(m_uploadMutex);

			// Execute the upload command list
			ID3D12CommandList* ppCommandLists[] = { _commandList };
			m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

			// Schedule a Signal command in the queue.
			ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_currentFrame]));

			// Wait until the fence has been crossed.
			ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_currentFrame], commandList->m_fenceEvent));

			// Increment the fence value for the current frame.
			m_fenceValues[m_currentFrame]++;
		}

		// Create vertex buffer view.
		vbuffer->vertexBufferView.BufferLocation = vbuffer->buffer->GetGPUVirtualAddress();
		vbuffer->vertexBufferView.StrideInBytes = static_cast<UINT>(data->GetStride());
		vbuffer->vertexBufferView.SizeInBytes = static_cast<UINT>(data->GetSize());
	}
}


void DeviceResources::CreateDeviceIndependentResources()
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

	// Create an event handle to use for frame synchronization.
	m_fenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
	if (m_fenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}
}


void DeviceResources::CreateDeviceResources()
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
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));

	// Create DirectX 12 device.
	ThrowIfFailed(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));

	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

	for (uint32_t n = 0; n < FrameCount; ++n)
	{
		ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[n])));
	}

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

	// Create synchronization objects.
	{
		ThrowIfFailed(m_device->CreateFence(m_fenceValues[m_currentFrame], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
		m_fenceValues[m_currentFrame]++;

		m_fenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
	}
}


void DeviceResources::CreateWindowSizeDependentResources()
{
	// Wait until all previous GPU work is complete.
	WaitForGPU();

	// Clear the previous window size specific content.
	for (UINT n = 0; n < FrameCount; n++)
	{
		m_backbuffers[n] = nullptr;
	}
	m_rtvHeap = nullptr;

	if (m_swapChain != nullptr)
	{
		// If the swap chain already exists, resize it.
		HRESULT hr = m_swapChain->ResizeBuffers(
			FrameCount,
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
		ComPtr<IDXGIFactory4> factory;
		ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));

		// Describe and create the swap chain.
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferCount = FrameCount;
		swapChainDesc.BufferDesc.Width = m_width;
		swapChainDesc.BufferDesc.Height = m_height;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.OutputWindow = m_hwnd;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.Windowed = TRUE;

		ComPtr<IDXGISwapChain> swapChain;
		ThrowIfFailed(factory->CreateSwapChain(
			m_commandQueue.Get(),		// Swap chain needs the queue so that it can force a flush on it.
			&swapChainDesc,
			&swapChain
			));

		ThrowIfFailed(swapChain.As(&m_swapChain));
	}

	// Create a render target view of the swap chain back buffer.
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = FrameCount;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_rtvHeap)));
		m_rtvHeap->SetName(L"Render Target View Descriptor Heap");

		// All pending GPU work was already finished. Update the tracked fence values
		// to the last value signaled.
		for (UINT n = 0; n < FrameCount; n++)
		{
			m_fenceValues[n] = m_fenceValues[m_currentFrame];
		}

		m_currentFrame = 0;
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
		m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		for (UINT n = 0; n < FrameCount; n++)
		{
			m_backbuffers[n] = make_shared<RenderTargetView>();

			ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_backbuffers[n]->m_rtv)));
			m_device->CreateRenderTargetView(m_backbuffers[n]->m_rtv.Get(), nullptr, rtvDescriptor);
			m_backbuffers[n]->m_rtvHandle = rtvDescriptor;
			rtvDescriptor.Offset(m_rtvDescriptorSize);

			WCHAR name[25];
			swprintf_s(name, L"Render Target %d", n);
			m_backbuffers[n]->m_rtv->SetName(name);
		}
	}
}


void DeviceResources::MoveToNextFrame()
{
	// Schedule a Signal command in the queue.
	const uint64_t currentFenceValue = m_fenceValues[m_currentFrame];
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));

	// Advance the frame index.
	m_currentFrame = (m_currentFrame + 1) % FrameCount;

	// Check to see if the next frame is ready to start.
	if (m_fence->GetCompletedValue() < m_fenceValues[m_currentFrame])
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_currentFrame], m_fenceEvent));
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
	}

	// Set the fence value for the next frame.
	m_fenceValues[m_currentFrame] = currentFenceValue + 1;
}


void DeviceResources::WaitForGPU()
{
	// Schedule a Signal command in the queue.
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_currentFrame]));

	// Wait until the fence has been crossed.
	ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_currentFrame], m_fenceEvent));
	WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

	// Increment the fence value for the current frame.
	m_fenceValues[m_currentFrame]++;
}