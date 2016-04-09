// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from CommandContext.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#pragma once

#include "DynamicDescriptorHeap12.h"
#include "GpuBuffer12.h"
#include "LinearAllocator12.h"
#include "RenderEnums12.h"
#include "RootSignature12.h"

namespace Kodiak
{

// Forward declarations
class ColorBuffer;
class CommandListManager;
class ComputeCommandList;
class ComputePSO;
class ConstantBuffer;
class DepthBuffer;
class GpuBuffer;
class GpuResource;
class GraphicsCommandList;
class GraphicsPSO;
class IndexBuffer;
class RenderTargetView;
class RootSignature;
class VertexBuffer;
struct Rectangle;
struct Viewport;


struct DWParam
{
	DWParam(FLOAT f) : Float(f) {}
	DWParam(UINT u) : Uint(u) {}
	DWParam(INT i) : Int(i) {}

	void operator= (FLOAT f) { Float = f; }
	void operator= (UINT u) { Uint = u; }
	void operator= (INT i) { Int = i; }

	union
	{
		FLOAT	Float;
		UINT	Uint;
		INT		Int;
	};
};


class CommandList
{
public:
	CommandList();
	virtual ~CommandList();

	static void DestroyAllCommandLists();
	static CommandList* Begin();
	uint64_t CloseAndExecute(bool waitForCompletion = false);

	void Initialize(CommandListManager& manager);

	GraphicsCommandList* GetGraphicsCommandList()
	{
		return reinterpret_cast<GraphicsCommandList*>(this);
	}

	ComputeCommandList* GetComputeCommandList()
	{
		return reinterpret_cast<ComputeCommandList*>(this);
	}

	void CopyBuffer(GpuResource& dest, GpuResource& src);
	void CopyBufferRegion(GpuResource& dest, size_t destOffset, GpuResource& src, size_t srcOffset, size_t numBytes);

	static void InitializeTexture(GpuResource& dest, UINT numSubresources, D3D12_SUBRESOURCE_DATA subData[]);
	static void InitializeBuffer(GpuResource& dest, const void* data, size_t numBytes);

	void WriteBuffer(GpuResource& dest, size_t destOffset, const void* data, size_t numBytes);
	void FillBuffer(GpuResource& dest, size_t destOffset, DWParam value, size_t numBytes);

	void TransitionResource(GpuResource& Resource, ResourceState NewState, bool FlushImmediate = false);
	void BeginResourceTransition(GpuResource& Resource, ResourceState NewState, bool FlushImmediate = false);
	void InsertUAVBarrier(GpuResource& Resource, bool FlushImmediate = false);
	void InsertAliasBarrier(GpuResource& Before, GpuResource& After, bool FlushImmediate = false);
	void FlushResourceBarriers();

	void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, ID3D12DescriptorHeap* heapPtr);
	void SetDescriptorHeaps(uint32_t heapCount, D3D12_DESCRIPTOR_HEAP_TYPE type[], ID3D12DescriptorHeap* heapPtrs[]);

	void PIXBeginEvent(const std::string& label);
	void PIXEndEvent();
	void PIXSetMarker(const std::string& label);

private:
	uint64_t Finish(bool wait = false);
	void Reset();

protected:
	void BindDescriptorHeaps();

protected:
	CommandListManager*			m_owner{ nullptr };
	ID3D12GraphicsCommandList*	m_commandList{ nullptr };
	ID3D12CommandAllocator*		m_currentAllocator{ nullptr };

	ID3D12RootSignature*		m_currentGraphicsRootSignature{ nullptr };
	ID3D12PipelineState*		m_currentGraphicsPSO{ nullptr };

	ID3D12RootSignature*		m_currentComputeRootSignature{ nullptr };
	ID3D12PipelineState*		m_currentComputePSO{ nullptr };

	DynamicDescriptorHeap		m_dynamicDescriptorHeap;

	D3D12_RESOURCE_BARRIER		m_resourceBarrierBuffer[16];
	uint32_t					m_numBarriersToFlush{ 0 };

	ID3D12DescriptorHeap*		m_currentDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

	LinearAllocator				m_cpuLinearAllocator;
	LinearAllocator				m_gpuLinearAllocator;

private:
	static CommandList* AllocateCommandList();
	static void FreeCommandList(CommandList* commandList);

private:
	static std::vector<std::unique_ptr<CommandList>>	s_commandListPool;
	static std::mutex									s_commandListAllocationMutex;
	static std::queue<CommandList*>						s_availableCommandLists;
};


class GraphicsCommandList : public CommandList
{
public:

	static GraphicsCommandList* Begin()
	{
		return CommandList::Begin()->GetGraphicsCommandList();
	}

	void ClearUAV(ColorBuffer& target);
	void ClearUAV(ColorBuffer& target, const DirectX::XMVECTORF32& clearColor);
	void ClearUAV(ColorBuffer& target, const DirectX::XMVECTORU32& clearValue);
	void ClearColor(ColorBuffer& target);
	void ClearColor(ColorBuffer& target, const DirectX::XMVECTORF32& clearColor);
	void ClearDepth(DepthBuffer& target);
	void ClearDepth(DepthBuffer& target, float clearDepth);
	void ClearStencil(DepthBuffer& target);
	void ClearStencil(DepthBuffer& target, uint32_t clearStencil);
	void ClearDepthAndStencil(DepthBuffer& target);
	void ClearDepthAndStencil(DepthBuffer& target, float clearDepth, uint32_t clearStencil);

	void SetRootSignature(const RootSignature& rootSig);

	void SetRenderTargets(uint32_t numRTVs, ColorBuffer* rtvs, DepthBuffer* dsv = nullptr, bool readOnlyDepth = false);
	void SetRenderTarget(ColorBuffer& rtv) { SetRenderTargets(1, &rtv); }
	void SetRenderTarget(ColorBuffer& rtv, DepthBuffer& dsv, bool readOnlyDepth = false)
	{
		SetRenderTargets(1, &rtv, &dsv, readOnlyDepth);
	}
	void SetDepthStencilTarget(DepthBuffer& dsv) { SetRenderTargets(0, nullptr, &dsv); }

	void SetViewport(const Viewport& vp);
	void SetViewport(float x, float y, float w, float h, float minDepth = 0.0f, float maxDepth = 1.0f);
	void SetScissor(const Rectangle& rect);
	void SetScissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom);
	void SetViewportAndScissor(const Viewport& vp, const Rectangle& rect);
	void SetViewportAndScissor(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
	void SetStencilRef(uint32_t stencilRef);
	void SetBlendFactor(const DirectX::XMVECTORF32& blendFactor);
	void SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY topology);

	void SetPipelineState(const GraphicsPSO& PSO);
	void SetConstantBuffer(uint32_t rootIndex, const ConstantBuffer& cbuffer);

	byte* MapConstants(ConstantBuffer& cbuffer);
	void UnmapConstants(const ConstantBuffer& cbuffer) {}

	void SetIndexBuffer(const IndexBuffer& indexBuffer, uint32_t offset = 0);
	void SetVertexBuffer(uint32_t slot, const VertexBuffer& vertexBuffer, uint32_t offset = 0)
	{
		SetVertexBuffers(1, slot, &vertexBuffer, &offset);
	}
	void SetVertexBuffers(uint32_t numVBs, uint32_t startSlot, const VertexBuffer* vertexBuffers, uint32_t* offsets);

	void SetDynamicDescriptor(uint32_t rootIndex, uint32_t offset, D3D12_CPU_DESCRIPTOR_HANDLE handle);
	void SetDynamicDescriptors(uint32_t rootIndex, uint32_t offset, uint32_t count, const D3D12_CPU_DESCRIPTOR_HANDLE handles[]);

	void Draw(uint32_t vertexCount, uint32_t vertexStartOffset = 0);
	void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation = 0, int32_t BaseVertexLocation = 0);
	void DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount,
		uint32_t startVertexLocation = 0, uint32_t startInstanceLocation = 0);
	void DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation,
		int32_t baseVertexLocation, uint32_t startInstanceLocation);
	//void DrawIndirect(GpuBuffer& argumentBuffer, size_t argumentBufferOffset = 0);
};


class ComputeCommandList : public CommandList
{
public:

	static ComputeCommandList* Begin()
	{
		return CommandList::Begin()->GetComputeCommandList();
	}

	void ClearUAV(ColorBuffer& target);
	void ClearUAV(ColorBuffer& target, const DirectX::XMVECTORF32& clearColor);
	void ClearUAV(ColorBuffer& target, const DirectX::XMVECTORU32& clearValue);

	void SetRootSignature(const RootSignature& rootSig);

	void SetPipelineState(const ComputePSO& pso);
	void SetConstants(uint32_t rootIndex, uint32_t numConstants, const void* constants);
	void SetConstants(uint32_t rootIndex, DWParam x);
	void SetConstants(uint32_t rootIndex, DWParam x, DWParam y);
	void SetConstants(uint32_t rootIndex, DWParam x, DWParam y, DWParam z);
	void SetConstants(uint32_t rootIndex, DWParam x, DWParam y, DWParam z, DWParam w);
	void SetConstantBuffer(uint32_t rootIndex, D3D12_GPU_VIRTUAL_ADDRESS cbv);
	void SetDynamicConstantBufferView(uint32_t rootIndex, size_t bufferSize, const void* bufferData);
	void SetDynamicSRV(uint32_t rootIndex, size_t bufferSize, const void* bufferData);
	void SetBufferSRV(uint32_t rootIndex, const GpuBuffer& srv);
	void SetBufferUAV(uint32_t rootIndex, const GpuBuffer& uav);
	void SetDescriptorTable(uint32_t rootIndex, D3D12_GPU_DESCRIPTOR_HANDLE firstHandle);

	void SetDynamicDescriptor(uint32_t rootIndex, uint32_t offset, D3D12_CPU_DESCRIPTOR_HANDLE handle);
	void SetDynamicDescriptors(uint32_t rootIndex, uint32_t offset, uint32_t count, const D3D12_CPU_DESCRIPTOR_HANDLE handles[]);

	void Dispatch(size_t groupCountX = 1, size_t groupCountY = 1, size_t groupCountZ = 1);
	void Dispatch1D(size_t threadCountX, size_t groupSizeX = 64);
	void Dispatch2D(size_t threadCountX, size_t threadCountY, size_t groupSizeX = 8, size_t groupSizeY = 8);
	void Dispatch3D(size_t threadCountX, size_t threadCountY, size_t threadCountZ, size_t groupSizeX, size_t groupSizeY, size_t groupSizeZ);
};


inline void CommandList::CopyBuffer(GpuResource& dest, GpuResource& src)
{
	TransitionResource(dest, ResourceState::CopyDest);
	TransitionResource(src, ResourceState::CopySource);
	FlushResourceBarriers();
	m_commandList->CopyResource(dest.GetResource(), src.GetResource());
}


inline void CommandList::CopyBufferRegion(GpuResource& dest, size_t destOffset, GpuResource& src, size_t srcOffset, size_t numBytes)
{
	TransitionResource(dest, ResourceState::CopyDest);
	TransitionResource(src, ResourceState::CopySource);
	FlushResourceBarriers();
	m_commandList->CopyBufferRegion(dest.GetResource(), destOffset, src.GetResource(), srcOffset, numBytes);
}


inline void CommandList::SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, ID3D12DescriptorHeap* heapPtr)
{
	if (m_currentDescriptorHeaps[type] != heapPtr)
	{
		m_currentDescriptorHeaps[type] = heapPtr;
		BindDescriptorHeaps();
	}
}


inline void CommandList::SetDescriptorHeaps(uint32_t heapCount, D3D12_DESCRIPTOR_HEAP_TYPE type[], ID3D12DescriptorHeap* heapPtrs[])
{
	bool anyChanged = false;

	for(uint32_t i = 0; i < heapCount; ++i)
	{
		if(m_currentDescriptorHeaps[type[i]] != heapPtrs[i])
		{
			m_currentDescriptorHeaps[type[i]] = heapPtrs[i];
			anyChanged = true;
		}
	}

	if (anyChanged)
	{
		BindDescriptorHeaps();
	}
}


inline void GraphicsCommandList::SetDynamicDescriptor(uint32_t rootIndex, uint32_t offset, D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
	SetDynamicDescriptors(rootIndex, offset, 1, &handle);
}


inline void GraphicsCommandList::SetDynamicDescriptors(uint32_t rootIndex, uint32_t offset, uint32_t count, 
	const D3D12_CPU_DESCRIPTOR_HANDLE handles[])
{
	m_dynamicDescriptorHeap.SetGraphicsDescriptorHandles(rootIndex, offset, count, handles);
}


inline void GraphicsCommandList::SetRootSignature(const RootSignature& rootSig)
{
	if(rootSig.GetSignature() == m_currentGraphicsRootSignature)
	{
		return;
	}

	m_commandList->SetGraphicsRootSignature(rootSig.GetSignature());
	m_currentGraphicsRootSignature = rootSig.GetSignature();

	m_dynamicDescriptorHeap.ParseGraphicsRootSignature(rootSig);
}


inline void GraphicsCommandList::SetStencilRef(uint32_t stencilRef)
{
	m_commandList->OMSetStencilRef(stencilRef);
}


inline void GraphicsCommandList::SetBlendFactor(const DirectX::XMVECTORF32& blendFactor)
{
	m_commandList->OMSetBlendFactor(blendFactor);
}


inline void GraphicsCommandList::SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY Topology)
{
	m_commandList->IASetPrimitiveTopology(Topology);
}


inline void GraphicsCommandList::Draw(uint32_t vertexCount, uint32_t vertexStartOffset)
{
	DrawInstanced(vertexCount, 1, vertexStartOffset, 0);
}


inline void GraphicsCommandList::DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation)
{
	DrawIndexedInstanced(indexCount, 1, startIndexLocation, baseVertexLocation, 0);
}


inline void GraphicsCommandList::DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount,
	uint32_t startVertexLocation, uint32_t startInstanceLocation)
{
	FlushResourceBarriers();
	m_dynamicDescriptorHeap.CommitGraphicsRootDescriptorTables(m_commandList);
	m_commandList->DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
}


inline void GraphicsCommandList::DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation,
	int32_t baseVertexLocation, uint32_t startInstanceLocation)
{
	FlushResourceBarriers();
	m_dynamicDescriptorHeap.CommitGraphicsRootDescriptorTables(m_commandList);
	m_commandList->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
}


inline void ComputeCommandList::SetRootSignature(const RootSignature& rootSig)
{
	if (rootSig.GetSignature() == m_currentComputeRootSignature)
	{
		return;
	}

	m_commandList->SetComputeRootSignature(m_currentComputeRootSignature = rootSig.GetSignature());

	m_dynamicDescriptorHeap.ParseComputeRootSignature(rootSig);
}


inline void ComputeCommandList::SetConstants(uint32_t rootEntry, uint32_t numConstants, const void* constants)
{
	m_commandList->SetComputeRoot32BitConstants(rootEntry, numConstants, constants, 0);
}


inline void ComputeCommandList::SetConstants(uint32_t rootEntry, DWParam x)
{
	m_commandList->SetComputeRoot32BitConstant(rootEntry, x.Uint, 0);
}


inline void ComputeCommandList::SetConstants(uint32_t rootEntry, DWParam x, DWParam y)
{
	m_commandList->SetComputeRoot32BitConstant(rootEntry, x.Uint, 0);
	m_commandList->SetComputeRoot32BitConstant(rootEntry, y.Uint, 1);
}


inline void ComputeCommandList::SetConstants(uint32_t rootEntry, DWParam x, DWParam y, DWParam z)
{
	m_commandList->SetComputeRoot32BitConstant(rootEntry, x.Uint, 0);
	m_commandList->SetComputeRoot32BitConstant(rootEntry, y.Uint, 1);
	m_commandList->SetComputeRoot32BitConstant(rootEntry, z.Uint, 2);
}


inline void ComputeCommandList::SetConstants(uint32_t rootEntry, DWParam x, DWParam y, DWParam z, DWParam w)
{
	m_commandList->SetComputeRoot32BitConstant(rootEntry, x.Uint, 0);
	m_commandList->SetComputeRoot32BitConstant(rootEntry, y.Uint, 1);
	m_commandList->SetComputeRoot32BitConstant(rootEntry, z.Uint, 2);
	m_commandList->SetComputeRoot32BitConstant(rootEntry, w.Uint, 3);
}


inline void ComputeCommandList::SetConstantBuffer(uint32_t rootIndex, D3D12_GPU_VIRTUAL_ADDRESS cbv)
{
	m_commandList->SetComputeRootConstantBufferView(rootIndex, cbv);
}


inline void ComputeCommandList::SetDynamicConstantBufferView(uint32_t rootIndex, size_t bufferSize, const void* bufferData)
{
	assert(bufferData != nullptr && Math::IsAligned(bufferData, 16));
	DynAlloc cb = m_cpuLinearAllocator.Allocate(bufferSize);
	//SIMDMemCopy(cb.DataPtr, BufferData, Math::AlignUp(BufferSize, 16) >> 4);
	memcpy(cb.dataPtr, bufferData, bufferSize);
	m_commandList->SetComputeRootConstantBufferView(rootIndex, cb.gpuAddress);
}


inline void ComputeCommandList::SetDynamicSRV(uint32_t rootIndex, size_t bufferSize, const void* bufferData)
{
	assert(bufferData != nullptr && Math::IsAligned(bufferData, 16));
	DynAlloc cb = m_cpuLinearAllocator.Allocate(bufferSize);
	SIMDMemCopy(cb.dataPtr, bufferData, Math::AlignUp(bufferSize, 16) >> 4);
	m_commandList->SetComputeRootShaderResourceView(rootIndex, cb.gpuAddress);
}


inline void ComputeCommandList::SetBufferSRV(uint32_t rootIndex, const GpuBuffer& srv)
{
	assert((srv.m_usageState & D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE) != 0);
	m_commandList->SetComputeRootShaderResourceView(rootIndex, srv.GetGpuVirtualAddress());
}


inline void ComputeCommandList::SetBufferUAV(uint32_t rootIndex, const GpuBuffer& uav)
{
	assert((uav.m_usageState & D3D12_RESOURCE_STATE_UNORDERED_ACCESS) != 0);
	m_commandList->SetComputeRootUnorderedAccessView(rootIndex, uav.GetGpuVirtualAddress());
}


inline void ComputeCommandList::SetDescriptorTable(uint32_t rootIndex, D3D12_GPU_DESCRIPTOR_HANDLE firstHandle)
{
	m_commandList->SetComputeRootDescriptorTable(rootIndex, firstHandle);
}


inline void ComputeCommandList::SetDynamicDescriptor(uint32_t rootIndex, uint32_t offset, D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
	SetDynamicDescriptors(rootIndex, offset, 1, &handle);
}


inline void ComputeCommandList::SetDynamicDescriptors(uint32_t rootIndex, uint32_t offset, uint32_t count, const D3D12_CPU_DESCRIPTOR_HANDLE handles[])
{
	m_dynamicDescriptorHeap.SetComputeDescriptorHandles(rootIndex, offset, count, handles);
}


inline void ComputeCommandList::Dispatch(size_t groupCountX, size_t groupCountY, size_t groupCountZ)
{
	FlushResourceBarriers();
	m_dynamicDescriptorHeap.CommitComputeRootDescriptorTables(m_commandList);
	m_commandList->Dispatch((UINT)groupCountX, (UINT)groupCountY, (UINT)groupCountZ);
}


inline void ComputeCommandList::Dispatch1D(size_t threadCountX, size_t groupSizeX)
{
	Dispatch(Math::DivideByMultiple(threadCountX, groupSizeX), 1, 1);
}


inline void ComputeCommandList::Dispatch2D(size_t threadCountX, size_t threadCountY, size_t groupSizeX, size_t groupSizeY)
{
	Dispatch(
		Math::DivideByMultiple(threadCountX, groupSizeX),
		Math::DivideByMultiple(threadCountY, groupSizeY), 1);
}


inline void ComputeCommandList::Dispatch3D(size_t threadCountX, size_t threadCountY, size_t threadCountZ, size_t groupSizeX, size_t groupSizeY, size_t groupSizeZ)
{
	Dispatch(
		Math::DivideByMultiple(threadCountX, groupSizeX),
		Math::DivideByMultiple(threadCountY, groupSizeY),
		Math::DivideByMultiple(threadCountZ, groupSizeZ));
}

} // namespace Kodiak