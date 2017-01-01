// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from Transform.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#pragma once

#include "Matrix3.h"

namespace Math
{

// This transform strictly prohibits non-uniform scale.  Scale itself is barely tolerated.
__declspec(align(16)) class OrthogonalTransform
{
public:
	__forceinline OrthogonalTransform() : m_rotation(kIdentity), m_translation(kZero) {}
	__forceinline OrthogonalTransform(Quaternion rotate) : m_rotation(rotate), m_translation(kZero) {}
	__forceinline OrthogonalTransform(Vector3 translate) : m_rotation(kIdentity), m_translation(translate) {}
	__forceinline OrthogonalTransform(Quaternion rotate, Vector3 translate) : m_rotation(rotate), m_translation(translate) {}
	__forceinline OrthogonalTransform(const Matrix3& mat) : m_rotation(mat), m_translation(kZero) {}
	__forceinline OrthogonalTransform(const Matrix3& mat, Vector3 translate) : m_rotation(mat), m_translation(translate) {}
	__forceinline OrthogonalTransform(EIdentityTag) : m_rotation(kIdentity), m_translation(kZero) {}
	__forceinline explicit OrthogonalTransform(const DirectX::XMMATRIX& mat) { *this = OrthogonalTransform(Matrix3(mat), Vector3(mat.r[3])); }

	__forceinline void SetRotation(Quaternion q) { m_rotation = q; }
	__forceinline void SetTranslation(Vector3 v) { m_translation = v; }

	__forceinline Quaternion GetRotation() const { return m_rotation; }
	__forceinline Vector3 GetTranslation() const { return m_translation; }

	static __forceinline OrthogonalTransform MakeXRotation(float angle) { return OrthogonalTransform(Quaternion(Vector3(kXUnitVector), angle)); }
	static __forceinline OrthogonalTransform MakeYRotation(float angle) { return OrthogonalTransform(Quaternion(Vector3(kYUnitVector), angle)); }
	static __forceinline OrthogonalTransform MakeZRotation(float angle) { return OrthogonalTransform(Quaternion(Vector3(kZUnitVector), angle)); }
	static __forceinline OrthogonalTransform MakeTranslation(Vector3 translate) { return OrthogonalTransform(translate); }

	__forceinline Vector3 operator*(Vector3 vec) const { return m_rotation * vec + m_translation; }
	__forceinline Vector4 operator*(Vector4 vec) const 
	{
		return
			Vector4(SetWToZero(m_rotation * Vector3((DirectX::XMVECTOR)vec))) +
			Vector4(SetWToOne(m_translation)) * vec.GetW();
	}

	__forceinline OrthogonalTransform operator*(const OrthogonalTransform& xform) const 
	{
		return OrthogonalTransform(m_rotation * xform.m_rotation, m_rotation * xform.m_translation + m_translation);
	}

	__forceinline OrthogonalTransform operator~() const 
	{
		Quaternion invertedRotation = ~m_rotation;
		return OrthogonalTransform(invertedRotation, invertedRotation * -m_translation);
	}

private:

	Quaternion m_rotation;
	Vector3 m_translation;
};


// A AffineTransform is a 3x4 matrix with an implicit 4th row = [0,0,0,1].  This is used to perform a change of
// basis on 3D points.  An affine transformation does not have to have orthonormal basis vectors.
__declspec(align(64)) class AffineTransform
{
public:
	__forceinline AffineTransform()
	{}
	__forceinline AffineTransform(Vector3 x, Vector3 y, Vector3 z, Vector3 w)
		: m_basis(x, y, z), m_translation(w) {}
	__forceinline AffineTransform(Vector3 translate)
		: m_basis(kIdentity), m_translation(translate) {}
	__forceinline AffineTransform(const Matrix3& mat, Vector3 translate = Vector3(kZero))
		: m_basis(mat), m_translation(translate) {}
	__forceinline AffineTransform(Quaternion rot, Vector3 translate = Vector3(kZero))
		: m_basis(rot), m_translation(translate) {}
	__forceinline AffineTransform(const OrthogonalTransform& xform)
		: m_basis(xform.GetRotation()), m_translation(xform.GetTranslation()) {}
	__forceinline AffineTransform(EIdentityTag)
		: m_basis(kIdentity), m_translation(kZero) {}
	__forceinline explicit AffineTransform(const DirectX::XMMATRIX& mat)
		: m_basis(mat), m_translation(mat.r[3]) {}

	__forceinline operator DirectX::XMMATRIX() const { return (DirectX::XMMATRIX&)*this; }

	__forceinline void SetX(Vector3 x) { m_basis.SetX(x); }
	__forceinline void SetY(Vector3 y) { m_basis.SetY(y); }
	__forceinline void SetZ(Vector3 z) { m_basis.SetZ(z); }
	__forceinline void SetTranslation(Vector3 w) { m_translation = w; }

	__forceinline Vector3 GetX() const { return m_basis.GetX(); }
	__forceinline Vector3 GetY() const { return m_basis.GetY(); }
	__forceinline Vector3 GetZ() const { return m_basis.GetZ(); }
	__forceinline Vector3 GetTranslation() const { return m_translation; }
	__forceinline const Matrix3& GetBasis() const { return (const Matrix3&)*this; }

	static __forceinline AffineTransform MakeXRotation(float angle) { return AffineTransform(Matrix3::MakeXRotation(angle)); }
	static __forceinline AffineTransform MakeYRotation(float angle) { return AffineTransform(Matrix3::MakeYRotation(angle)); }
	static __forceinline AffineTransform MakeZRotation(float angle) { return AffineTransform(Matrix3::MakeZRotation(angle)); }
	static __forceinline AffineTransform MakeScale(float scale) { return AffineTransform(Matrix3::MakeScale(scale)); }
	static __forceinline AffineTransform MakeScale(Vector3 scale) { return AffineTransform(Matrix3::MakeScale(scale)); }
	static __forceinline AffineTransform MakeTranslation(Vector3 translate) { return AffineTransform(translate); }

	__forceinline Vector3 operator*(Vector3 vec) const { return m_basis * vec + m_translation; }
	__forceinline AffineTransform operator*(const AffineTransform& mat) const {
		return AffineTransform(m_basis * mat.m_basis, *this * mat.GetTranslation());
	}

private:
	Matrix3 m_basis;
	Vector3 m_translation;
};

} // namespace Math