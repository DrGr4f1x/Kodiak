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

#include "Camera.h"
#include "InputState.h"


using namespace Kodiak;
using namespace Math;
using namespace std;


CameraController::CameraController(shared_ptr<Camera> camera, shared_ptr<InputState> inputState, const Vector3& worldUp)
	: m_camera(camera)
	, m_inputState(inputState)
	, m_worldUp(worldUp)
{

	m_worldUp = Normalize(worldUp);
	m_worldNorth = Normalize(Cross(m_worldUp, Vector3(kXUnitVector)));
	m_worldEast = Cross(m_worldNorth, m_worldUp);

	Vector3 tempForward(DirectX::XMLoadFloat3(&camera->GetForwardVector())); // TODO get rid of this
	m_currentPitch = Sin(Dot(tempForward, m_worldUp));

	Vector3 tempRight(DirectX::XMLoadFloat3(&camera->GetRightVector())); // TODO get rid of this
	Vector3 forward = Normalize(Cross(m_worldUp, tempRight));
	m_currentHeading = ATan2(-Dot(forward, m_worldEast), Dot(forward, m_worldNorth));
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
	m_currentPitch = DirectX::XMMin(DirectX::XM_PIDIV2, m_currentPitch);
	m_currentPitch = DirectX::XMMax(-DirectX::XM_PIDIV2, m_currentPitch);

	m_currentHeading -= yaw;
	if (m_currentHeading > DirectX::XM_PI)
	{
		m_currentHeading -= DirectX::XM_2PI;
	}
	else if (m_currentHeading <= -DirectX::XM_PI)
	{
		m_currentHeading += DirectX::XM_2PI;
	}

	Matrix3 temp = Matrix3(m_worldEast, m_worldUp, -m_worldNorth);
	Matrix3 rotY = Matrix3::MakeYRotation(m_currentHeading);
	Matrix3 rotX = Matrix3::MakeXRotation(m_currentPitch);
	Matrix3 temp_orientation = temp * rotY * rotX;
	Vector3 tempPosition(DirectX::XMLoadFloat3(&m_camera->GetPosition())); // TODO get rid of this
	Vector3 temp_position = temp_orientation * Vector3(strafe, ascent, -forward) + tempPosition;

	DirectX::XMFLOAT3 position;
	DirectX::XMStoreFloat3(&position, temp_position);

	DirectX::XMFLOAT4 orientation;
	DirectX::XMStoreFloat4(&orientation, DirectX::XMQuaternionRotationMatrix(temp_orientation));

	m_camera->SetPositionAndOrientation(position, orientation);
}


void CameraController::ApplyMomentum(float& oldValue, float& newValue, float deltaTime)
{
	float blendedValue;

	if (Abs(newValue) > Abs(oldValue))
	{
		blendedValue = Lerp(newValue, oldValue, Pow(0.6f, deltaTime * 60.0f));
	}
	else
	{
		blendedValue = Lerp(newValue, oldValue, Pow(0.8f, deltaTime * 60.0f));
	}

	oldValue = blendedValue;
	newValue = blendedValue;
}