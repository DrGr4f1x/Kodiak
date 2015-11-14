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
#include "RootSignature12.h"

namespace Kodiak
{

// Forward declarations
class ColorBuffer;
class CommandListManager;
class ComputeCommandList;
class ComputePSO;
class DepthBuffer;
class GpuResource;
class GraphicsCommandList;
class GraphicsPSO;
class RenderTargetView;
class RootSignature;
struct Rectangle;
struct Viewport;


class CommandList
{
public:
	CommandList();
	virtual ~CommandList();

	static void DestroyAllCommandLists();
	static CommandList& Begin();
	uint64_t CloseAndExecute(bool waitForCompletion = false);

	void Initialize(CommandListManager& manager);

	GraphicsCommandList& GetGraphicsCommandList()
	{
		return reinterpret_cast<GraphicsCommandList&>(*this);
	}

	ComputeCommandList& GetComputeCommandList()
	{
		return reinterpret_cast<ComputeCommandList&>(*this);
	}

	void TransitionResource(GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = false);
	void BeginResourceTransition(GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = false);
	void InsertUAVBarrier(GpuResource& Resource, bool FlushImmediate = false);
	void InsertAliasBarrier(GpuResource& Before, GpuResource& After, bool FlushImmediate = false);
	void FlushResourceBarriers();

	void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, ID3D12DescriptorHeap* heapPtr);
	void SetDescriptorHeaps(uint32_t heapCount, D3D12_DESCRIPTOR_HEAP_TYPE type[], ID3D12DescriptorHeap* heapPtrs[]);

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

	DynamicDescriptorHeap		m_dynamicDescriptorHeap;

	D3D12_RESOURCE_BARRIER		m_resourceBarrierBuffer[16];
	uint32_t					m_numBarriersToFlush{ 0 };

	ID3D12DescriptorHeap*		m_currentDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

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

	static GraphicsCommandList& Begin()
	{
		return CommandList::Begin().GetGraphicsCommandList();
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

	static ComputeCommandList& Begin()
	{
		return CommandList::Begin().GetComputeCommandList();
	}
};


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


} // namespace Kodiak
