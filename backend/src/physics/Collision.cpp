/**
 * @file Collision.cpp
 * @brief 碰撞检测系统实现
 */

#include "../../include/physics/Collision.h"
#include <cmath>

bool Collision::checkAABB(
    const Vector2D& pos1, float width1, float height1,
    const Vector2D& pos2, float width2, float height2
) {
    // AABB碰撞检测
    // 检查两个矩形是否重叠

    float left1 = pos1.x - width1 / 2.0f;
    float right1 = pos1.x + width1 / 2.0f;
    float top1 = pos1.y - height1 / 2.0f;
    float bottom1 = pos1.y + height1 / 2.0f;

    float left2 = pos2.x - width2 / 2.0f;
    float right2 = pos2.x + width2 / 2.0f;
    float top2 = pos2.y - height2 / 2.0f;
    float bottom2 = pos2.y + height2 / 2.0f;

    // 检查是否有重叠
    return !(right1 < left2 || left1 > right2 || bottom1 < top2 || top1 > bottom2);
}

bool Collision::checkCircle(
    const Vector2D& pos1, float radius1,
    const Vector2D& pos2, float radius2
) {
    // 圆形碰撞检测
    // 检查两个圆心距离是否小于半径之和

    float dx = pos2.x - pos1.x;
    float dy = pos2.y - pos1.y;
    float distanceSquared = dx * dx + dy * dy;
    float radiusSum = radius1 + radius2;

    return distanceSquared <= (radiusSum * radiusSum);
}

bool Collision::pointInRect(
    const Vector2D& point,
    const Vector2D& rectPos, float width, float height
) {
    // 点与矩形碰撞
    float left = rectPos.x - width / 2.0f;
    float right = rectPos.x + width / 2.0f;
    float top = rectPos.y - height / 2.0f;
    float bottom = rectPos.y + height / 2.0f;

    return (point.x >= left && point.x <= right &&
            point.y >= top && point.y <= bottom);
}
