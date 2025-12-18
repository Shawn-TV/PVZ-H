/**
 * @file Plant.h
 * @brief 植物基类
 *
 * 所有植物的基类：
 * - 固定位置（不移动）
 * - 攻击或阻挡逻辑
 * - 攻击范围和冷却
 */

#ifndef PLANT_H
#define PLANT_H

#include "Entity.h"

enum class PlantType {
    PEA_SHOOTER,
    CHERRY_BOMB,
    WALL_NUT
};

class Plant : public Entity {
public:
    Plant(float x, float y, PlantType type);
    virtual ~Plant();

    void update(float deltaTime) override;
    void onCollision(Entity* other) override;

    PlantType getType() const;

protected:
    PlantType type_;
    float attackCooldown_;
    float attackRange_;
    int attackDamage_;
};

#endif // PLANT_H
