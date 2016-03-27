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
class ComputeShader;
class DomainShader;
class GeometryShader;
class HullShader;
class InputLayout;
class PixelShader;
class VertexShader;
enum class Blend;
enum class BlendOp;
enum class ColorWrite;
enum class ComparisonFunc;
enum class CullMode;
enum class DepthWrite;
enum class FillMode;
enum class StencilOp;


struct RenderTargetBlendDesc
{
	RenderTargetBlendDesc();
	RenderTargetBlendDesc(Blend srcBlend, Blend dstBlend);
	
	bool		blendEnable;
	Blend		srcBlend;
	Blend		dstBlend;
	BlendOp		blendOp;
	Blend		srcBlendAlpha;
	Blend		dstBlendAlpha;
	BlendOp		blendOpAlpha;
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
	bool		scissorEnable;
	bool		multisampleEnable;
	bool		antialiasedLineEnable;
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


struct GraphicsPSODesc
{
	D3D11_BLEND_DESC			blendDesc;
	D3D11_RASTERIZER_DESC		rasterizerDesc;
	D3D11_DEPTH_STENCIL_DESC	depthStencilDesc;
	ID3D11InputLayout*			inputLayout{ nullptr };
	ID3D11VertexShader*			vertexShader{ nullptr };
	ID3D11HullShader*			hullShader{ nullptr };
	ID3D11DomainShader*			domainShader{ nullptr };
	ID3D11GeometryShader*		geometryShader{ nullptr };
	ID3D11PixelShader*			pixelShader{ nullptr };
};


class GraphicsPSO
{
	friend class GraphicsCommandList;

public:
	GraphicsPSO();

	void SetBlendState(const BlendStateDesc& blendDesc);
	void SetRasterizerState(const RasterizerStateDesc& rasterizerDesc);
	void SetDepthStencilState(const DepthStencilStateDesc& depthStencilDesc);

	void SetInputLayout(ID3D11InputLayout* inputLayout);

	void SetVertexShader(VertexShader* vertexShader);
	void SetHullShader(HullShader* hullShader);
	void SetDomainShader(DomainShader* domainShader);
	void SetGeometryShader(GeometryShader* geometryShader);
	void SetPixelShader(PixelShader* pixelShader);

	void Finalize();

private:
	void CompileBlendState();
	void CompileRasterizerState();
	void CompileDepthStencilState();

private:
	GraphicsPSODesc m_desc;

	ID3D11BlendState*			m_blendState{ nullptr };
	ID3D11RasterizerState*		m_rasterizerState{ nullptr };
	ID3D11DepthStencilState*	m_depthStencilState{ nullptr };

	Microsoft::WRL::ComPtr<ID3D11InputLayout>		m_inputLayout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>		m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11HullShader>		m_hullShader;
	Microsoft::WRL::ComPtr<ID3D11DomainShader>		m_domainShader;
	Microsoft::WRL::ComPtr<ID3D11GeometryShader>	m_geometryShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>		m_pixelShader;
};


class ComputePSO
{

};


} // namespace Kodiak