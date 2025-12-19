/**
 * @file Dave.h
 * @brief 戴夫NPC类
 *
 * 负责：
 * - 使用A*算法追踪僵尸
 * - 自动移动和攻击
 * - 路径规划和导航
 * - 动画播放（行走、攻击等）
 */

#ifndef DAVE_H
#define DAVE_H

#include "Entity.h"
#include <vector>

class Zombie;
class Maze;

// 戴夫状态
enum class DaveState {
    IDLE,       // 待机
    CHASING,    // 追逐
    ATTACKING,  // 攻击
    STUNNED,    // 眩晕（被植物攻击等）
    PLANTING    // 种植植物
};

/**
 * 戴夫类 - 智能NPC，使用A*算法追踪玩家
 */
class Dave : public Entity {
public:
    Dave(float x, float y, Maze* maze);
    ~Dave() override;

    // 核心更新函数
    void update(float deltaTime) override;

    // 碰撞处理
    void onCollision(Entity* other) override;

    // 设置追踪目标
    void setTarget(Zombie* zombie);
    Zombie* getTarget() const { return target_; }

    // 路径规划
    void updatePath();
    void clearPath();
    bool hasPath() const { return !path_.empty(); }

    // 攻击相关
    float getAttackRange() const { return attackRange_; }
    float getAttackDamage() const { return attackDamage_; }

    // 状态
    DaveState getState() const { return state_; }

    // 眩晕
    void stun(float duration);

    // 种植植物功能
    void plantPeaShooter(float x, float y, Direction shootDirection = Direction::RIGHT);
    void plantDoublePeaShooter(float x, float y, Direction shootDirection = Direction::RIGHT);
    void plantCherryBomb(float x, float y);
    void plantWallNut(float x, float y);

    // 设置实体管理器（用于添加种植的植物）
    void setEntityManager(class EntityManager* manager) { entityManager_ = manager; }

    // 阳光系统
    int getSunlight() const { return sunlight_; }
    void addSunlight(int amount) { sunlight_ += amount; }
    bool canAffordPlant(int cost) const { return sunlight_ >= cost; }

    // 序列化
    std::string toJson() const override;

protected:
    // 动画初始化
    void initializeAnimations() override;

    // 更新动画状态
    void updateAnimation() override;

private:
    // AI相关
    Maze* maze_;
    Zombie* target_;
    std::vector<Vector2D> path_;
    int currentPathIndex_;
    DaveState state_;

    // 属性
    float attackDamage_;
    float attackRange_;
    float attackCooldown_;
    float currentAttackCooldown_;

    // 路径更新计时器
    float pathUpdateTimer_;
    float pathUpdateInterval_;  // 多久重新计算一次路径

    // 检测范围
    float detectionRange_;

    // 眩晕状态
    bool isStunned_;
    float stunTimer_;

    // 上一次的目标位置（用于判断目标是否移动）
    Vector2D lastTargetPosition_;

    // 实体管理器引用（用于种植植物）
    class EntityManager* entityManager_;

    // 种植相关
    float plantCooldown_;           // 种植冷却时间
    float currentPlantCooldown_;    // 当前种植冷却计时器
    int sunlight_;                  // 阳光数量

    // 内部辅助函数
    void updateAI(float deltaTime);
    void followPath(float deltaTime);
    void attackTarget();
    bool canSeeTarget() const;
    float distanceToTarget() const;
    void moveTowardsPosition(const Vector2D& targetPos, float deltaTime);
    void setState(DaveState newState);
};

#endif // DAVE_H
