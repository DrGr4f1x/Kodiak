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
	: m_position(0.0f, 0.0f, 0.0f)
	, m_orientation(0.0f, 0.0f, 0.0f, 1.0f)
	, m_fov(60.0f)
	, m_aspect(1.0f)
	, m_zNear(0.1f)
	, m_zFar(10.0f)
	, m_cameraProxy()
{
	CreateCameraProxy();
}


Camera::Camera(const XMFLOAT3& position, const XMFLOAT4& orientation)
	: m_position(position)
	, m_orientation(orientation)
	, m_fov(60.0f)
	, m_aspect(1.0f)
	, m_zNear(0.1f)
	, m_zFar(10.0f)
	, m_cameraProxy()
{
	CreateCameraProxy();
}


Camera::Camera(const XMFLOAT3& position, const XMFLOAT4& orientation, float fov, float aspect, float zNear, float zFar)
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


void Camera::SetPosition(const XMFLOAT3& position)
{
	if (!Compare(m_position, position))
	{
		m_position = position;
		RenderThreadSetCameraPositionAndOrientation();
	}
}


void Camera::SetOrientation(const XMFLOAT4& orientation)
{
	if (!Compare(m_orientation, orientation))
	{
		m_orientation = orientation;

		XMVECTOR xmOrientation = XMLoadFloat4(&m_orientation);
		XMStoreFloat3(&m_forward, XMVector3Rotate(XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), xmOrientation));

		RenderThreadSetCameraPositionAndOrientation();
	}
}


void Camera::SetPositionAndOrientation(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT4& orientation)
{
	bool update = false;

	if (!Compare(m_position, position))
	{
		m_position = position;
		update = true;
	}

	if (!Compare(m_orientation, orientation))
	{
		m_orientation = orientation;

		XMVECTOR xmOrientation = XMLoadFloat4(&m_orientation);
		XMStoreFloat3(&m_forward, XMVector3Rotate(XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), xmOrientation));

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


void Camera::LookAt(const XMFLOAT3& target, const XMFLOAT3& up)
{
	XMMATRIX temp = XMMatrixLookAtRH(XMLoadFloat3(&m_position), XMLoadFloat3(&target), XMLoadFloat3(&up));
	XMVECTOR dummy;
	XMVECTOR xmOrientation = XMQuaternionNormalize(XMQuaternionRotationMatrix(XMMatrixInverse(&dummy, temp)));
	XMStoreFloat4(&m_orientation, xmOrientation);

	XMStoreFloat3(&m_forward, XMVector3Rotate(XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), xmOrientation));
	XMStoreFloat3(&m_right, XMVector3Rotate(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), xmOrientation));

	RenderThreadSetCameraPositionAndOrientation();
}


void Camera::LookIn(const XMFLOAT3& dir, const XMFLOAT3& up)
{
	XMVECTOR pos = XMLoadFloat3(&m_position);
	XMVECTOR dummy;
	XMMATRIX temp = XMMatrixLookAtRH(pos, XMVectorAdd(pos, XMLoadFloat3(&dir)), XMLoadFloat3(&up));
	XMVECTOR xmOrientation = XMQuaternionNormalize(XMQuaternionRotationMatrix(XMMatrixInverse(&dummy, temp)));
	XMStoreFloat4(&m_orientation, xmOrientation);

	XMStoreFloat3(&m_forward, XMVector3Rotate(XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), xmOrientation));
	XMStoreFloat3(&m_right, XMVector3Rotate(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), xmOrientation));

	RenderThreadSetCameraPositionAndOrientation();
}


void Camera::Orbit(float deltaPitch, float deltaYaw, const XMFLOAT3& target, const XMFLOAT3& up)
{
	XMVECTOR xmPosition = XMLoadFloat3(&m_position);
	XMVECTOR xmOrientation = XMLoadFloat4(&m_orientation);
	XMVECTOR xmTarget = XMLoadFloat3(&target);
	
	xmOrientation = XMQuaternionMultiply(XMQuaternionRotationRollPitchYaw(0.0f, deltaPitch, deltaYaw), xmOrientation);
	XMVECTOR xmZAxis = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	xmZAxis = XMVector3TransformNormal(xmZAxis, XMMatrixRotationQuaternion(xmOrientation));
	XMVECTOR xmDeltaPosition = XMVectorSubtract(xmTarget, xmPosition);
	xmPosition = XMVectorAdd(xmTarget, XMVectorScale(xmZAxis, XMVectorGetX(XMVector3Length(xmDeltaPosition))));

	XMStoreFloat3(&m_position, xmPosition);
	XMStoreFloat4(&m_orientation, xmOrientation);

	XMStoreFloat3(&m_forward, XMVector3Rotate(XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), xmOrientation));
	XMStoreFloat3(&m_right, XMVector3Rotate(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), xmOrientation));

	LookAt(target, up);
}


void Camera::Rotate(float deltaPitch, float deltaYaw)
{
	// TODO: Implement me
}


void Camera::Strafe(const XMFLOAT3& strafe)
{
	XMVECTOR xmPosition = XMLoadFloat3(&m_position);
	XMVECTOR xmOrientation = XMLoadFloat4(&m_orientation);
	XMVECTOR xmStrafe = XMLoadFloat3(&strafe);

	xmPosition = XMVectorAdd(xmPosition, XMVector3Rotate(xmStrafe, xmOrientation));

	XMStoreFloat3(&m_position, xmPosition);

	RenderThreadSetCameraPositionAndOrientation();
}


void Camera::CreateCameraProxy()
{
	if (!m_cameraProxy)
	{
		m_cameraProxy = make_shared<RenderThread::Camera>();

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



Kodiak::RenderThread::Camera::Camera()
	: m_projectionMatrix()
	, m_viewMatrix()
	, m_prevViewMatrix()
	, m_position(0.0f, 0.0f, 0.0f)
	, m_orientation(0.0f, 0.0f, 0.0f, 1.0f)
	, m_fov(60.0f)
	, m_aspect(1.0f)
	, m_zNear(0.1f)
	, m_zFar(10.0f)
{
	XMStoreFloat4x4(&m_projectionMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_viewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_prevViewMatrix, XMMatrixIdentity());
}


void Kodiak::RenderThread::Camera::SetPositionAndOrientation(const XMFLOAT3& position, const XMFLOAT4& orientation)
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

	XMStoreFloat4x4(&m_projectionMatrix, XMMatrixPerspectiveFovRH(XMConvertToRadians(m_fov), m_aspect, m_zNear, m_zFar));
}


void Kodiak::RenderThread::Camera::UpdateViewMatrix()
{
	m_prevViewMatrix = m_viewMatrix;

	XMVECTOR dummy;
	XMStoreFloat4x4(&m_viewMatrix,
		XMMatrixInverse(&dummy,
			XMMatrixMultiply(
				XMMatrixRotationQuaternion(XMLoadFloat4(&m_orientation)),
				XMMatrixTranslationFromVector(XMLoadFloat3(&m_position))
				)
			)
		);
}