// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from CameraController.cpp in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#include "Stdafx.h"

#include "CameraController.h"

#include "Engine\Source\Camera.h"
#include "Engine\Source\InputState.h"


using namespace Kodiak;
using namespace DirectX;
using namespace std;


CameraController::CameraController(shared_ptr<Camera> camera, shared_ptr<InputState> inputState, const XMFLOAT3& worldUp)
	: m_camera(camera)
	, m_inputState(inputState)
	, m_worldUp(worldUp)
{
	auto xmWorldUp = XMVector3Normalize(XMLoadFloat3(&worldUp));
	auto xmWorldNorth = XMVector3Normalize(XMVector3Cross(xmWorldUp, XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f)));
	auto xmWorldEast = XMVector3Cross(xmWorldNorth, xmWorldUp);
	
	XMStoreFloat3(&m_worldUp, xmWorldUp);
	XMStoreFloat3(&m_worldNorth, xmWorldNorth);
	XMStoreFloat3(&m_worldEast, xmWorldEast);

	XMVECTOR xmForward = XMLoadFloat3(&m_camera->GetForwardVector());
	m_currentPitch = sinf(XMVectorGetX(XMVector3Dot(xmForward, xmWorldUp)));

	XMVECTOR xmRight = XMLoadFloat3(&m_camera->GetRightVector());

	xmForward = XMVector3Normalize(XMVector3Cross(xmWorldUp, xmRight));
	m_currentHeading = atan2f(
		-XMVectorGetX(XMVector3Dot(xmForward, xmWorldEast)), 
		XMVectorGetX(XMVector3Dot(xmForward, xmWorldNorth)));
}


void CameraController::Update(float deltaTime)
{
	if (m_inputState->IsFirstPressed(InputState::kLThumbClick)
		|| m_inputState->IsFirstPressed(InputState::kKey_lshift))
	{
		m_fineMovement = !m_fineMovement;
	}

	if (m_inputState->IsFirstPressed(InputState::kRThumbClick))
	{
		m_fineRotation = !m_fineRotation;
	}

	float speedScale = m_fineMovement ? 0.1f : 1.0f;
	float panScale = m_fineRotation ? 0.5f : 1.0f;

	float yaw = m_inputState->GetTimeCorrectedAnalogInput(InputState::kAnalogRightStickX) * m_horizontalLookSensitivity * panScale;
	float pitch = m_inputState->GetTimeCorrectedAnalogInput(InputState::kAnalogRightStickY) * m_verticalLookSensitivity * panScale;
	float forward = m_moveSpeed * speedScale * (
		m_inputState->GetTimeCorrectedAnalogInput(InputState::kAnalogLeftStickY) +
		(m_inputState->IsPressed(InputState::kKey_w) ? deltaTime : 0.0f) +
		(m_inputState->IsPressed(InputState::kKey_s) ? -deltaTime : 0.0f)
		);
	float strafe = m_strafeSpeed * speedScale * (
		m_inputState->GetTimeCorrectedAnalogInput(InputState::kAnalogLeftStickX) +
		(m_inputState->IsPressed(InputState::kKey_d) ? deltaTime : 0.0f) +
		(m_inputState->IsPressed(InputState::kKey_a) ? -deltaTime : 0.0f)
		);
	float ascent = m_strafeSpeed * speedScale * (
		m_inputState->GetTimeCorrectedAnalogInput(InputState::kAnalogRightTrigger) -
		m_inputState->GetTimeCorrectedAnalogInput(InputState::kAnalogLeftTrigger) +
		(m_inputState->IsPressed(InputState::kKey_e) ? deltaTime : 0.0f) +
		(m_inputState->IsPressed(InputState::kKey_q) ? -deltaTime : 0.0f)
		);

	if (m_momentum)
	{
		ApplyMomentum(m_lastYaw, yaw, deltaTime);
		ApplyMomentum(m_lastPitch, pitch, deltaTime);
		ApplyMomentum(m_lastForward, forward, deltaTime);
		ApplyMomentum(m_lastStrafe, strafe, deltaTime);
		ApplyMomentum(m_lastAscent, ascent, deltaTime);
	}

	// don't apply momentum to mouse inputs
	yaw += m_inputState->GetAnalogInput(InputState::kAnalogMouseX) * m_mouseSensitivityX;
	pitch += m_inputState->GetAnalogInput(InputState::kAnalogMouseY) * m_mouseSensitivityY;

	m_currentPitch += pitch;
	m_currentPitch = XMMin(XM_PIDIV2, m_currentPitch);
	m_currentPitch = XMMax(-XM_PIDIV2, m_currentPitch);

	m_currentHeading -= yaw;
	if (m_currentHeading > XM_PI)
	{
		m_currentHeading -= XM_2PI;
	}
	else if (m_currentHeading <= -XM_PI)
	{
		m_currentHeading += XM_2PI;
	}

	auto xmWorldNorth = XMLoadFloat3(&m_worldNorth);
	auto xmWorldEast = XMLoadFloat3(&m_worldEast);
	auto xmWorldUp = XMLoadFloat3(&m_worldUp);
	XMMATRIX xmTemp{ xmWorldEast, xmWorldUp, XMVectorNegate(xmWorldNorth), XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f) };
	XMMATRIX rotY = XMMatrixRotationY(m_currentHeading);
	XMMATRIX rotX = XMMatrixRotationX(m_currentPitch);
	XMMATRIX xmOrientation = XMMatrixMultiply(XMMatrixMultiply(rotX, rotY), xmTemp);
	auto xmPosition = XMVectorAdd(XMVector3Transform(XMVectorSet(strafe, ascent, -forward, 0.0f), xmOrientation), XMLoadFloat3(&m_camera->GetPosition()));

	XMFLOAT3 position;
	XMStoreFloat3(&position, xmPosition);

	XMFLOAT4 orientation;
	XMStoreFloat4(&orientation, XMQuaternionRotationMatrix(xmOrientation));

	m_camera->SetPositionAndOrientation(position, orientation);
}


// TODO: Move this to the math library
// TODO: Write a math library
namespace
{
inline float Lerp(float a, float b, float s)
{
	return (1.0f - s) * a + s * b;
}
}


void CameraController::ApplyMomentum(float& oldValue, float& newValue, float deltaTime)
{
	float blendedValue;
	if (fabsf(newValue) > fabsf(oldValue))
	{
		blendedValue = Lerp(newValue, oldValue, powf(0.6f, deltaTime * 60.0f));
	}
	else
		blendedValue = Lerp(newValue, oldValue, powf(0.8f, deltaTime * 60.0f));
	oldValue = blendedValue;
	newValue = blendedValue;
}