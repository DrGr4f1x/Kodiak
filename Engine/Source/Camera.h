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
	friend class Scene;

public:
	Camera();
	Camera(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT4& orientation);
	Camera(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT4& orientation, float fov, float aspect,
		float zNear = 0.1f, float zFar = 10.0f);

	const DirectX::XMFLOAT3& GetPosition() const { return m_position; }
	void SetPosition(const DirectX::XMFLOAT3& position);

	const DirectX::XMFLOAT4& GetOrientation() const { return m_orientation; }
	void SetOrientation(const DirectX::XMFLOAT4& orientation);

	void GetPerspective(float& fov, float& aspect, float& zNear, float& zFar)
	{
		fov = m_fov;
		aspect = m_aspect;
		zNear = m_zNear;
		zFar = m_zFar;
	}
	void SetPerspective(float fov, float aspect, float zNear, float zFar);
	void SetAspectRatio(float aspect);
	void SetFOV(float fov);

	void LookAt(const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up);
	void LookIn(const DirectX::XMFLOAT3& dir, const DirectX::XMFLOAT3& up);

	void Orbit(float deltaPitch, float deltaYaw, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up);
	void Rotate(float deltaPitch, float deltaYaw);
	void Strafe(const DirectX::XMFLOAT3& strafe);

private:
	void RenderThreadSetCameraPerspective();
	void RenderThreadSetCameraPositionAndOrientation();
	void CreateCameraProxy();

private:
	DirectX::XMFLOAT3	m_position;
	DirectX::XMFLOAT4	m_orientation;
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

	void SetPositionAndOrientation(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT4& orientation);
	void SetPerspective(float fov, float aspect, float zNear, float zFar);

	const DirectX::XMFLOAT4X4& GetProjectionMatrix() const { return m_projectionMatrix; }
	const DirectX::XMFLOAT4X4& GetViewMatrix() const { return m_viewMatrix; }
	const DirectX::XMFLOAT4X4& GetPrevViewMatrix() const { return m_prevViewMatrix; }
	const DirectX::XMFLOAT3& GetPosition() const { return m_position; }

private:
	void UpdateViewMatrix();

private:
	DirectX::XMFLOAT4X4		m_projectionMatrix;
	DirectX::XMFLOAT4X4		m_viewMatrix;
	DirectX::XMFLOAT4X4		m_prevViewMatrix;
	DirectX::XMFLOAT3		m_position;
	DirectX::XMFLOAT4		m_orientation;
	float					m_fov;
	float					m_aspect;
	float					m_zNear;
	float					m_zFar;
};

} // namespace RenderThread

} // namespace Kodiak