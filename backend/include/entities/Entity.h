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
#include "../utils/Animation.h"
#include <string>

// 实体类型枚举
enum class EntityType {
    ZOMBIE,
    DAVE,
    PLANT,
    ITEM,
    PROJECTILE,
    EXPLOSION
};

// 方向枚举
enum class Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NONE
};

/**
 * 实体基类
 */
class Entity {
public:
    Entity(float x, float y, EntityType type);
    virtual ~Entity();

    // 纯虚函数 - 子类必须实现
    virtual void update(float deltaTime) = 0;
    virtual void onCollision(Entity* other) = 0;
    virtual void initializeAnimations() = 0;
    virtual void updateAnimation() = 0;

    // 虚函数 - 子类可以覆盖
    virtual void takeDamage(float damage);
    virtual void heal(float amount);
    virtual std::string toJson() const;

    // 移动相关
    void move(const Vector2D& offset);
    void move(float dx, float dy);
    void setVelocity(const Vector2D& velocity);
    void setVelocity(float vx, float vy);
    void applyVelocity(float deltaTime);

    // Getters
    int getId() const { return id_; }
    EntityType getType() const { return type_; }
    Vector2D getPosition() const { return position_; }
    Vector2D getVelocity() const { return velocity_; }
    float getSpeed() const { return speed_; }
    Direction getDirection() const { return direction_; }
    float getHealth() const { return health_; }
    float getMaxHealth() const { return maxHealth_; }
    bool isAlive() const { return alive_; }
    float getWidth() const { return width_; }
    float getHeight() const { return height_; }

    // Setters
    void setPosition(float x, float y);
    void setPosition(const Vector2D& pos);
    void setHealth(float health);
    void setMaxHealth(float maxHealth) { maxHealth_ = maxHealth; }
    void setSize(float width, float height);

protected:
    // ID系统
    static int nextId_;
    int id_;

    // 实体类型
    EntityType type_;

    // 位置和移动
    Vector2D position_;
    Vector2D velocity_;
    float speed_;
    Direction direction_;

    // 生命值
    float health_;
    float maxHealth_;
    bool alive_;

    // 碰撞箱
    float width_;
    float height_;

    // 动画控制器
    AnimationController animationController_;
};

#endif // ENTITY_H
