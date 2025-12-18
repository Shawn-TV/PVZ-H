/**
 * @file Vector2D.cpp
 * @brief 2D向量类实现
 */

#include "../../include/physics/Vector2D.h"

Vector2D::Vector2D() : x(0.0f), y(0.0f) {
}

Vector2D::Vector2D(float x, float y) : x(x), y(y) {
}

Vector2D Vector2D::operator+(const Vector2D& other) const {
    return Vector2D(x + other.x, y + other.y);
}

Vector2D Vector2D::operator-(const Vector2D& other) const {
    return Vector2D(x - other.x, y - other.y);
}

Vector2D Vector2D::operator*(float scalar) const {
    return Vector2D(x * scalar, y * scalar);
}

Vector2D Vector2D::operator/(float scalar) const {
    return Vector2D(x / scalar, y / scalar);
}

float Vector2D::length() const {
    return std::sqrt(x * x + y * y);
}

float Vector2D::lengthSquared() const {
    return x * x + y * y;
}

Vector2D Vector2D::normalized() const {
    float len = length();
    if (len > 0) {
        return *this / len;
    }
    return Vector2D(0, 0);
}

float Vector2D::dot(const Vector2D& other) const {
    return x * other.x + y * other.y;
}

float Vector2D::distance(const Vector2D& other) const {
    return (*this - other).length();
}
