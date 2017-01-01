// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from ShadowCamera.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#pragma once

#include "Camera.h"

namespace Kodiak
{

struct ShadowCameraProxy;

class ShadowCamera : public BaseCamera, public std::enable_shared_from_this<ShadowCamera>
{
public:

	ShadowCamera() 
	{
		m_proxy = std::make_shared<ShadowCameraProxy>();
	}

	void UpdateMatrix(
		Math::Vector3 LightDirection,		// Direction parallel to light, in direction of travel
		Math::Vector3 ShadowCenter,		// Center location on far bounding plane of shadowed region
		Math::Vector3 ShadowBounds,		// Width, height, and depth in world space represented by the shadow buffer
		uint32_t BufferWidth,		// Shadow buffer width
		uint32_t BufferHeight,		// Shadow buffer height--usually same as width
		uint32_t BufferPrecision	// Bit depth of shadow buffer--usually 16 or 24
	);

	// Used to transform world space to texture space for shadow sampling
	const Math::Matrix4& GetShadowMatrix() const { return m_ShadowMatrix; }

	ShadowCameraProxy* GetProxy() { return m_proxy.get(); }
	
private:
	Math::Matrix4 m_ShadowMatrix;
	std::shared_ptr<ShadowCameraProxy> m_proxy;
};

struct ShadowCameraProxy
{
	void CopyFromShadowCamera(ShadowCamera* camera);

	BaseCameraProxy		Base;
	Math::Matrix4		ShadowMatrix;
};

} // namespace Kodiak
