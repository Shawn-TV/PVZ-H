/**
 * @file Plant.h
 * @brief 植物基类
 *
 * 所有植物的基类：
 * - 固定位置（不移动，种植后位置固定）
 * - 攻击或阻挡逻辑
 * - 攻击范围和冷却
 * - 动画播放（待机、攻击等）
 * - 方向属性（用于定向攻击）
 */

#ifndef PLANT_H
#define PLANT_H

#include "Entity.h"

// 植物类型
enum class PlantType {
    PEA_SHOOTER,    // 豌豆射手
    CHERRY_BOMB,    // 樱桃炸弹
    WALL_NUT        // 坚果墙
};

// 前向声明
class EntityManager;

/**
 * 植物基类
 * 所有植物继承自Entity，但不移动
 */
class Plant : public Entity {
public:
    Plant(float x, float y, PlantType plantType);
    virtual ~Plant() override;

    // 更新（子类实现具体逻辑）
    void update(float deltaTime) override;

    // 碰撞处理
    void onCollision(Entity* other) override;

    // 获取植物类型
    PlantType getPlantType() const { return plantType_; }

    // 设置实体管理器（用于生成投射物等）
    void setEntityManager(EntityManager* manager) { entityManager_ = manager; }

    // 序列化
    std::string toJson() const override;

protected:
    // 植物类型
    PlantType plantType_;

    // 攻击属性
    float attackCooldown_;          // 攻击冷却时间
    float currentAttackCooldown_;   // 当前冷却计时器
    float attackRange_;             // 攻击范围
    float attackDamage_;            // 攻击伤害

    // 攻击方向（用于豌豆射手等定向植物）
    Direction attackDirection_;

    // 实体管理器引用（用于生成投射物、爆炸等）
    EntityManager* entityManager_;

    // 攻击逻辑（子类实现）
    virtual void performAttack() = 0;

    // 检测攻击范围内是否有目标
    bool hasTargetInRange() const;
};

#endif // PLANT_H
