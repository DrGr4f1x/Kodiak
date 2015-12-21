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

#include "IAsyncRenderTask.h"
#include "MathUtil.h"
#include "Renderer.h"
#include "Scene.h"


using namespace Kodiak;
using namespace Math;
using namespace std;
using namespace DirectX;


Camera::Camera()
	: m_position(0.0f, 0.0f, 0.0f)
	, m_orientation(0.0f, 0.0f, 0.0f, 1.0f)
	, m_fov(60.0f)
	, m_aspect(1.0f)
	, m_zNear(0.1f)
	, m_zFar(10.0f)
	, m_cameraProxy()
{}


Camera::Camera(const XMFLOAT3& position, const XMFLOAT4& orientation)
	: m_position(position)
	, m_orientation(orientation)
	, m_fov(60.0f)
	, m_aspect(1.0f)
	, m_zNear(0.1f)
	, m_zFar(10.0f)
	, m_cameraProxy()
{}


Camera::Camera(const XMFLOAT3& position, const XMFLOAT4& orientation, float fov, float aspect, float zNear, float zFar)
	: m_position(position)
	, m_orientation(orientation)
	, m_fov(fov)
	, m_aspect(aspect)
	, m_zNear(zNear)
	, m_zFar(zFar)
	, m_cameraProxy()
{}


void Camera::SetPosition(const XMFLOAT3& position)
{
	if (m_position != position)
	{
		m_position = position;
		RenderThread::SetCameraPositionAndOrientation(shared_from_this(), m_position, m_orientation);
	}
}


void Camera::SetOrientation(const XMFLOAT4& orientation)
{
	if (m_orientation != orientation)
	{
		m_orientation = orientation;
		RenderThread::SetCameraPositionAndOrientation(shared_from_this(), m_position, m_orientation);
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
		RenderThread::SetCameraPerspective(shared_from_this(), m_fov, m_aspect, m_zNear, m_zFar);
	}
}


void Camera::SetAspectRatio(float aspect)
{
	if (m_aspect != aspect)
	{
		m_aspect = aspect;
		RenderThread::SetCameraPerspective(shared_from_this(), m_fov, m_aspect, m_zNear, m_zFar);
	}
}


void Camera::SetFOV(float fov)
{
	if (m_fov != fov)
	{
		m_fov = fov;
		RenderThread::SetCameraPerspective(shared_from_this(), m_fov, m_aspect, m_zNear, m_zFar);
	}
}


void Camera::LookAt(const XMFLOAT3& target, const XMFLOAT3& up)
{
	XMMATRIX temp = XMMatrixLookAtRH(XMLoadFloat3(&m_position), XMLoadFloat3(&target), XMLoadFloat3(&up));
	XMVECTOR dummy;
	XMStoreFloat4(&m_orientation, XMQuaternionNormalize(XMQuaternionRotationMatrix(XMMatrixInverse(&dummy, temp))));

	RenderThread::SetCameraPositionAndOrientation(shared_from_this(), m_position, m_orientation);
}


void Camera::LookIn(const XMFLOAT3& dir, const XMFLOAT3& up)
{
	XMVECTOR pos = XMLoadFloat3(&m_position);
	XMVECTOR dummy;
	XMMATRIX temp = XMMatrixLookAtRH(pos, XMVectorAdd(pos, XMLoadFloat3(&dir)), XMLoadFloat3(&up));
	XMStoreFloat4(&m_orientation, XMQuaternionNormalize(XMQuaternionRotationMatrix(XMMatrixInverse(&dummy, temp))));

	RenderThread::SetCameraPositionAndOrientation(shared_from_this(), m_position, m_orientation);
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

	RenderThread::SetCameraPositionAndOrientation(shared_ptr<Camera>(this), m_position, m_orientation);
}


void Camera::CreateProxy()
{
	if (!m_cameraProxy)
	{
		m_cameraProxy = make_shared<RenderThread::Camera>();

		m_cameraProxy->SetPositionAndOrientation(m_position, m_orientation);
		m_cameraProxy->SetPerspective(m_fov, m_aspect, m_zNear, m_zFar);
	}
}


namespace
{

class SetCameraPositionAndOrientationTask : public IAsyncRenderTask
{
public:
	SetCameraPositionAndOrientationTask(shared_ptr<Kodiak::Camera> camera, const XMFLOAT3& position, const XMFLOAT4& orientation)
		: m_camera(camera)
		, m_position(position)
		, m_orientation(orientation)
	{}
	
	void Execute(RenderTaskEnvironment& environment) override
	{
		auto cameraProxy = m_camera->GetProxy();
		if (cameraProxy)
		{
			cameraProxy->SetPositionAndOrientation(m_position, m_orientation);
		}
	}

private:
	shared_ptr<Kodiak::Camera>	m_camera;
	const XMFLOAT3				m_position;
	const XMFLOAT4				m_orientation;
};


class SetCameraPerspectiveTask : public IAsyncRenderTask
{
public:
	SetCameraPerspectiveTask(shared_ptr<Kodiak::Camera> camera, float fov, float aspect, float zNear, float zFar)
		: m_camera(camera)
		, m_fov(fov)
		, m_aspect(aspect)
		, m_zNear(zNear)
		, m_zFar(zFar)
	{}

	void Execute(RenderTaskEnvironment& environment) override
	{
		auto cameraProxy = m_camera->GetProxy();
		if (cameraProxy)
		{
			cameraProxy->SetPerspective(m_fov, m_aspect, m_zNear, m_zFar);
		}
	}

private:
	shared_ptr<Kodiak::Camera>	m_camera;
	float						m_fov;
	float						m_aspect;
	float						m_zNear;
	float						m_zFar;
};


class SetSceneCameraTask : public IAsyncRenderTask
{
public:
	SetSceneCameraTask(shared_ptr<Kodiak::Scene> scene, shared_ptr<Kodiak::Camera> camera)
		: m_scene(scene)
		, m_camera(camera)
	{}

	void Execute(RenderTaskEnvironment& environment) override
	{
		m_scene->SetCamera(m_camera);
	}

private:
	shared_ptr<Kodiak::Scene>	m_scene;
	shared_ptr<Kodiak::Camera>	m_camera;
};


} // anonymous namespace



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

namespace Kodiak
{

namespace RenderThread
{

void SetSceneCamera(shared_ptr<Kodiak::Scene> scene, shared_ptr<Kodiak::Camera> camera)
{
	auto task = make_shared<SetSceneCameraTask>(scene, camera);
	Renderer::EnqueueTask(task);
}


void SetCameraPositionAndOrientation(shared_ptr<Kodiak::Camera> camera, const XMFLOAT3& position, const XMFLOAT4& orientation)
{
	auto task = make_shared<SetCameraPositionAndOrientationTask>(camera, position, orientation);
	Renderer::EnqueueTask(task);
}


void SetCameraPerspective(std::shared_ptr<Kodiak::Camera> camera, float fov, float aspect, float zNear, float zFar)
{
	auto task = make_shared<SetCameraPerspectiveTask>(camera, fov, aspect, zNear, zFar);
	Renderer::EnqueueTask(task);
}

} // namespace RenderThread

} // namespace Kodiak