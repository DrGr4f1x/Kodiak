// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from Matrix3.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#pragma once

#include "Quaternion.h"

namespace Math
{

// Represents a 3x3 matrix while occuping a 4x4 memory footprint.  The unused row and column are undefined but implicitly
// (0, 0, 0, 1).  Constructing a Matrix4 will make those values explicit.
__declspec(align(16)) class Matrix3
{
public:
	__forceinline Matrix3() {}
	__forceinline Matrix3(Vector3 x, Vector3 y, Vector3 z) { m_mat[0] = x; m_mat[1] = y; m_mat[2] = z; }
	__forceinline Matrix3(const Matrix3& m) { m_mat[0] = m.m_mat[0]; m_mat[1] = m.m_mat[1]; m_mat[2] = m.m_mat[2]; }
	__forceinline Matrix3(Quaternion q) { *this = Matrix3(DirectX::XMMatrixRotationQuaternion(q)); }
	__forceinline explicit Matrix3(const DirectX::XMMATRIX& m) { m_mat[0] = Vector3(m.r[0]); m_mat[1] = Vector3(m.r[1]); m_mat[2] = Vector3(m.r[2]); }
	__forceinline explicit Matrix3(EIdentityTag) { m_mat[0] = Vector3(kXUnitVector); m_mat[1] = Vector3(kYUnitVector); m_mat[2] = Vector3(kZUnitVector); }
	__forceinline explicit Matrix3(EZeroTag) { m_mat[0] = m_mat[1] = m_mat[2] = Vector3(kZero); }

	__forceinline void SetX(Vector3 x) { m_mat[0] = x; }
	__forceinline void SetY(Vector3 y) { m_mat[1] = y; }
	__forceinline void SetZ(Vector3 z) { m_mat[2] = z; }

	__forceinline Vector3 GetX() const { return m_mat[0]; }
	__forceinline Vector3 GetY() const { return m_mat[1]; }
	__forceinline Vector3 GetZ() const { return m_mat[2]; }

	static __forceinline Matrix3 MakeXRotation(float angle) { return Matrix3(DirectX::XMMatrixRotationX(angle)); }
	static __forceinline Matrix3 MakeYRotation(float angle) { return Matrix3(DirectX::XMMatrixRotationY(angle)); }
	static __forceinline Matrix3 MakeZRotation(float angle) { return Matrix3(DirectX::XMMatrixRotationZ(angle)); }
	static __forceinline Matrix3 MakeScale(float scale) { return Matrix3(DirectX::XMMatrixScaling(scale, scale, scale)); }
	static __forceinline Matrix3 MakeScale(float sx, float sy, float sz) { return Matrix3(DirectX::XMMatrixScaling(sx, sy, sz)); }
	static __forceinline Matrix3 MakeScale(Vector3 scale) { return Matrix3(DirectX::XMMatrixScalingFromVector(scale)); }

	__forceinline operator DirectX::XMMATRIX() const { return (const DirectX::XMMATRIX&)m_mat; }

	__forceinline Vector3 operator* (Vector3 vec) const { return Vector3(DirectX::XMVector3TransformNormal(vec, *this)); }
	__forceinline Matrix3 operator* (const Matrix3& mat) const { return Matrix3(*this * mat.GetX(), *this * mat.GetY(), *this * mat.GetZ()); }

private:
	Vector3 m_mat[3];
};

} // namespace Math