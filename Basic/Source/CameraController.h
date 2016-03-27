// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from CameraController.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#pragma once

namespace Kodiak
{

// Forward declarations
class Camera;
class InputState;

class CameraController
{
public:
	CameraController(std::shared_ptr<Camera> camera, std::shared_ptr<InputState> inputState, const DirectX::XMFLOAT3& worldUp);

	void Update(float deltaTime);

	void SlowMovement(bool enable)
	{
		m_fineMovement = enable;
	}

	void SlowRotation(bool enable)
	{
		m_fineRotation = enable;
	}

	void EnableMomemtum(bool enable)
	{
		m_momentum = enable;
	}

private:
	void ApplyMomentum(float& oldValue, float& newValue, float deltaTime);

private:
	std::shared_ptr<Camera>			m_camera;
	std::shared_ptr<InputState>		m_inputState;

	DirectX::XMFLOAT3 m_worldUp;
	DirectX::XMFLOAT3 m_worldNorth;
	DirectX::XMFLOAT3 m_worldEast;

	float m_horizontalLookSensitivity{ 2.0f };
	float m_verticalLookSensitivity{ 2.0f };
	float m_moveSpeed{ 10.0f };
	float m_strafeSpeed{ 10.0f };
	float m_mouseSensitivityX{ 1.0f };
	float m_mouseSensitivityY{ 1.0f };
	float m_currentHeading;
	float m_currentPitch;

	bool m_fineMovement{ false };
	bool m_fineRotation{ false };
	bool m_momentum{ true };

	float m_lastYaw{ 0.0f };
	float m_lastPitch{ 0.0f };
	float m_lastForward{ 0.0f };
	float m_lastStrafe{ 0.0f };
	float m_lastAscent{ 0.0f };
};

} // namespace Kodiak