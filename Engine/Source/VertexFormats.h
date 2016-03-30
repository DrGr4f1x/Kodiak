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

struct VertexPosition
{
	Math::Vector3 pos;
};

struct VertexPositionNormal
{
	Math::Vector3 pos;
	Math::Vector3 normal;
};

struct VertexPositionColor
{
	Math::Vector3 pos;
	Math::Vector3 color;
};

struct VertexPositionNormalColor
{
	Math::Vector3 pos;
	Math::Vector3 normal;
	Math::Vector3 color;
};

} // namespace Kodiak
