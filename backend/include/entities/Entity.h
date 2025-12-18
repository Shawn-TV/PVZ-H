/**
 * @file Entity.h
 * @brief 实体基类
 *
 * 所有游戏实体的基类：
 * - 位置和移动
 * - 生命值
 * - 碰撞检测
 * - 更新和渲染接口
 */

#ifndef ENTITY_H
#define ENTITY_H

#include "../physics/Vector2D.h"

class Entity {
public:
    Entity(float x, float y);
    virtual ~Entity();

    virtual void update(float deltaTime) = 0;
    virtual void onCollision(Entity* other) = 0;

    // Getters and Setters
    Vector2D getPosition() const;
    void setPosition(float x, float y);

    bool isAlive() const;

protected:
    Vector2D position_;
    float health_;
    bool alive_;
};

#endif // ENTITY_H
