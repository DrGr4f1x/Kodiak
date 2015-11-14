// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from PipelineState.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#pragma once

namespace Kodiak
{

// Forward declarations
class CommandList;
class ComputeShader;
class DomainShader;
class GeometryShader;
class HullShader;
class InputLayout;
class PixelShader;
class RootSignature;
class VertexShader;
enum class Blend;
enum class BlendOp;
enum class ColorFormat;
enum class ColorWrite;
enum class ComparisonFunc;
enum class CullMode;
enum class DepthFormat;
enum class DepthWrite;
enum class FillMode;
enum class LogicOp;
enum class PrimitiveTopologyType;
enum class StencilOp;
enum class IndexBufferStripCutValue;


struct RenderTargetBlendDesc
{
	RenderTargetBlendDesc();
	RenderTargetBlendDesc(Blend srcBlend, Blend dstBlend);

	bool blendEnable;
	bool logicOpEnable;
	Blend		srcBlend;
	Blend		dstBlend;
	BlendOp		blendOp;
	Blend		srcBlendAlpha;
	Blend		dstBlendAlpha;
	BlendOp		blendOpAlpha;
	LogicOp		logicOp;
	ColorWrite	writeMask;
};


struct BlendStateDesc
{
	BlendStateDesc();
	BlendStateDesc(Blend srcBlend, Blend dstBlend);

	bool alphaToCoverageEnable;
	bool independentBlendEnable;
	RenderTargetBlendDesc renderTargetBlend[8];
};


struct RasterizerStateDesc
{
	RasterizerStateDesc();
	RasterizerStateDesc(CullMode cullMode, FillMode fillMode);

	CullMode	cullMode;
	FillMode	fillMode;
	bool		frontCounterClockwise;
	int32_t		depthBias;
	float		depthBiasClamp;
	float		slopeScaledDepthBias;
	bool		depthClipEnable;
	bool		multisampleEnable;
	bool		antialiasedLineEnable;
	uint32_t	forcedSampleCount;
	bool		conservativeRasterizationEnable;
};


struct StencilOpDesc
{
	StencilOpDesc();

	StencilOp		stencilFailOp;
	StencilOp		stencilDepthFailOp;
	StencilOp		stencilPassOp;
	ComparisonFunc	stencilFunc;
};


struct DepthStencilStateDesc
{
	DepthStencilStateDesc();
	DepthStencilStateDesc(bool enable, bool writeEnable);

	bool			depthEnable;
	DepthWrite		depthWriteMask;
	ComparisonFunc	depthFunc;
	bool			stencilEnable;
	uint8_t			stencilReadMask;
	uint8_t			stencilWriteMask;
	StencilOpDesc	frontFace;
	StencilOpDesc	backFace;
};


class PSO
{
public:
	PSO() : m_rootSignature(nullptr) {}

	static void DestroyAll();

	void SetRootSignature(const RootSignature& bindMappings)
	{
		m_rootSignature = &bindMappings;
	}

	const RootSignature& GetRootSignature() const
	{
		assert(m_rootSignature != nullptr);
		return *m_rootSignature;
	}

	ID3D12PipelineState* GetPipelineStateObject() const { return m_pso; }

protected:
	const RootSignature* m_rootSignature;
	ID3D12PipelineState* m_pso;
};


class GraphicsPSO : public PSO
{
	friend class CommandList;

public:
	// Start with empty state
	GraphicsPSO();

	void SetBlendState(const BlendStateDesc& blendDesc);
	void SetRasterizerState(const RasterizerStateDesc& rasterizerDesc);
	void SetDepthStencilState(const DepthStencilStateDesc& depthStencilDesc);
	void SetSampleMask(uint32_t sampleMask);
	void SetPrimitiveTopology(PrimitiveTopologyType topology);
	void SetRenderTargetFormat(ColorFormat colorFormat, DepthFormat depthFormat, uint32_t msaaCount = 1, uint32_t msaaQuality = 0);
	void SetRenderTargetFormats(uint32_t numRTVs, const ColorFormat* colorFormats, DepthFormat depthFormat, uint32_t msaaCount = 1, 
		uint32_t msaaQuality = 0);
	void SetInputLayout(const InputLayout& inputLayout);
	void SetPrimitiveRestart(IndexBufferStripCutValue ibProps);

	void SetVertexShader(VertexShader* vertexShader);
	void SetHullShader(HullShader* hullShader);
	void SetDomainShader(DomainShader* domainShader);
	void SetGeometryShader(GeometryShader* geometryShader);
	void SetPixelShader(PixelShader* pixelShader);

	void Finalize();

private:
	D3D12_GRAPHICS_PIPELINE_STATE_DESC m_psoDesc;
	std::shared_ptr<const D3D12_INPUT_ELEMENT_DESC> m_inputLayouts;
};


class ComputePSO : public PSO
{
	friend class CommandList;

public:
	// Start with empty state
	ComputePSO();

	void SetComputeShader(ComputeShader* computeShader);

	void Finalize();

private:
	D3D12_COMPUTE_PIPELINE_STATE_DESC m_psoDesc;
};

} // namespace Kodiak