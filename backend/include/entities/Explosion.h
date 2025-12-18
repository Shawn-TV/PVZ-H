/**
 * @file Explosion.h
 * @brief 爆炸效果类
 *
 * 功能：
 * - 范围伤害
 * - 短暂存在
 * - 爆炸动画
 */

#ifndef EXPLOSION_H
#define EXPLOSION_H

#include "Entity.h"

/**
 * 爆炸类
 * 对范围内所有实体造成伤害
 */
class Explosion : public Entity {
public:
    Explosion(float x, float y, float radius, float damage);
    ~Explosion() override;

    // 更新
    void update(float deltaTime) override;

    // 碰撞处理
    void onCollision(Entity* other) override;

    // 获取爆炸半径
    float getRadius() const { return radius_; }

    // 序列化
    std::string toJson() const override;

protected:
    // 动画初始化
    void initializeAnimations() override;

    // 更新动画
    void updateAnimation() override;

private:
    float radius_;          // 爆炸半径
    float damage_;          // 爆炸伤害
    float lifetime_;        // 已存在时间
    float maxLifetime_;     // 最大存在时间
    bool hasDealtDamage_;   // 是否已造成伤害（只造成一次）
};

#endif // EXPLOSION_H
