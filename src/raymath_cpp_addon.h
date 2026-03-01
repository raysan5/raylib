/**********************************************************************************************
*
*   raymath_addon.h - C++ Operator Overloads for raymath
*
*   This file contains C++ specific operator overloads for raymath types.
*   It is automatically included by raymath.h when compiling as C++.
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2015-2026 Ramon Santamaria (@raysan5)
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

#ifndef RAYMATH_ADDON_H
#define RAYMATH_ADDON_H

#if defined(__cplusplus) && !defined(RAYMATH_DISABLE_CPP_OPERATORS)

#include "raymath.h"

// Optional C++ math operators
//-------------------------------------------------------------------------------

// Vector2 operators
RMAPI Vector2 Vector2Zeros = {0, 0};
RMAPI Vector2 Vector2Ones = {1, 1};
RMAPI Vector2 Vector2UnitX = {1, 0};
RMAPI Vector2 Vector2UnitY = {0, 1};

RMAPI Vector2 operator+(const Vector2 &lhs, const Vector2 &rhs) {
  return Vector2Add(lhs, rhs);
}

RMAPI const Vector2 &operator+=(Vector2 &lhs, const Vector2 &rhs) {
  lhs = Vector2Add(lhs, rhs);
  return lhs;
}

RMAPI Vector2 operator-(const Vector2 &lhs, const Vector2 &rhs) {
  return Vector2Subtract(lhs, rhs);
}

RMAPI const Vector2 &operator-=(Vector2 &lhs, const Vector2 &rhs) {
  lhs = Vector2Subtract(lhs, rhs);
  return lhs;
}

RMAPI Vector2 operator*(const Vector2 &lhs, const float &rhs) {
  return Vector2Scale(lhs, rhs);
}

RMAPI const Vector2 &operator*=(Vector2 &lhs, const float &rhs) {
  lhs = Vector2Scale(lhs, rhs);
  return lhs;
}

RMAPI Vector2 operator*(const Vector2 &lhs, const Vector2 &rhs) {
  return Vector2Multiply(lhs, rhs);
}

RMAPI const Vector2 &operator*=(Vector2 &lhs, const Vector2 &rhs) {
  lhs = Vector2Multiply(lhs, rhs);
  return lhs;
}

RMAPI Vector2 operator*(const Vector2 &lhs, const Matrix &rhs) {
  return Vector2Transform(lhs, rhs);
}

RMAPI const Vector2 &operator*=(Vector2 &lhs, const Matrix &rhs) {
  lhs = Vector2Transform(lhs, rhs);
  return lhs;
}

RMAPI Vector2 operator/(const Vector2 &lhs, const float &rhs) {
  return Vector2Scale(lhs, 1.0f / rhs);
}

RMAPI const Vector2 &operator/=(Vector2 &lhs, const float &rhs) {
  lhs = Vector2Scale(lhs, 1.0f / rhs);
  return lhs;
}

RMAPI Vector2 operator/(const Vector2 &lhs, const Vector2 &rhs) {
  return Vector2Divide(lhs, rhs);
}

RMAPI const Vector2 &operator/=(Vector2 &lhs, const Vector2 &rhs) {
  lhs = Vector2Divide(lhs, rhs);
  return lhs;
}

RMAPI bool operator==(const Vector2 &lhs, const Vector2 &rhs) {
  return FloatEquals(lhs.x, rhs.x) && FloatEquals(lhs.y, rhs.y);
}

RMAPI bool operator!=(const Vector2 &lhs, const Vector2 &rhs) {
  return !FloatEquals(lhs.x, rhs.x) || !FloatEquals(lhs.y, rhs.y);
}

// Vector3 operators
RMAPI Vector3 Vector3Zeros = {0, 0, 0};
RMAPI Vector3 Vector3Ones = {1, 1, 1};
RMAPI Vector3 Vector3UnitX = {1, 0, 0};
RMAPI Vector3 Vector3UnitY = {0, 1, 0};
RMAPI Vector3 Vector3UnitZ = {0, 0, 1};

RMAPI Vector3 operator+(const Vector3 &lhs, const Vector3 &rhs) {
  return Vector3Add(lhs, rhs);
}

RMAPI const Vector3 &operator+=(Vector3 &lhs, const Vector3 &rhs) {
  lhs = Vector3Add(lhs, rhs);
  return lhs;
}

RMAPI Vector3 operator-(const Vector3 &lhs, const Vector3 &rhs) {
  return Vector3Subtract(lhs, rhs);
}

RMAPI const Vector3 &operator-=(Vector3 &lhs, const Vector3 &rhs) {
  lhs = Vector3Subtract(lhs, rhs);
  return lhs;
}

RMAPI Vector3 operator*(const Vector3 &lhs, const float &rhs) {
  return Vector3Scale(lhs, rhs);
}

RMAPI const Vector3 &operator*=(Vector3 &lhs, const float &rhs) {
  lhs = Vector3Scale(lhs, rhs);
  return lhs;
}

RMAPI Vector3 operator*(const Vector3 &lhs, const Vector3 &rhs) {
  return Vector3Multiply(lhs, rhs);
}

RMAPI const Vector3 &operator*=(Vector3 &lhs, const Vector3 &rhs) {
  lhs = Vector3Multiply(lhs, rhs);
  return lhs;
}

RMAPI Vector3 operator*(const Vector3 &lhs, const Matrix &rhs) {
  return Vector3Transform(lhs, rhs);
}

RMAPI const Vector3 &operator*=(Vector3 &lhs, const Matrix &rhs) {
  lhs = Vector3Transform(lhs, rhs);
  return lhs;
}

RMAPI Vector3 operator/(const Vector3 &lhs, const float &rhs) {
  return Vector3Scale(lhs, 1.0f / rhs);
}

RMAPI const Vector3 &operator/=(Vector3 &lhs, const float &rhs) {
  lhs = Vector3Scale(lhs, 1.0f / rhs);
  return lhs;
}

RMAPI Vector3 operator/(const Vector3 &lhs, const Vector3 &rhs) {
  return Vector3Divide(lhs, rhs);
}

RMAPI const Vector3 &operator/=(Vector3 &lhs, const Vector3 &rhs) {
  lhs = Vector3Divide(lhs, rhs);
  return lhs;
}

RMAPI bool operator==(const Vector3 &lhs, const Vector3 &rhs) {
  return FloatEquals(lhs.x, rhs.x) && FloatEquals(lhs.y, rhs.y) &&
         FloatEquals(lhs.z, rhs.z);
}

RMAPI bool operator!=(const Vector3 &lhs, const Vector3 &rhs) {
  return !FloatEquals(lhs.x, rhs.x) || !FloatEquals(lhs.y, rhs.y) ||
         !FloatEquals(lhs.z, rhs.z);
}

// Vector4 operators
RMAPI Vector4 Vector4Zeros = {0, 0, 0, 0};
RMAPI Vector4 Vector4Ones = {1, 1, 1, 1};
RMAPI Vector4 Vector4UnitX = {1, 0, 0, 0};
RMAPI Vector4 Vector4UnitY = {0, 1, 0, 0};
RMAPI Vector4 Vector4UnitZ = {0, 0, 1, 0};
RMAPI Vector4 Vector4UnitW = {0, 0, 0, 1};

RMAPI Vector4 operator+(const Vector4 &lhs, const Vector4 &rhs) {
  return Vector4Add(lhs, rhs);
}

RMAPI const Vector4 &operator+=(Vector4 &lhs, const Vector4 &rhs) {
  lhs = Vector4Add(lhs, rhs);
  return lhs;
}

RMAPI Vector4 operator-(const Vector4 &lhs, const Vector4 &rhs) {
  return Vector4Subtract(lhs, rhs);
}

RMAPI const Vector4 &operator-=(Vector4 &lhs, const Vector4 &rhs) {
  lhs = Vector4Subtract(lhs, rhs);
  return lhs;
}

RMAPI Vector4 operator*(const Vector4 &lhs, const float &rhs) {
  return Vector4Scale(lhs, rhs);
}

RMAPI const Vector4 &operator*=(Vector4 &lhs, const float &rhs) {
  lhs = Vector4Scale(lhs, rhs);
  return lhs;
}

RMAPI Vector4 operator*(const Vector4 &lhs, const Vector4 &rhs) {
  return Vector4Multiply(lhs, rhs);
}

RMAPI const Vector4 &operator*=(Vector4 &lhs, const Vector4 &rhs) {
  lhs = Vector4Multiply(lhs, rhs);
  return lhs;
}

RMAPI Vector4 operator/(const Vector4 &lhs, const float &rhs) {
  return Vector4Scale(lhs, 1.0f / rhs);
}

RMAPI const Vector4 &operator/=(Vector4 &lhs, const float &rhs) {
  lhs = Vector4Scale(lhs, 1.0f / rhs);
  return lhs;
}

RMAPI Vector4 operator/(const Vector4 &lhs, const Vector4 &rhs) {
  return Vector4Divide(lhs, rhs);
}

RMAPI const Vector4 &operator/=(Vector4 &lhs, const Vector4 &rhs) {
  lhs = Vector4Divide(lhs, rhs);
  return lhs;
}

RMAPI bool operator==(const Vector4 &lhs, const Vector4 &rhs) {
  return FloatEquals(lhs.x, rhs.x) && FloatEquals(lhs.y, rhs.y) &&
         FloatEquals(lhs.z, rhs.z) && FloatEquals(lhs.w, rhs.w);
}

RMAPI bool operator!=(const Vector4 &lhs, const Vector4 &rhs) {
  return !FloatEquals(lhs.x, rhs.x) || !FloatEquals(lhs.y, rhs.y) ||
         !FloatEquals(lhs.z, rhs.z) || !FloatEquals(lhs.w, rhs.w);
}

// Quaternion operators
RMAPI Quaternion QuaternionZeros = {0, 0, 0, 0};
RMAPI Quaternion QuaternionOnes = {1, 1, 1, 1};
RMAPI Quaternion QuaternionUnitX = {0, 0, 0, 1};

RMAPI Quaternion operator+(const Quaternion &lhs, const float &rhs) {
  return QuaternionAddValue(lhs, rhs);
}

RMAPI const Quaternion &operator+=(Quaternion &lhs,
                                              const float &rhs) {
  lhs = QuaternionAddValue(lhs, rhs);
  return lhs;
}

RMAPI Quaternion operator-(const Quaternion &lhs, const float &rhs) {
  return QuaternionSubtractValue(lhs, rhs);
}

RMAPI const Quaternion &operator-=(Quaternion &lhs,
                                              const float &rhs) {
  lhs = QuaternionSubtractValue(lhs, rhs);
  return lhs;
}

RMAPI Quaternion operator*(const Quaternion &lhs,
                                      const Matrix &rhs) {
  return QuaternionTransform(lhs, rhs);
}

RMAPI const Quaternion &operator*=(Quaternion &lhs,
                                              const Matrix &rhs) {
  lhs = QuaternionTransform(lhs, rhs);
  return lhs;
}

// Matrix operators
RMAPI Matrix MatrixZeros = {};
RMAPI Matrix MatrixUnit = { 1, 0, 0, 0, 
                                       0, 1, 0, 0,
                                       0, 0, 1, 0, 
                                       0, 0, 0, 1 };
RMAPI Matrix operator+(const Matrix &lhs, const Matrix &rhs) {
  return MatrixAdd(lhs, rhs);
}

RMAPI const Matrix &operator+=(Matrix &lhs, const Matrix &rhs) {
  lhs = MatrixAdd(lhs, rhs);
  return lhs;
}

RMAPI Matrix operator-(const Matrix &lhs, const Matrix &rhs) {
  return MatrixSubtract(lhs, rhs);
}

RMAPI const Matrix &operator-=(Matrix &lhs, const Matrix &rhs) {
  lhs = MatrixSubtract(lhs, rhs);
  return lhs;
}

RMAPI Matrix operator*(const Matrix &lhs, const Matrix &rhs) {
  return MatrixMultiply(lhs, rhs);
}

RMAPI const Matrix &operator*=(Matrix &lhs, const Matrix &rhs) {
  lhs = MatrixMultiply(lhs, rhs);
  return lhs;
}

RMAPI Matrix operator*(const Matrix &lhs, const float value) {
  return MatrixMultiplyValue(lhs, value);
}

RMAPI const Matrix &operator*=(Matrix &lhs, const float value) {
  lhs = MatrixMultiplyValue(lhs, value);
  return lhs;
}

//-------------------------------------------------------------------------------
#endif // C++ operators

#endif // RAYMATH_ADDON_H
