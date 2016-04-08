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

enum class Usage
{
	Immutable = D3D11_USAGE_IMMUTABLE,
	Dynamic =	D3D11_USAGE_DYNAMIC,
	Staging =	D3D11_USAGE_STAGING,
	Default =	D3D11_USAGE_DEFAULT
};


enum class Blend
{
	Zero =				D3D11_BLEND_ZERO,
	One =				D3D11_BLEND_ONE,
	SrcColor =			D3D11_BLEND_SRC_COLOR,
	InvSrcColor =		D3D11_BLEND_INV_SRC_COLOR,
	SrcAlpha =			D3D11_BLEND_SRC_ALPHA,
	InvSrcAlpha =		D3D11_BLEND_INV_SRC_ALPHA,
	DstAlpha =			D3D11_BLEND_DEST_ALPHA,
	InvDstAlpha =		D3D11_BLEND_INV_DEST_ALPHA,
	DstColor =			D3D11_BLEND_DEST_COLOR,
	InvDstColor =		D3D11_BLEND_INV_DEST_COLOR,
	SrcAlphaSat =		D3D11_BLEND_SRC_ALPHA_SAT,
	BlendFactor =		D3D11_BLEND_BLEND_FACTOR,
	InvBlendFactor =	D3D11_BLEND_INV_BLEND_FACTOR,
	Src1Color =			D3D11_BLEND_SRC1_COLOR,
	InvSrc1Color =		D3D11_BLEND_INV_SRC1_COLOR,
	Src1Alpha =			D3D11_BLEND_SRC1_ALPHA,
	InvSrc1Alpha =		D3D11_BLEND_INV_SRC1_ALPHA
};


enum class BlendOp
{
	Add =			D3D11_BLEND_OP_ADD,
	Subtract =		D3D11_BLEND_OP_SUBTRACT,
	RevSubtract =	D3D11_BLEND_OP_REV_SUBTRACT,
	Min =			D3D11_BLEND_OP_MIN,
	Max =			D3D11_BLEND_OP_MAX
};


enum class ColorWrite
{
	Red = 1,
	Green = 2,
	Blue = 4,
	Alpha = 8,
	All = Red | Green | Blue | Alpha
};


enum class CullMode
{
	None =		D3D11_CULL_NONE,
	Front =		D3D11_CULL_FRONT,
	Back =		D3D11_CULL_BACK
};


enum class FillMode
{
	Wireframe = D3D11_FILL_WIREFRAME,
	Solid =		D3D11_FILL_SOLID
};


enum class DepthWrite
{
	Zero =	D3D11_DEPTH_WRITE_MASK_ZERO,
	All =	D3D11_DEPTH_WRITE_MASK_ALL
};


enum class ComparisonFunc
{
	Never =			D3D11_COMPARISON_NEVER,
	Less =			D3D11_COMPARISON_LESS,
	Equal =			D3D11_COMPARISON_EQUAL,
	LessEqual =		D3D11_COMPARISON_LESS_EQUAL,
	Greater =		D3D11_COMPARISON_GREATER,
	NotEqual =		D3D11_COMPARISON_NOT_EQUAL,
	GreaterEqual =	D3D11_COMPARISON_GREATER_EQUAL,
	Always =		D3D11_COMPARISON_ALWAYS
};


enum class StencilOp
{
	Keep =		D3D11_STENCIL_OP_KEEP,
	Zero =		D3D11_STENCIL_OP_ZERO,
	Replace =	D3D11_STENCIL_OP_REPLACE,
	IncrSat =	D3D11_STENCIL_OP_INCR_SAT,
	DecrSat =	D3D11_STENCIL_OP_DECR_SAT,
	Invert =	D3D11_STENCIL_OP_INVERT,
	Incr =		D3D11_STENCIL_OP_INCR,
	Decr =		D3D11_STENCIL_OP_DECR
};


enum class PrimitiveTopologyType
{
	Undefined,
	Point,
	Line,
	Triangle,
	Patch 
};


enum class ResourceState
{
	Common,
	VertexAndConstantBuffer,
	IndexBuffer,
	RenderTarget,
	UnorderedAccess,
	DepthWrite,
	DepthRead,
	NonPixelShaderResource,
	PixelShaderResource,
	StreamOut,
	IndirectArgument,
	CopyDest,
	CopySource,
	ResolveDest,
	ResolveSource,
	GenericRead,
	Present,
	Predication
};

} // namespace Kodiak
