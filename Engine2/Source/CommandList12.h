#pragma once

namespace Kodiak
{

class RenderTargetView;

class CommandList
{
public:
	void Begin();
	void End();

	void SynchronizeRenderTargetViewForRendering(const std::shared_ptr<RenderTargetView>& rtv);
	void SynchronizeRenderTargetViewForPresent(const std::shared_ptr<RenderTargetView>& rtv);

	void ClearRenderTargetView(const std::shared_ptr<RenderTargetView>& rtv, const float* color);

public:
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_allocator;
};

} // namespace Kodiak
