// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

namespace Kodiak
{

// Forward declarations
class ColorBuffer;
class CommandListManager;
class ComputeCommandList;
class ConstantBuffer;
class DepthBuffer;
class GraphicsCommandList;
class GraphicsPSO;
class IndexBuffer;
class RenderTargetView;
class VertexBuffer;
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

protected:
	CommandListManager*			m_owner{ nullptr };
	ID3D11DeviceContext*		m_context{ nullptr };
	ID3D11DeviceContext1*		m_context1{ nullptr };

	// Current state caching
	uint32_t m_currentStencilRef{ 0 };
	DirectX::XMVECTORF32 m_currentBlendFactor;
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_currentBlendState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_currentDepthStencilState;
	GraphicsPSO* m_currentGraphicsPSO{ nullptr };

private:
	static CommandList* AllocateCommandList();
	static void FreeCommandList(CommandList* commandList);
	void Reset();

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

	void SetRenderTargets(uint32_t numRTVs, ColorBuffer* rtvs, DepthBuffer* dsv = nullptr, bool readOnlyDepth = false);
	void SetRenderTarget(ColorBuffer& rtv) { SetRenderTargets(1, &rtv); }
	void SetRenderTarget(ColorBuffer& rtv, DepthBuffer& dsv, bool readOnlyDepth = false)
	{
		SetRenderTargets(1, &rtv, &dsv, readOnlyDepth);
	}
	void SetDepthStencilTarget(DepthBuffer& dsv) { SetRenderTargets(0, nullptr, &dsv); }
	void UnbindRenderTargets();

	void SetViewport(const Viewport& vp);
	void SetViewport(float x, float y, float w, float h, float minDepth = 0.0f, float maxDepth = 1.0f);
	void SetScissor(const Rectangle& rect);
	void SetScissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom);
	void SetViewportAndScissor(const Viewport& vp, const Rectangle& rect);
	void SetViewportAndScissor(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
	void SetStencilRef(uint32_t stencilRef);
	void SetBlendFactor(const DirectX::XMVECTORF32& blendFactor);
	void SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY topology);

	void SetPipelineState(GraphicsPSO& PSO);

	byte* MapConstants(const ConstantBuffer& cbuffer);
	void UnmapConstants(const ConstantBuffer& cbuffer);

	void SetIndexBuffer(const IndexBuffer& indexBuffer, uint32_t offset = 0);
	void SetVertexBuffer(uint32_t slot, const VertexBuffer& vertexBuffer, uint32_t offset = 0)
	{
		SetVertexBuffers(1, slot, &vertexBuffer, &offset);
	}
	void SetVertexBuffers(uint32_t numVBs, uint32_t startSlot, const VertexBuffer* vertexBuffers, uint32_t* offsets);

	void Draw(uint32_t vertexCount, uint32_t vertexStartOffset = 0);
	void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation = 0, int32_t BaseVertexLocation = 0);
	void DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount,
		uint32_t startVertexLocation = 0, uint32_t startInstanceLocation = 0);
	void DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation,
		int32_t baseVertexLocation, uint32_t startInstanceLocation);
	//void DrawIndirect(GpuBuffer& argumentBuffer, size_t argumentBufferOffset = 0);

	void SetVertexShaderResource(uint32_t slot, ID3D11ShaderResourceView* srv);
	void SetVertexShaderResources(uint32_t startSlot, uint32_t numResources, ID3D11ShaderResourceView* const * srvs);
	void SetVertexShaderConstants(uint32_t slot, const ConstantBuffer& cbuffer);
	void SetVertexShaderConstants(uint32_t startSlot, uint32_t numBuffers, ID3D11Buffer* const * cbuffers, const uint32_t* firstConstant, 
		const uint32_t* numConstants);

	void SetDomainShaderResource(uint32_t slot, ID3D11ShaderResourceView* srv);
	void SetDomainShaderResources(uint32_t startSlot, uint32_t numResources, ID3D11ShaderResourceView* const * srvs);
	void SetDomainShaderConstants(uint32_t slot, const ConstantBuffer& cbuffer);
	void SetDomainShaderConstants(uint32_t startSlot, uint32_t numBuffers, ID3D11Buffer* const * cbuffers, const uint32_t* firstConstant,
		const uint32_t* numConstants);

	void SetHullShaderResource(uint32_t slot, ID3D11ShaderResourceView* srv);
	void SetHullShaderResources(uint32_t startSlot, uint32_t numResources, ID3D11ShaderResourceView* const * srvs);
	void SetHullShaderConstants(uint32_t slot, const ConstantBuffer& cbuffer);
	void SetHullShaderConstants(uint32_t startSlot, uint32_t numBuffers, ID3D11Buffer* const * cbuffers, const uint32_t* firstConstant,
		const uint32_t* numConstants);

	void SetGeometryShaderResource(uint32_t slot, ID3D11ShaderResourceView* srv);
	void SetGeometryShaderResources(uint32_t startSlot, uint32_t numResources, ID3D11ShaderResourceView* const * srvs);
	void SetGeometryShaderConstants(uint32_t slot, const ConstantBuffer& cbuffer);
	void SetGeometryShaderConstants(uint32_t startSlot, uint32_t numBuffers, ID3D11Buffer* const * cbuffers, const uint32_t* firstConstant,
		const uint32_t* numConstants);

	void SetPixelShaderResource(uint32_t slot, ID3D11ShaderResourceView* srv);
	void SetPixelShaderResources(uint32_t startSlot, uint32_t numResources, ID3D11ShaderResourceView* const * srvs);
	void SetPixelShaderConstants(uint32_t slot, const ConstantBuffer& cbuffer);
	void SetPixelShaderConstants(uint32_t startSlot, uint32_t numBuffers, ID3D11Buffer* const * cbuffers, const uint32_t* firstConstant,
		const uint32_t* numConstants);
};


class ComputeCommandList : public CommandList
{
public:

	static ComputeCommandList& Begin()
	{
		return CommandList::Begin().GetComputeCommandList();
	}
};


inline void GraphicsCommandList::Draw(uint32_t vertexCount, uint32_t vertexStartOffset)
{
	m_context->Draw(vertexCount, vertexStartOffset);
}


inline void GraphicsCommandList::DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation)
{
	m_context->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
}


inline void GraphicsCommandList::DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount,
	uint32_t startVertexLocation, uint32_t startInstanceLocation)
{
	m_context->DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
}


inline void GraphicsCommandList::DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation,
	int32_t baseVertexLocation, uint32_t startInstanceLocation)
{
	m_context->DrawIndexedInstanced(
		indexCountPerInstance,
		instanceCount,
		startIndexLocation,
		baseVertexLocation,
		startInstanceLocation);
}


inline void GraphicsCommandList::SetVertexShaderResource(uint32_t slot, ID3D11ShaderResourceView* srv)
{
	m_context->VSSetShaderResources(slot, 1, &srv);
}


inline void GraphicsCommandList::SetVertexShaderResources(uint32_t startSlot, uint32_t numResources, ID3D11ShaderResourceView* const * srvs)
{
	m_context->VSSetShaderResources(startSlot, numResources, srvs);
}


inline void GraphicsCommandList::SetHullShaderResource(uint32_t slot, ID3D11ShaderResourceView* srv)
{
	m_context->HSSetShaderResources(slot, 1, &srv);
}


inline void GraphicsCommandList::SetHullShaderResources(uint32_t startSlot, uint32_t numResources, ID3D11ShaderResourceView* const * srvs)
{
	m_context->HSSetShaderResources(startSlot, numResources, srvs);
}


inline void GraphicsCommandList::SetDomainShaderResource(uint32_t slot, ID3D11ShaderResourceView* srv)
{
	m_context->DSSetShaderResources(slot, 1, &srv);
}


inline void GraphicsCommandList::SetDomainShaderResources(uint32_t startSlot, uint32_t numResources, ID3D11ShaderResourceView* const * srvs)
{
	m_context->DSSetShaderResources(startSlot, numResources, srvs);
}


inline void GraphicsCommandList::SetGeometryShaderResource(uint32_t slot, ID3D11ShaderResourceView* srv)
{
	m_context->GSSetShaderResources(slot, 1, &srv);
}


inline void GraphicsCommandList::SetGeometryShaderResources(uint32_t startSlot, uint32_t numResources, ID3D11ShaderResourceView* const * srvs)
{
	m_context->GSSetShaderResources(startSlot, numResources, srvs);
}


inline void GraphicsCommandList::SetPixelShaderResource(uint32_t slot, ID3D11ShaderResourceView* srv)
{
	m_context->PSSetShaderResources(slot, 1, &srv);
}


inline void GraphicsCommandList::SetPixelShaderResources(uint32_t startSlot, uint32_t numResources, ID3D11ShaderResourceView* const * srvs)
{
	m_context->PSSetShaderResources(startSlot, numResources, srvs);
}


} // namespace Kodiak