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

    // 伤害处理（重写以添加50HP眩晕机制）
    void takeDamage(float damage) override;

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
    bool isStunned() const { return isStunned_; }

    // 玩家控制（多人模式）
    void setPlayerControlled(bool controlled) { isPlayerControlled_ = controlled; }
    bool isPlayerControlled() const { return isPlayerControlled_; }
    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();
    void stopMoving();

    // 种植植物功能
    bool plantPeaShooter(float x, float y, Direction shootDirection = Direction::RIGHT);
    bool plantDoublePeaShooter(float x, float y, Direction shootDirection = Direction::RIGHT);
    bool plantCherryBomb(float x, float y);
    bool plantWallNut(float x, float y);

    // 玩家种植功能（多人模式）
    void plantAtCurrentPosition(int plantType);  // 在当前位置种植
    void plantAtGridPosition(int plantType, int gridX, int gridY);  // 在指定格子种植
    bool canPlant(int plantType) const;          // 检查是否可以种植
    int getPlantCost(int plantType) const;       // 获取植物花费
    float getPlantCooldown(int plantType) const; // 获取植物冷却时间

    // 设置实体管理器（用于添加种植的植物）
    void setEntityManager(class EntityManager* manager) { entityManager_ = manager; }

    // 阳光系统
    int getSunlight() const { return sunlight_; }
    void addSunlight(int amount) { sunlight_ += amount; }
    bool canAffordPlant(int cost) const { return sunlight_ >= cost; }
    void updateSunlightGeneration(float deltaTime);  // 阳光生成更新

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
    bool lowHpStunTriggered_;  // 是否已触发50HP眩晕（防止重复触发）

    // 上一次的目标位置（用于判断目标是否移动）
    Vector2D lastTargetPosition_;

    // 实体管理器引用（用于种植植物）
    class EntityManager* entityManager_;

    // 种植相关
    float plantCooldown_;           // 全局种植冷却时间（AI用）
    float currentPlantCooldown_;    // 当前种植冷却计时器（AI用）
    int sunlight_;                  // 阳光数量
    float sunlightTimer_;           // 阳光生成计时器
    float sunlightInterval_;        // 阳光生成间隔（10秒）
    int sunlightPerInterval_;       // 每次生成的阳光数量（50）

    // 各植物冷却时间（玩家控制模式）
    // 植物类型: 0=豌豆射手, 1=双发射手, 2=樱桃炸弹, 3=坚果墙
    float peaShooterCooldown_;         // 豌豆射手冷却：10秒
    float repeaterCooldown_;           // 双发射手冷却：20秒
    float cherryBombCooldown_;         // 樱桃炸弹冷却：30秒
    float wallNutCooldown_;            // 坚果墙冷却：20秒
    float currentPeaShooterCooldown_;
    float currentRepeaterCooldown_;
    float currentCherryBombCooldown_;
    float currentWallNutCooldown_;

    // 玩家控制相关
    bool isPlayerControlled_;       // 是否由玩家控制
    Vector2D inputDirection_;       // 玩家输入方向
    bool isMovingInput_;            // 是否有移动输入

    // 内部辅助函数
    void updateAI(float deltaTime);
    void updatePlayerControl(float deltaTime);  // 玩家控制更新
    void followPath(float deltaTime);
    void attackTarget();
    bool canSeeTarget() const;
    float distanceToTarget() const;
    void moveTowardsPosition(const Vector2D& targetPos, float deltaTime);
    void setState(DaveState newState);

    // 植物种植AI
    void updatePlantingAI(float deltaTime);
    bool findOptimalPlantLocation(int& gridX, int& gridY, Direction& plantDirection);
    bool isCorridorCell(int gridX, int gridY) const;
    bool isCellOnZombiePath(int gridX, int gridY) const;
    int calculatePlantScore(int gridX, int gridY) const;
    bool isZombieStuckByWalnut() const;
    bool hasWalnutOnZombiePath() const;
    bool findPositionBetween(int& gridX, int& gridY, const Vector2D& pos1, const Vector2D& pos2) const;

    // AI状态追踪
    Vector2D lastZombiePosition_;    // 上次僵尸位置（检测是否卡住）
    float zombieStuckTimer_;         // 僵尸卡住计时器
    bool hasPlacedWalnut_;           // 是否已在路径上放置坚果
};

#endif // DAVE_H
