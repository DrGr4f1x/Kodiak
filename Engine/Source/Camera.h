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

class BaseCamera
{
public:

	// Call this function once per frame and after you've changed any state.  This
	// regenerates all matrices.  Calling it more or less than once per frame will break
	// temporal effects and cause unpredictable results.
	void Update();

	// Public functions for controlling where the camera is and its orientation
	void SetEyeAtUp(Math::Vector3 eye, Math::Vector3 at, Math::Vector3 up);
	void SetLookDirection(Math::Vector3 forward, Math::Vector3 up);
	void SetRotation(Math::Quaternion basisRotation);
	void SetPosition(Math::Vector3 worldPos);
	void SetTransform(const Math::AffineTransform& xform);
	
	const Math::Quaternion GetRotation() const { return m_cameraToWorld.GetRotation(); }
	const Math::Vector3 GetRightVec() const { return m_basis.GetX(); }
	const Math::Vector3 GetUpVec() const { return m_basis.GetY(); }
	const Math::Vector3 GetForwardVec() const { return -m_basis.GetZ(); }
	const Math::Vector3 GetPosition() const { return m_cameraToWorld.GetTranslation(); }

	// Accessors for reading the various matrices and frusta
	const Math::Matrix4& GetViewMatrix() const { return m_viewMatrix; }
	const Math::Matrix4& GetProjMatrix() const { return m_projMatrix; }
	const Math::Matrix4& GetViewProjMatrix() const { return m_viewProjMatrix; }
	const Math::Matrix4& GetReprojectionMatrix() const { return m_reprojectMatrix; }
	// TODO: make a frustum class
#if 0
	const Math::Frustum& GetViewSpaceFrustum() const { return m_frustumVS; }
	const Math::Frustum& GetWorldSpaceFrustum() const { return m_frustumWS; }
#endif

protected:

	BaseCamera() : m_cameraToWorld(Math::kIdentity), m_basis(Math::kIdentity) {}

	void SetProjMatrix(const Math::Matrix4& ProjMat) { m_projMatrix = ProjMat; }

	Math::OrthogonalTransform m_cameraToWorld;

	// Redundant data cached for faster lookups.
	Math::Matrix3 m_basis;

	// Transforms homogeneous coordinates from world space to view space.  In this case, view space is defined as +X is
	// to the right, +Y is up, and -Z is forward.  This has to match what the projection matrix expects, but you might
	// also need to know what the convention is if you work in view space in a shader.
	Math::Matrix4 m_viewMatrix;		// i.e. "World-to-View" matrix

	// The projection matrix transforms view space to clip space.  Once division by W has occurred, the final coordinates
	// can be transformed by the viewport matrix to screen space.  The projection matrix is determined by the screen aspect 
	// and camera field of view.  A projection matrix can also be orthographic.  In that case, field of view would be defined
	// in linear units, not angles.
	Math::Matrix4 m_projMatrix;		// i.e. "View-to-Projection" matrix

	// A concatenation of the view and projection matrices.
	Math::Matrix4 m_viewProjMatrix;	// i.e.  "World-To-Projection" matrix.

	// The view-projection matrix from the previous frame
	Math::Matrix4 m_previousViewProjMatrix;

	// Projects a clip-space coordinate to the previous frame (useful for temporal effects).
	Math::Matrix4 m_reprojectMatrix;

	// TODO: make a frustum class
#if 0
	Math::Frustum m_FrustumVS;		// View-space view frustum
	Math::Frustum m_FrustumWS;		// World-space view frustum
#endif
};


class Camera : public BaseCamera
{
public:
	Camera();

	// Controls the view-to-projection matrix
	void SetPerspectiveMatrix(float verticalFovRadians, float aspectHeightOverWidth, float nearZClip, float farZClip);
	void SetFOV(float verticalFovInRadians) { m_verticalFOV = verticalFovInRadians; UpdateProjMatrix(); }
	void SetAspectRatio(float heightOverWidth) { m_aspectRatio = heightOverWidth; UpdateProjMatrix(); }
	void SetZRange(float nearZ, float farZ) { m_nearClip = nearZ; m_farClip = farZ; UpdateProjMatrix(); }
	void ReverseZ(bool enable) { m_reverseZ = enable; UpdateProjMatrix(); }

	float GetFOV() const { return m_verticalFOV; }
	float GetNearClip() const { return m_nearClip; }
	float GetFarClip() const { return m_farClip; }
	float GetClearDepth() const { return m_reverseZ ? 0.0f : 1.0f; }

private:

	void UpdateProjMatrix();

	float m_verticalFOV;			// Field of view angle in radians
	float m_aspectRatio;
	float m_nearClip;
	float m_farClip;
	bool m_reverseZ;				// Invert near and far clip distances so that Z=0 is the far plane
};


inline void BaseCamera::SetEyeAtUp(Math::Vector3 eye, Math::Vector3 at, Math::Vector3 up)
{
	SetLookDirection(at - eye, up);
	SetPosition(eye);
}


inline void BaseCamera::SetPosition(Math::Vector3 worldPos)
{
	m_cameraToWorld.SetTranslation(worldPos);
}


inline void BaseCamera::SetTransform(const Math::AffineTransform& xform)
{
	// By using these functions, we rederive an orthogonal transform.
	SetLookDirection(-xform.GetZ(), xform.GetY());
	SetPosition(xform.GetTranslation());
}

inline void BaseCamera::SetRotation(Math::Quaternion basisRotation)
{
	m_cameraToWorld.SetRotation(Normalize(basisRotation));
	m_basis = Math::Matrix3(m_cameraToWorld.GetRotation());
}

inline Camera::Camera() : m_reverseZ(true)
{
	SetPerspectiveMatrix(DirectX::XM_PIDIV4, 9.0f / 16.0f, 1.0f, 1000.0f);
}

inline void Camera::SetPerspectiveMatrix(float verticalFovRadians, float aspectHeightOverWidth, float nearZClip, float farZClip)
{
	m_verticalFOV = verticalFovRadians;
	m_aspectRatio = aspectHeightOverWidth;
	m_nearClip = nearZClip;
	m_farClip = farZClip;

	UpdateProjMatrix();
}

} // namespace Kodiak