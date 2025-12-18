/**
 * @file Collision.h
 * @brief 碰撞检测系统
 *
 * 功能：
 * - AABB碰撞检测
 * - 圆形碰撞检测
 * - 碰撞响应
 */

#ifndef COLLISION_H
#define COLLISION_H

#include "Vector2D.h"

class Entity;

class Collision {
public:
    // 矩形碰撞检测 (AABB)
    static bool checkAABB(
        const Vector2D& pos1, float width1, float height1,
        const Vector2D& pos2, float width2, float height2
    );

    // 圆形碰撞检测
    static bool checkCircle(
        const Vector2D& pos1, float radius1,
        const Vector2D& pos2, float radius2
    );

    // 点与矩形碰撞
    static bool pointInRect(
        const Vector2D& point,
        const Vector2D& rectPos, float width, float height
    );
};

#endif // COLLISION_H
