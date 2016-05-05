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
namespace RenderThread { class Camera; }
class Scene;


// Main thread view of the camera
class Camera : public std::enable_shared_from_this<Camera>
{
public:
	Camera();
	
	Camera(const Math::Vector3& position, const Math::Quaternion& orientation);
	Camera(const Math::Vector3& position, const Math::Quaternion& orientation, float fov, float aspect,
		float zNear = 0.1f, float zFar = 10.0f);

	void SetPosition(const Math::Vector3& position);
	void SetOrientation(const Math::Quaternion& orientation);
	void SetPositionAndOrientation(const Math::Vector3& position, const Math::Quaternion& orientation);

	const Math::Vector3& GetPosition() const { return m_position; }
	const Math::Quaternion& GetOrientation() const { return m_orientation; }
	const Math::Vector3& GetForwardVector() const { return m_forward; }
	const Math::Vector3& GetRightVector() const { return m_right; }

	void SetPerspective(float fov, float aspect, float zNear, float zFar);
	void SetAspectRatio(float aspect);
	void SetFOV(float fov);

	void GetPerspective(float& fov, float& aspect, float& zNear, float& zFar)
	{
		fov = m_fov;
		aspect = m_aspect;
		zNear = m_zNear;
		zFar = m_zFar;
	}
	
	void LookAt(const Math::Vector3& target, const Math::Vector3& up);
	void LookIn(const Math::Vector3& dir, const Math::Vector3& up);

	void Orbit(float deltaPitch, float deltaYaw, const Math::Vector3& target, const Math::Vector3& up);
	void Strafe(const Math::Vector3& strafe);

	void Rotate(float deltaPitch, float deltaYaw);

	std::shared_ptr<RenderThread::Camera> GetRenderThreadData() { return m_cameraProxy; }

private:
	void RenderThreadSetCameraPerspective();
	void RenderThreadSetCameraPositionAndOrientation();
	void CreateCameraProxy();

private:
	Math::Vector3		m_position;
	Math::Quaternion	m_orientation;
	Math::Vector3		m_forward;
	Math::Vector3		m_right;
	float				m_fov;
	float				m_aspect;
	float				m_zNear;
	float				m_zFar;

	std::shared_ptr<RenderThread::Camera> m_cameraProxy;
};


// Render thread classes and functions
namespace RenderThread
{

class Camera
{
public:
	Camera();

	void SetPositionAndOrientation(const Math::Vector3& position, const Math::Quaternion& orientation);
	void SetPerspective(float fov, float aspect, float zNear, float zFar);

	const Math::Matrix4& GetProjectionMatrix() const { return m_projectionMatrix; }
	const Math::Matrix4& GetViewMatrix() const { return m_viewMatrix; }
	const Math::Matrix4& GetPrevViewMatrix() const { return m_prevViewMatrix; }
	const Math::Vector3& GetPosition() const { return m_position; }
	const float GetZFar() const { return m_zFar; }
	const float GetZNear() const { return m_zNear; }

private:
	void UpdateViewMatrix();

private:
	Math::Matrix4			m_projectionMatrix;
	Math::Matrix4			m_viewMatrix;
	Math::Matrix4			m_prevViewMatrix;
	Math::Vector3			m_position;
	Math::Quaternion		m_orientation;

	float					m_fov;
	float					m_aspect;
	float					m_zNear;
	float					m_zFar;
};

} // namespace RenderThread

} // namespace Kodiak