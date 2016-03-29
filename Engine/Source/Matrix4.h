// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from Matrx4.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#pragma once

#include "Transform.h"

namespace Math
{

__declspec(align(64)) class Matrix4
{
public:
	__forceinline Matrix4() {}
	__forceinline Matrix4(Vector3 x, Vector3 y, Vector3 z, Vector3 w)
	{
		m_mat.r[0] = SetWToZero(x); m_mat.r[1] = SetWToZero(y);
		m_mat.r[2] = SetWToZero(z); m_mat.r[3] = SetWToOne(w);
	}
	__forceinline Matrix4(Vector4 x, Vector4 y, Vector4 z, Vector4 w) { m_mat.r[0] = x; m_mat.r[1] = y; m_mat.r[2] = z; m_mat.r[3] = w; }
	__forceinline Matrix4(const Matrix4& mat) { m_mat = mat.m_mat; }
	__forceinline Matrix4(const Matrix3& mat)
	{
		m_mat.r[0] = SetWToZero(mat.GetX());
		m_mat.r[1] = SetWToZero(mat.GetY());
		m_mat.r[2] = SetWToZero(mat.GetZ());
		m_mat.r[3] = CreateWUnitVector();
	}
	__forceinline Matrix4(const Matrix3& xyz, Vector3 w)
	{
		m_mat.r[0] = SetWToZero(xyz.GetX());
		m_mat.r[1] = SetWToZero(xyz.GetY());
		m_mat.r[2] = SetWToZero(xyz.GetZ());
		m_mat.r[3] = SetWToOne(w);
	}
	__forceinline Matrix4(const AffineTransform& xform) { *this = Matrix4(xform.GetBasis(), xform.GetTranslation()); }
	__forceinline Matrix4(const OrthogonalTransform& xform) { *this = Matrix4(Matrix3(xform.GetRotation()), xform.GetTranslation()); }
	__forceinline explicit Matrix4(const DirectX::XMMATRIX& mat) { m_mat = mat; }
	__forceinline explicit Matrix4(EIdentityTag) { m_mat = DirectX::XMMatrixIdentity(); }
	__forceinline explicit Matrix4(EZeroTag) { m_mat.r[0] = m_mat.r[1] = m_mat.r[2] = m_mat.r[3] = SplatZero(); }

	__forceinline const Matrix3& Get3x3() const { return (const Matrix3&)*this; }

	__forceinline Vector4 GetX() const { return Vector4(m_mat.r[0]); }
	__forceinline Vector4 GetY() const { return Vector4(m_mat.r[1]); }
	__forceinline Vector4 GetZ() const { return Vector4(m_mat.r[2]); }
	__forceinline Vector4 GetW() const { return Vector4(m_mat.r[3]); }

	__forceinline void SetX(Vector4 x) { m_mat.r[0] = x; }
	__forceinline void SetY(Vector4 y) { m_mat.r[1] = y; }
	__forceinline void SetZ(Vector4 z) { m_mat.r[2] = z; }
	__forceinline void SetW(Vector4 w) { m_mat.r[3] = w; }

	__forceinline operator DirectX::XMMATRIX() const { return m_mat; }

	__forceinline Vector4 operator*(Vector3 vec) const { return Vector4(DirectX::XMVector3Transform(vec, m_mat)); }
	__forceinline Vector4 operator*(Vector4 vec) const { return Vector4(DirectX::XMVector4Transform(vec, m_mat)); }
	__forceinline Matrix4 operator*(const Matrix4& mat) const { return Matrix4(DirectX::XMMatrixMultiply(mat, m_mat)); }

	static __forceinline Matrix4 Scaling(float scale) { return Matrix4(DirectX::XMMatrixScaling(scale, scale, scale)); }
	static __forceinline Matrix4 Scaling(Vector3 scale) { return Matrix4(DirectX::XMMatrixScalingFromVector(scale)); }

	static __forceinline Matrix4 RotationX(float angle) { return Matrix4(DirectX::XMMatrixRotationX(angle)); }
	static __forceinline Matrix4 RotationY(float angle) { return Matrix4(DirectX::XMMatrixRotationY(angle)); }
	static __forceinline Matrix4 RotationZ(float angle) { return Matrix4(DirectX::XMMatrixRotationZ(angle)); }

	static __forceinline Matrix4 Translation(float x, float y, float z) { return Matrix4(DirectX::XMMatrixTranslation(x, y, z)); }
	static __forceinline Matrix4 Translation(Vector3 translation) { return Matrix4(DirectX::XMMatrixTranslationFromVector(translation)); }

	static __forceinline Matrix4 LookAtRH(Vector3 position, Vector3 target, Vector3 up)
	{
		return Matrix4(DirectX::XMMatrixLookAtRH(position, target, up));
	}

	static __forceinline Matrix4 LookToRH(Vector3 position, Vector3 direction, Vector3 up)
	{
		return Matrix4(DirectX::XMMatrixLookToRH(position, direction, up));
	}

	static __forceinline Matrix4 LookAtLH(Vector3 position, Vector3 target, Vector3 up)
	{
		return Matrix4(DirectX::XMMatrixLookAtLH(position, target, up));
	}

	static __forceinline Matrix4 LookToLH(Vector3 position, Vector3 direction, Vector3 up)
	{
		return Matrix4(DirectX::XMMatrixLookToLH(position, direction, up));
	}

	static __forceinline Matrix4 PerspectiveFovRH(float fov, float aspect, float zNear, float zFar)
	{
		return Matrix4(DirectX::XMMatrixPerspectiveFovRH(fov, aspect, zNear, zFar));
	}

	static __forceinline Matrix4 PerspectiveFovLH(float fov, float aspect, float zNear, float zFar)
	{
		return Matrix4(DirectX::XMMatrixPerspectiveFovLH(fov, aspect, zNear, zFar));
	}

private:
	DirectX::XMMATRIX m_mat;
};

} // namespace Math