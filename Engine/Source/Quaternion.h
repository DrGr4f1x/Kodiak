// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from Quaternion.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#pragma once

#include "Vector.h"

namespace Math
{

class Quaternion
{
public:
	__forceinline Quaternion() { m_vec = DirectX::XMQuaternionIdentity(); }
	__forceinline Quaternion(const Vector3& axis, const Scalar& angle) { m_vec = DirectX::XMQuaternionRotationAxis(axis, angle); }
	__forceinline Quaternion(float pitch, float yaw, float roll) { m_vec = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll); }
	__forceinline explicit Quaternion(const DirectX::XMMATRIX& matrix) { m_vec = DirectX::XMQuaternionRotationMatrix(matrix); }
	__forceinline explicit Quaternion(DirectX::FXMVECTOR vec) { m_vec = vec; }
	__forceinline explicit Quaternion(EIdentityTag) { m_vec = DirectX::XMQuaternionIdentity(); }

	__forceinline operator DirectX::XMVECTOR() const { return m_vec; }

	__forceinline Quaternion operator~() const { return Quaternion(DirectX::XMQuaternionConjugate(m_vec)); }
	__forceinline Quaternion operator-() const { return Quaternion(DirectX::XMVectorNegate(m_vec)); }

	__forceinline Quaternion operator*(Quaternion rhs) const { return Quaternion(DirectX::XMQuaternionMultiply(rhs, m_vec)); }
	__forceinline Vector3 operator*(Vector3 rhs) const { return Vector3(DirectX::XMVector3Rotate(rhs, m_vec)); }

	__forceinline Quaternion& operator=(Quaternion rhs) { m_vec = rhs; return *this; }
	__forceinline Quaternion& operator*=(Quaternion rhs) { *this = *this * rhs; return *this; }

	__forceinline bool Equal(Quaternion other)
	{
		return DirectX::XMQuaternionEqual(m_vec, other);
	}

	__forceinline bool NotEqual(Quaternion other)
	{
		return DirectX::XMQuaternionNotEqual(m_vec, other);
	}

protected:
	DirectX::XMVECTOR m_vec;
};

} // namespace Math