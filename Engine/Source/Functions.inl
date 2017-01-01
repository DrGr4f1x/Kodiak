// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from Functions.inl in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#pragma once

namespace Math
{

// To allow floats to implicitly construct Scalars, we need to clarify these operators and suppress
// upconversion.
__forceinline bool operator<  (Scalar lhs, float rhs) { return (float)lhs <  rhs; }
__forceinline bool operator<= (Scalar lhs, float rhs) { return (float)lhs <= rhs; }
__forceinline bool operator>  (Scalar lhs, float rhs) { return (float)lhs >  rhs; }
__forceinline bool operator>= (Scalar lhs, float rhs) { return (float)lhs >= rhs; }
__forceinline bool operator== (Scalar lhs, float rhs) { return (float)lhs == rhs; }
__forceinline bool operator<  (float lhs, Scalar rhs) { return lhs <  (float)rhs; }
__forceinline bool operator<= (float lhs, Scalar rhs) { return lhs <= (float)rhs; }
__forceinline bool operator>  (float lhs, Scalar rhs) { return lhs >  (float)rhs; }
__forceinline bool operator>= (float lhs, Scalar rhs) { return lhs >= (float)rhs; }
__forceinline bool operator== (float lhs, Scalar rhs) { return lhs == (float)rhs; }

#define CREATE_SIMD_FUNCTIONS( TYPE ) \
	__forceinline TYPE Sqrt( TYPE s ) { return TYPE(DirectX::XMVectorSqrt(s)); } \
	__forceinline TYPE Recip( TYPE s ) { return TYPE(DirectX::XMVectorReciprocal(s)); } \
	__forceinline TYPE RecipSqrt( TYPE s ) { return TYPE(DirectX::XMVectorReciprocalSqrt(s)); } \
	__forceinline TYPE Floor( TYPE s ) { return TYPE(DirectX::XMVectorFloor(s)); } \
	__forceinline TYPE Ceiling( TYPE s ) { return TYPE(DirectX::XMVectorCeiling(s)); } \
	__forceinline TYPE Round( TYPE s ) { return TYPE(DirectX::XMVectorRound(s)); } \
	__forceinline TYPE Abs( TYPE s ) { return TYPE(DirectX::XMVectorAbs(s)); } \
	__forceinline TYPE Exp( TYPE s ) { return TYPE(DirectX::XMVectorExp(s)); } \
	__forceinline TYPE Pow( TYPE b, TYPE e ) { return TYPE(DirectX::XMVectorPow(b, e)); } \
	__forceinline TYPE Log( TYPE s ) { return TYPE(DirectX::XMVectorLog(s)); } \
	__forceinline TYPE Sin( TYPE s ) { return TYPE(DirectX::XMVectorSin(s)); } \
	__forceinline TYPE Cos( TYPE s ) { return TYPE(DirectX::XMVectorCos(s)); } \
	__forceinline TYPE Tan( TYPE s ) { return TYPE(DirectX::XMVectorTan(s)); } \
	__forceinline TYPE ASin( TYPE s ) { return TYPE(DirectX::XMVectorASin(s)); } \
	__forceinline TYPE ACos( TYPE s ) { return TYPE(DirectX::XMVectorACos(s)); } \
	__forceinline TYPE ATan( TYPE s ) { return TYPE(DirectX::XMVectorATan(s)); } \
	__forceinline TYPE ATan2( TYPE y, TYPE x ) { return TYPE(DirectX::XMVectorATan2(y, x)); } \
	__forceinline TYPE Lerp( TYPE a, TYPE b, TYPE t ) { return TYPE(DirectX::XMVectorLerpV(a, b, t)); } \
	__forceinline TYPE Max( TYPE a, TYPE b ) { return TYPE(DirectX::XMVectorMax(a, b)); } \
	__forceinline TYPE Min( TYPE a, TYPE b ) { return TYPE(DirectX::XMVectorMin(a, b)); } \
	__forceinline TYPE Clamp( TYPE v, TYPE a, TYPE b ) { return Min(Max(v, a), b); } \
	__forceinline BoolVector operator<  ( TYPE lhs, TYPE rhs ) { return DirectX::XMVectorLess(lhs, rhs); } \
	__forceinline BoolVector operator<= ( TYPE lhs, TYPE rhs ) { return DirectX::XMVectorLessOrEqual(lhs, rhs); } \
	__forceinline BoolVector operator>  ( TYPE lhs, TYPE rhs ) { return DirectX::XMVectorGreater(lhs, rhs); } \
	__forceinline BoolVector operator>= ( TYPE lhs, TYPE rhs ) { return DirectX::XMVectorGreaterOrEqual(lhs, rhs); } \
	__forceinline BoolVector operator== ( TYPE lhs, TYPE rhs ) { return DirectX::XMVectorEqual(lhs, rhs); } \
	__forceinline BoolVector operator!= ( TYPE lhs, TYPE rhs ) { return DirectX::XMVectorNotEqual(lhs, rhs); } \
	__forceinline TYPE Select( TYPE lhs, TYPE rhs, BoolVector mask ) { return TYPE(DirectX::XMVectorSelect(lhs, rhs, mask)); }


CREATE_SIMD_FUNCTIONS(Scalar)
CREATE_SIMD_FUNCTIONS(Vector3)
CREATE_SIMD_FUNCTIONS(Vector4)

#undef CREATE_SIMD_FUNCTIONS

__forceinline float Sqrt(float s) { return Sqrt(Scalar(s)); }
__forceinline float Recip(float s) { return Recip(Scalar(s)); }
__forceinline float RecipSqrt(float s) { return RecipSqrt(Scalar(s)); }
__forceinline float Floor(float s) { return Floor(Scalar(s)); }
__forceinline float Ceiling(float s) { return Ceiling(Scalar(s)); }
__forceinline float Round(float s) { return Round(Scalar(s)); }
__forceinline float Abs(float s) { return Abs(Scalar(s)); }
__forceinline float Exp(float s) { return Exp(Scalar(s)); }
__forceinline float Pow(float b, float e) { return Pow(Scalar(b), Scalar(e)); }
__forceinline float Log(float s) { return Log(Scalar(s)); }
__forceinline float Sin(float s) { return Sin(Scalar(s)); }
__forceinline float Cos(float s) { return Cos(Scalar(s)); }
__forceinline float Tan(float s) { return Tan(Scalar(s)); }
__forceinline float ASin(float s) { return ASin(Scalar(s)); }
__forceinline float ACos(float s) { return ACos(Scalar(s)); }
__forceinline float ATan(float s) { return ATan(Scalar(s)); }
__forceinline float ATan2(float y, float x) { return ATan2(Scalar(y), Scalar(x)); }
__forceinline float Lerp(float a, float b, float t) { return Lerp(Scalar(a), Scalar(b), Scalar(t)); }
__forceinline float Max(float a, float b) { return Max(Scalar(a), Scalar(b)); }
__forceinline float Min(float a, float b) { return Min(Scalar(a), Scalar(b)); }
__forceinline float Clamp(float v, float a, float b) { return Clamp(Scalar(v), Scalar(a), Scalar(b)); }

__forceinline Scalar Length(Vector3 v) { return Scalar(DirectX::XMVector3Length(v)); }
__forceinline Scalar LengthSquare(Vector3 v) { return Scalar(DirectX::XMVector3LengthSq(v)); }
__forceinline Scalar LengthRecip(Vector3 v) { return Scalar(DirectX::XMVector3ReciprocalLength(v)); }
__forceinline Scalar Dot(Vector3 v1, Vector3 v2) { return Scalar(DirectX::XMVector3Dot(v1, v2)); }
__forceinline Scalar Dot(Vector4 v1, Vector4 v2) { return Scalar(DirectX::XMVector4Dot(v1, v2)); }
__forceinline Vector3 Cross(Vector3 v1, Vector3 v2) { return Vector3(DirectX::XMVector3Cross(v1, v2)); }
__forceinline Vector3 Normalize(Vector3 v) { return Vector3(DirectX::XMVector3Normalize(v)); }
__forceinline Vector4 Normalize(Vector4 v) { return Vector4(DirectX::XMVector4Normalize(v)); }
__forceinline Quaternion Normalize(Quaternion q) { return Quaternion(DirectX::XMQuaternionNormalize(q)); }

__forceinline Matrix3 Transpose(const Matrix3& mat) { return Matrix3(DirectX::XMMatrixTranspose(mat)); }

// __forceinline Matrix3 Inverse( const Matrix3& mat ) { TBD }
// __forceinline Transform Inverse( const Transform& mat ) { TBD }

// This specialized matrix invert assumes that the 3x3 matrix is orthogonal (and normalized).
__forceinline AffineTransform OrthoInvert(const AffineTransform& xform)
{
	Matrix3 basis = Transpose(xform.GetBasis());
	return AffineTransform(basis, basis * -xform.GetTranslation());
}

__forceinline OrthogonalTransform Invert(const OrthogonalTransform& xform) { return ~xform; }

__forceinline Matrix4 Transpose(const Matrix4& mat) { return Matrix4(DirectX::XMMatrixTranspose(mat)); }
__forceinline Matrix4 Invert(const Matrix4& mat) { return Matrix4(DirectX::XMMatrixInverse(nullptr, mat)); }

__forceinline Matrix4 OrthoInvert(const Matrix4& xform)
{
	Matrix3 basis = Transpose(xform.Get3x3());
	Vector3 translate = basis * -Vector3(xform.GetW());
	return Matrix4(basis, translate);
}

} // namespace Math