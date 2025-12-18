/**
 * @file Vector2D.h
 * @brief 2D向量类
 *
 * 功能：
 * - 位置和方向表示
 * - 向量运算（加减乘除）
 * - 距离和长度计算
 */

#ifndef VECTOR2D_H
#define VECTOR2D_H

#include <cmath>

class Vector2D {
public:
    float x;
    float y;

    Vector2D();
    Vector2D(float x, float y);

    // 向量运算
    Vector2D operator+(const Vector2D& other) const;
    Vector2D operator-(const Vector2D& other) const;
    Vector2D operator*(float scalar) const;
    Vector2D operator/(float scalar) const;

    // 辅助函数
    float length() const;
    float lengthSquared() const;
    Vector2D normalized() const;
    float dot(const Vector2D& other) const;
    float distance(const Vector2D& other) const;
};

#endif // VECTOR2D_H
