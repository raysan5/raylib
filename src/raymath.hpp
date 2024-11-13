/**********************************************************************************************
*
*   raymath v2.0 C++ operators - Math functions to work with Vector2, Vector3, Matrix and Quaternions
*
*   CONVENTIONS:
*     - Matrix structure is defined as row-major (memory layout) but parameters naming AND all
*       math operations performed by the library consider the structure as it was column-major
*       It is like transposed versions of the matrices are used for all the maths
*       It benefits some functions making them cache-friendly and also avoids matrix
*       transpositions sometimes required by OpenGL
*       Example: In memory order, row0 is [m0 m4 m8 m12] but in semantic math row0 is [m0 m1 m2 m3]
*     - Functions are always self-contained, no function use another raymath function inside,
*       required code is directly re-implemented inside
*     - Functions input parameters are always received by value (2 unavoidable exceptions)
*     - Functions use always a "result" variable for return (except C++ operators)
*     - Functions are always defined inline
*     - Angles are always in radians (DEG2RAD/RAD2DEG macros provided for convenience)
*     - No compound literals used to make sure libray is compatible with C++
*
*   CONFIGURATION:
*       #define RAYMATH_IMPLEMENTATION
*           Generates the implementation of the library into the included file.
*           If not defined, the library is in header only mode and can be included in other headers
*           or source files without problems. But only ONE file should hold the implementation.
*
*       #define RAYMATH_STATIC_INLINE
*           Define static inline functions code, so #include header suffices for use.
*           This may use up lots of memory.
*
*       #define RAYMATH_DISABLE_CPP_OPERATORS
*           Disables C++ operator overloads for raymath types.
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2015-2024 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#ifndef RAYMATH_HPP
#define RAYMATH_HPP

// Optional C++ math operators
//-------------------------------------------------------------------------------

// Vector2 operators
static constexpr Vector2 Vector2Zeros = { 0, 0 };
static constexpr Vector2 Vector2Ones = { 1, 1 };
static constexpr Vector2 Vector2UnitX = { 1, 0 };
static constexpr Vector2 Vector2UnitY = { 0, 1 };

inline Vector2 operator + (const Vector2& lhs, const Vector2& rhs)
{
    return Vector2Add(lhs, rhs);
}

inline const Vector2& operator += (Vector2& lhs, const Vector2& rhs)
{
    lhs = Vector2Add(lhs, rhs);
    return lhs;
}

inline Vector2 operator - (const Vector2& lhs, const Vector2& rhs)
{
    return Vector2Subtract(lhs, rhs);
}

inline const Vector2& operator -= (Vector2& lhs, const Vector2& rhs)
{
    lhs = Vector2Subtract(lhs, rhs);
    return lhs;
}

inline Vector2 operator * (const Vector2& lhs, const float& rhs)
{
    return Vector2Scale(lhs, rhs);
}

inline const Vector2& operator *= (Vector2& lhs, const float& rhs)
{
    lhs = Vector2Scale(lhs, rhs);
    return lhs;
}

inline Vector2 operator * (const Vector2& lhs, const Vector2& rhs)
{
    return Vector2Multiply(lhs, rhs);
}

inline const Vector2& operator *= (Vector2& lhs, const Vector2& rhs)
{
    lhs = Vector2Multiply(lhs, rhs);
    return lhs;
}

inline Vector2 operator * (const Vector2& lhs, const Matrix& rhs)
{
    return Vector2Transform(lhs, rhs);
}

inline const Vector2& operator -= (Vector2& lhs, const Matrix& rhs)
{
    lhs = Vector2Transform(lhs, rhs);
    return lhs;
}

inline Vector2 operator / (const Vector2& lhs, const float& rhs)
{
    return Vector2Scale(lhs, 1.0f / rhs);
}

inline const Vector2& operator /= (Vector2& lhs, const float& rhs)
{
    lhs = Vector2Scale(lhs, rhs);
    return lhs;
}

inline Vector2 operator / (const Vector2& lhs, const Vector2& rhs)
{
    return Vector2Divide(lhs, rhs);
}

inline const Vector2& operator /= (Vector2& lhs, const Vector2& rhs)
{
    lhs = Vector2Divide(lhs, rhs);
    return lhs;
}

inline bool operator == (const Vector2& lhs, const Vector2& rhs)
{
    return FloatEquals(lhs.x, rhs.x) && FloatEquals(lhs.y, rhs.y);
}

inline bool operator != (const Vector2& lhs, const Vector2& rhs)
{
    return !FloatEquals(lhs.x, rhs.x) || !FloatEquals(lhs.y, rhs.y);
}

// Vector3 operators
static constexpr Vector3 Vector3Zeros = { 0, 0, 0 };
static constexpr Vector3 Vector3Ones = { 1, 1, 1 };
static constexpr Vector3 Vector3UnitX = { 1, 0, 0 };
static constexpr Vector3 Vector3UnitY = { 0, 1, 0 };
static constexpr Vector3 Vector3UnitZ = { 0, 0, 1 };

inline Vector3 operator + (const Vector3& lhs, const Vector3& rhs)
{
    return Vector3Add(lhs, rhs);
}

inline const Vector3& operator += (Vector3& lhs, const Vector3& rhs)
{
    lhs = Vector3Add(lhs, rhs);
    return lhs;
}

inline Vector3 operator - (const Vector3& lhs, const Vector3& rhs)
{
    return Vector3Subtract(lhs, rhs);
}

inline const Vector3& operator -= (Vector3& lhs, const Vector3& rhs)
{
    lhs = Vector3Subtract(lhs, rhs);
    return lhs;
}

inline Vector3 operator * (const Vector3& lhs, const float& rhs)
{
    return Vector3Scale(lhs, rhs);
}

inline const Vector3& operator *= (Vector3& lhs, const float& rhs)
{
    lhs = Vector3Scale(lhs, rhs);
    return lhs;
}

inline Vector3 operator * (const Vector3& lhs, const Vector3& rhs)
{
    return Vector3Multiply(lhs, rhs);
}

inline const Vector3& operator *= (Vector3& lhs, const Vector3& rhs)
{
    lhs = Vector3Multiply(lhs, rhs);
    return lhs;
}

inline Vector3 operator * (const Vector3& lhs, const Matrix& rhs)
{
    return Vector3Transform(lhs, rhs);
}

inline const Vector3& operator -= (Vector3& lhs, const Matrix& rhs)
{
    lhs = Vector3Transform(lhs, rhs);
    return lhs;
}

inline Vector3 operator / (const Vector3& lhs, const float& rhs)
{
    return Vector3Scale(lhs, 1.0f / rhs);
}

inline const Vector3& operator /= (Vector3& lhs, const float& rhs)
{
    lhs = Vector3Scale(lhs, rhs);
    return lhs;
}

inline Vector3 operator / (const Vector3& lhs, const Vector3& rhs)
{
    return Vector3Divide(lhs, rhs);
}

inline const Vector3& operator /= (Vector3& lhs, const Vector3& rhs)
{
    lhs = Vector3Divide(lhs, rhs);
    return lhs;
}

inline bool operator == (const Vector3& lhs, const Vector3& rhs)
{
    return FloatEquals(lhs.x, rhs.x) && FloatEquals(lhs.y, rhs.y) && FloatEquals(lhs.z, rhs.z);
}

inline bool operator != (const Vector3& lhs, const Vector3& rhs)
{
    return !FloatEquals(lhs.x, rhs.x) || !FloatEquals(lhs.y, rhs.y) || !FloatEquals(lhs.z, rhs.z);
}

// Vector4 operators
static constexpr Vector4 Vector4Zeros = { 0, 0, 0, 0 };
static constexpr Vector4 Vector4Ones = { 1, 1, 1, 1 };
static constexpr Vector4 Vector4UnitX = { 1, 0, 0, 0 };
static constexpr Vector4 Vector4UnitY = { 0, 1, 0, 0 };
static constexpr Vector4 Vector4UnitZ = { 0, 0, 1, 0 };
static constexpr Vector4 Vector4UnitW = { 0, 0, 0, 1 };

inline Vector4 operator + (const Vector4& lhs, const Vector4& rhs)
{
    return Vector4Add(lhs, rhs);
}

inline const Vector4& operator += (Vector4& lhs, const Vector4& rhs)
{
    lhs = Vector4Add(lhs, rhs);
    return lhs;
}

inline Vector4 operator - (const Vector4& lhs, const Vector4& rhs)
{
    return Vector4Subtract(lhs, rhs);
}

inline const Vector4& operator -= (Vector4& lhs, const Vector4& rhs)
{
    lhs = Vector4Subtract(lhs, rhs);
    return lhs;
}

inline Vector4 operator * (const Vector4& lhs, const float& rhs)
{
    return Vector4Scale(lhs, rhs);
}

inline const Vector4& operator *= (Vector4& lhs, const float& rhs)
{
    lhs = Vector4Scale(lhs, rhs);
    return lhs;
}

inline Vector4 operator * (const Vector4& lhs, const Vector4& rhs)
{
    return Vector4Multiply(lhs, rhs);
}

inline const Vector4& operator *= (Vector4& lhs, const Vector4& rhs)
{
    lhs = Vector4Multiply(lhs, rhs);
    return lhs;
}

inline Vector4 operator / (const Vector4& lhs, const float& rhs)
{
    return Vector4Scale(lhs, 1.0f / rhs);
}

inline const Vector4& operator /= (Vector4& lhs, const float& rhs)
{
    lhs = Vector4Scale(lhs, rhs);
    return lhs;
}

inline Vector4 operator / (const Vector4& lhs, const Vector4& rhs)
{
    return Vector4Divide(lhs, rhs);
}

inline const Vector4& operator /= (Vector4& lhs, const Vector4& rhs)
{
    lhs = Vector4Divide(lhs, rhs);
    return lhs;
}

inline bool operator == (const Vector4& lhs, const Vector4& rhs)
{
    return FloatEquals(lhs.x, rhs.x) && FloatEquals(lhs.y, rhs.y) && FloatEquals(lhs.z, rhs.z) && FloatEquals(lhs.w, rhs.w);
}

inline bool operator != (const Vector4& lhs, const Vector4& rhs)
{
    return !FloatEquals(lhs.x, rhs.x) || !FloatEquals(lhs.y, rhs.y) || !FloatEquals(lhs.z, rhs.z) || !FloatEquals(lhs.w, rhs.w);
}

// Quaternion operators
static constexpr Quaternion QuaternionZeros = { 0, 0, 0, 0 };
static constexpr Quaternion QuaternionOnes = { 1, 1, 1, 1 };
static constexpr Quaternion QuaternionUnitX = { 0, 0, 0, 1 };

inline Quaternion operator + (const Quaternion& lhs, const float& rhs)
{
    return QuaternionAddValue(lhs, rhs);
}

inline const Quaternion& operator += (Quaternion& lhs, const float& rhs)
{
    lhs = QuaternionAddValue(lhs, rhs);
    return lhs;
}

inline Quaternion operator - (const Quaternion& lhs, const float& rhs)
{
    return QuaternionSubtractValue(lhs, rhs);
}

inline const Quaternion& operator -= (Quaternion& lhs, const float& rhs)
{
    lhs = QuaternionSubtractValue(lhs, rhs);
    return lhs;
}

inline Quaternion operator * (const Quaternion& lhs, const Matrix& rhs)
{
    return QuaternionTransform(lhs, rhs);
}

inline const Quaternion& operator *= (Quaternion& lhs, const Matrix& rhs)
{
    lhs = QuaternionTransform(lhs, rhs);
    return lhs;
}

// Matrix operators
inline Matrix operator + (const Matrix& lhs, const Matrix& rhs)
{
    return MatrixAdd(lhs, rhs);
}

inline const Matrix& operator += (Matrix& lhs, const Matrix& rhs)
{
    lhs = MatrixAdd(lhs, rhs);
    return lhs;
}

inline Matrix operator - (const Matrix& lhs, const Matrix& rhs)
{
    return MatrixSubtract(lhs, rhs);
}

inline const Matrix& operator -= (Matrix& lhs, const Matrix& rhs)
{
    lhs = MatrixSubtract(lhs, rhs);
    return lhs;
}

inline Matrix operator * (const Matrix& lhs, const Matrix& rhs)
{
    return MatrixMultiply(lhs, rhs);
}

inline const Matrix& operator *= (Matrix& lhs, const Matrix& rhs)
{
    lhs = MatrixMultiply(lhs, rhs);
    return lhs;
}
//-------------------------------------------------------------------------------
#endif RAYMATH_HPP
