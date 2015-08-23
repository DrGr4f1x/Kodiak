#pragma once

namespace Kodiak
{

struct VertexPosition
{
	DirectX::XMFLOAT3 pos;
};

struct VertexPositionNormal
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
};

struct VertexPositionColor
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 color;
};

struct VertexPositionNormalColor
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT3 color;
};

} // namespace Kodiak
