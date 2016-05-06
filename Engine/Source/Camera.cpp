// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "Camera.h"

#include "Renderer.h"
#include "Scene.h"


using namespace Kodiak;
using namespace Math;
using namespace std;
using namespace DirectX;


static bool Compare(const XMFLOAT3& a, const XMFLOAT3& b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

static bool Compare(const XMFLOAT4& a, const XMFLOAT4& b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}


Camera::Camera()
	: m_position(kZero)
	, m_orientation(kIdentity)
	, m_fov(60.0f)
	, m_aspect(1.0f)
	, m_zNear(0.1f)
	, m_zFar(10.0f)
	, m_reverseZ(false)
	, m_cameraProxy()
{
	CreateCameraProxy();
}


Camera::Camera(const Vector3& position, const Quaternion& orientation)
	: m_position(position)
	, m_orientation(orientation)
	, m_fov(60.0f)
	, m_aspect(1.0f)
	, m_zNear(0.1f)
	, m_zFar(10.0f)
	, m_reverseZ(false)
	, m_cameraProxy()
{
	CreateCameraProxy();
}



Camera::Camera(const Vector3& position, const Quaternion& orientation, float fov, float aspect, float zNear, float zFar)
	: m_position(position)
	, m_orientation(orientation)
	, m_fov(fov)
	, m_aspect(aspect)
	, m_zNear(zNear)
	, m_zFar(zFar)
	, m_cameraProxy()
{
	CreateCameraProxy();
}


void Camera::SetPosition(const Vector3& position)
{
	if (m_position.NotEqual(position))
	{
		m_position = position;
		RenderThreadSetCameraPositionAndOrientation();
	}
}


void Camera::SetOrientation(const Quaternion& orientation)
{
	if (m_orientation.NotEqual(orientation))
	{
		m_orientation = orientation;

		m_forward = m_orientation * Vector3(kNegZUnitVector);

		RenderThreadSetCameraPositionAndOrientation();
	}
}


void Camera::SetPositionAndOrientation(const Vector3& position, const Quaternion& orientation)
{
	bool update = false;

	if (m_position.NotEqual(position))
	{
		m_position = position;
		update = true;
	}

	if (m_orientation.NotEqual(orientation))
	{
		m_orientation = orientation;

		m_forward = m_orientation * Vector3(kNegZUnitVector);

		update = true;
	}

	if (update)
	{
		RenderThreadSetCameraPositionAndOrientation();
	}
}


void Camera::SetPerspective(float fov, float aspect, float zNear, float zFar)
{
	if (fov != m_fov || aspect != m_aspect || zNear != m_zNear || zFar != m_zFar)
	{
		m_fov = fov;
		m_aspect = aspect;
		m_zNear = zNear;
		m_zFar = zFar;
		RenderThreadSetCameraPerspective();
	}
}


void Camera::SetAspectRatio(float aspect)
{
	if (m_aspect != aspect)
	{
		m_aspect = aspect;
		RenderThreadSetCameraPerspective();
	}
}


void Camera::SetFOV(float fov)
{
	if (m_fov != fov)
	{
		m_fov = fov;
		RenderThreadSetCameraPerspective();
	}
}

void Camera::SetReverseZ(bool enable)
{
	if (m_reverseZ != enable)
	{
		m_reverseZ = enable;
		RenderThreadSetReverseZ();
	}
}


void Camera::LookAt(const Vector3& target, const Vector3& up)
{
	m_orientation = Normalize(Quaternion(Invert(Matrix4::LookAtRH(m_position, target, up))));

	m_forward = m_orientation * Vector3(kNegZUnitVector);
	m_right = m_orientation * Vector3(kXUnitVector);

	RenderThreadSetCameraPositionAndOrientation();
}


void Camera::LookIn(const Vector3& dir, const Vector3& up)
{
	m_orientation = Normalize(Quaternion(Invert(Matrix4::LookToRH(m_position, dir, up))));

	m_forward = m_orientation * Vector3(kNegZUnitVector);
	m_right = m_orientation * Vector3(kXUnitVector);

	RenderThreadSetCameraPositionAndOrientation();
}


void Camera::Orbit(float deltaPitch, float deltaYaw, const Vector3& target, const Vector3& up)
{
	m_orientation = m_orientation * Quaternion(deltaPitch, deltaYaw, 0.0f);

	auto zAxis = m_orientation * Vector3(kZUnitVector);
	auto deltaPosition = target - m_position;
	m_position = target + zAxis * Length(deltaPosition);

	m_forward = m_orientation * Vector3(kNegZUnitVector);
	m_right = m_orientation * Vector3(kXUnitVector);

	LookAt(target, up);
}


void Camera::Rotate(float deltaPitch, float deltaYaw)
{
	// TODO: Implement me
}


void Camera::Strafe(const Vector3& strafe)
{
	m_position = m_position + m_orientation * strafe;
	RenderThreadSetCameraPositionAndOrientation();
}


void Camera::CreateCameraProxy()
{
	if (!m_cameraProxy)
	{
		auto cameraProxy = (RenderThread::Camera*)_aligned_malloc(sizeof(RenderThread::Camera), alignof(RenderThread::Camera));

		m_cameraProxy = shared_ptr<RenderThread::Camera>(cameraProxy, [=](RenderThread::Camera* camera) { _aligned_free(camera); });

		m_cameraProxy->SetPositionAndOrientation(m_position, m_orientation);
		m_cameraProxy->SetPerspective(m_fov, m_aspect, m_zNear, m_zFar);
	}
}


void Camera::RenderThreadSetCameraPerspective()
{
	auto camera = shared_from_this();
	Renderer::GetInstance().EnqueueTask([camera](RenderTaskEnvironment& rte)
	{
		camera->m_cameraProxy->SetPerspective(camera->m_fov, camera->m_aspect, camera->m_zNear, camera->m_zFar);
	});
}


void Camera::RenderThreadSetCameraPositionAndOrientation()
{
	auto camera = shared_from_this();
	Renderer::GetInstance().EnqueueTask([camera](RenderTaskEnvironment& rte)
	{
		camera->m_cameraProxy->SetPositionAndOrientation(camera->m_position, camera->m_orientation);
	});
}


void Camera::RenderThreadSetReverseZ()
{
	auto camera = shared_from_this();
	Renderer::GetInstance().EnqueueTask([camera](RenderTaskEnvironment& rte)
	{
		camera->m_cameraProxy->SetReverseZ(camera->m_reverseZ);
	});
}



Kodiak::RenderThread::Camera::Camera()
	: m_projectionMatrix(kIdentity)
	, m_viewMatrix(kIdentity)
	, m_prevViewMatrix(kIdentity)
	, m_position(kZero)
	, m_orientation(kIdentity)
	, m_fov(60.0f)
	, m_aspect(1.0f)
	, m_zNear(0.1f)
	, m_zFar(10.0f)
	, m_reverseZ(false)
{}


void Kodiak::RenderThread::Camera::SetPositionAndOrientation(const Vector3& position, const Quaternion& orientation)
{
	m_position = position;
	m_orientation = orientation;
	UpdateViewMatrix();
}


void Kodiak::RenderThread::Camera::SetPerspective(float fov, float aspect, float zNear, float zFar)
{
	m_fov = fov;
	m_aspect = aspect;
	m_zNear = zNear;
	m_zFar = zFar;

	m_projectionMatrix = Matrix4::PerspectiveFovRH(ConvertToRadians(m_fov), m_aspect, m_zNear, m_zFar);
	SetReverseZ(m_reverseZ);
}


void Kodiak::RenderThread::Camera::UpdateViewMatrix()
{
	m_prevViewMatrix = m_viewMatrix;

	m_viewMatrix = Invert(Matrix4(AffineTransform(m_orientation, m_position)));
}


void Kodiak::RenderThread::Camera::SetReverseZ(bool enable)
{
	m_reverseZ = enable;

	float Q1, Q2;
	if (m_reverseZ)
	{
		Q1 = m_zNear / (m_zFar - m_zNear);
		Q2 = Q1 * m_zFar;
	}
	else
	{
		Q1 = m_zFar / (m_zNear - m_zFar);
		Q2 = Q1 * m_zNear;
	}

	Vector4 rowZ = m_projectionMatrix.GetZ();
	rowZ.SetZ(Q1);
	m_projectionMatrix.SetZ(rowZ);

	Vector4 rowW = m_projectionMatrix.GetW();
	rowW.SetZ(Q2);
	m_projectionMatrix.SetW(rowW);
}