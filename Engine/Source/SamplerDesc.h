// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "RenderEnums.h"

namespace Kodiak
{

struct SamplerDesc
{
	SamplerDesc()
	{
		filter = TextureFilter::Anisotropic;
		addressU = addressV = addressW = TextureAddress::Wrap;
		minLOD = 0.0f;
		maxLOD = Float32Max;
		mipLODBias = 0.0f;
		maxAnisotropy = 16;
		comparisonFunc = ComparisonFunc::LessEqual;
		borderColor[0] = 1.0f;
		borderColor[1] = 1.0f;
		borderColor[2] = 1.0f;
		borderColor[3] = 1.0f;
	}


	SamplerDesc(TextureFilter filter, TextureAddress address = TextureAddress::Clamp)
	{
		this->filter = filter;
		addressU = addressV = addressW = address;
		minLOD = 0.0f;
		maxLOD = Float32Max;
		mipLODBias = 0.0f;
		maxAnisotropy = 16;
		comparisonFunc = ComparisonFunc::LessEqual;
		borderColor[0] = 1.0f;
		borderColor[1] = 1.0f;
		borderColor[2] = 1.0f;
		borderColor[3] = 1.0f;
	}


	void SetBorderColor(const Color& color)
	{
		borderColor[0] = color.R();
		borderColor[1] = color.G();
		borderColor[2] = color.B();
		borderColor[3] = color.A();
	}


	TextureFilter filter;
	TextureAddress addressU;
	TextureAddress addressV;
	TextureAddress addressW;
	float mipLODBias;
	uint32_t maxAnisotropy;
	ComparisonFunc comparisonFunc;
	float borderColor[4];
	float minLOD;
	float maxLOD;
};

} // namespace Kodiak