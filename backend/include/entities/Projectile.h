/**
 * @file Projectile.h
 * @brief 投射物基类
 *
 * 所有投射物的基类：
 * - 直线移动
 * - 碰撞伤害
 * - 自动销毁
 */

#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "Entity.h"

// 前向声明
class Maze;

// 投射物类型
enum class ProjectileType {
    PEA,        // 豌豆
    // 可扩展其他投射物
};

/**
 * 投射物基类
 */
class Projectile : public Entity {
public:
    Projectile(float x, float y, ProjectileType projType, Direction dir, float speed, float damage);
    virtual ~Projectile() override;

    // 更新
    void update(float deltaTime) override;

    // 碰撞处理
    void onCollision(Entity* other) override;

    // 获取投射物类型
    ProjectileType getProjectileType() const { return projectileType_; }

    // 获取伤害值
    float getDamage() const { return damage_; }

    // 设置迷宫引用（用于墙壁碰撞检测）
    void setMaze(Maze* maze) { maze_ = maze; }

    // 序列化
    std::string toJson() const override;

protected:
    // 动画初始化
    void initializeAnimations() override;

    // 更新动画
    void updateAnimation() override;

private:
    ProjectileType projectileType_;
    float damage_;
    float lifetime_;        // 生命周期（超时自动销毁）
    float maxLifetime_;     // 最大生命周期
    Maze* maze_;            // 迷宫引用（用于墙壁碰撞检测）
};

#endif // PROJECTILE_H
