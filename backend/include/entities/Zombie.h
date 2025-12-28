/**
 * @file Zombie.h
 * @brief 僵尸类（玩家控制）
 *
 * 负责：
 * - 玩家输入响应
 * - 移动控制
 * - 拾取道具
 * - 受到植物和戴夫的攻击
 * - 装备效果（铁桶等）
 * - 动画播放（行走、待机、攻击、死亡等）
 */

#ifndef ZOMBIE_H
#define ZOMBIE_H

#include "Entity.h"
#include <vector>
#include <map>

class Item;
class EntityManager;
class Maze;

// 僵尸形态枚举
enum class ZombieForm {
    NORMAL,         // 普通僵尸
    POLE_VAULTER,   // 撑杆跳僵尸（跑动）
    BUCKET          // 铁桶僵尸（戴铁桶）
};

// 僵尸状态
enum class ZombieState {
    IDLE,       // 待机
    WALKING,    // 行走
    RUNNING,    // 跑动（撑杆跳僵尸）
    JUMPING,    // 撑杆跳跃中
    EATING,     // 吃植物
    DAMAGED,    // 受伤
    DEAD        // 死亡
};

/**
 * 僵尸类 - 玩家控制的角色
 */
class Zombie : public Entity {
public:
    Zombie(float x, float y);
    ~Zombie() override;

    // 核心更新函数
    void update(float deltaTime) override;

    // 碰撞处理
    void onCollision(Entity* other) override;

    // 移动控制（响应玩家输入）
    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();
    void stopMoving();

    // 道具系统
    void pickupItem(Item* item);
    void useItem(int slot);
    bool hasItem(int slot) const;
    Item* getItem(int slot) const;
    int getInventorySize() const { return maxInventorySize_; }
    const std::vector<Item*>& getInventory() const { return inventory_; }

    // 装备系统
    void equipBucket(float armorValue = 200.0f);  // 装备铁桶（带护甲值）
    void equipPoleVault();                        // 装备撑杆跳套装
    void removeBucket();                          // 移除铁桶
    void removePoleVault();                       // 移除撑杆跳套装
    bool hasBucket() const { return hasBucket_; }
    bool hasPoleVault() const { return hasPoleVault_; }
    bool hasPoleVaultJumped() const { return poleVaultJumped_; }  // 撑杆是否已跳跃
    bool isPoleVaultJumping() const { return poleVaultJumping_; }  // 撑杆是否正在跳跃中
    void performPoleVaultJump();                  // 执行撑杆跳跃
    float getArmor() const { return armor_; }

    // 装备掉落（切换装备时掉落）
    void dropEquipmentAtPosition(ZombieForm formToDrop);
    void setEntityManager(EntityManager* manager) { entityManager_ = manager; }
    void setMaze(Maze* maze) { maze_ = maze; }

    // 增益效果
    void applySpeedBoost(float multiplier, float duration);
    void applyShield(float duration);
    bool hasShield() const { return shieldActive_; }

    // 治疗
    void heal(float amount);

    // 伤害处理（覆盖基类）
    void takeDamage(float damage) override;

    // 状态
    ZombieState getState() const { return state_; }
    ZombieForm getForm() const { return form_; }

    // 序列化
    std::string toJson() const override;

protected:
    // 动画初始化（在构造函数中调用）
    void initializeAnimations() override;

    // 更新动画状态
    void updateAnimation() override;

private:
    // 当前状态和形态
    ZombieState state_;
    ZombieForm form_;

    // 移动相关
    bool isMoving_;
    Vector2D inputDirection_;  // 输入方向
    float normalSpeed_;        // 普通速度（走）
    float poleVaultSpeed_;     // 撑杆跳速度（跑）

    // 道具栏
    std::vector<Item*> inventory_;
    int maxInventorySize_;

    // 装备
    bool hasBucket_;
    bool hasPoleVault_;
    bool poleVaultJumped_;   // 撑杆跳是否已使用（跳跃后变为walk）
    bool poleVaultJumping_;  // 撑杆跳是否正在跳跃中
    float jumpAnimationTimer_;  // 跳跃动画计时器
    float jumpAnimationDuration_;  // 跳跃动画持续时间
    float jumpDistance_;     // 跳跃距离
    Direction jumpDirection_;  // 跳跃方向（用于前端旋转动画）
    float armor_;
    float maxArmor_;

    // 实体管理器引用（用于掉落道具）
    EntityManager* entityManager_;

    // 迷宫引用（用于墙壁碰撞检测）
    Maze* maze_;

    // 增益效果
    bool shieldActive_;
    float shieldTimer_;
    float speedBoostMultiplier_;
    float speedBoostTimer_;

    // 受伤无敌时间
    float damageInvulnerabilityTimer_;
    float damageInvulnerabilityDuration_;

    // 受伤硬直时间（被攻击时无法对植物造成伤害）
    float attackStunTimer_;
    float attackStunDuration_;

    // 道具拾取冷却（防止不断拾取/放下循环）
    float itemPickupCooldown_;
    float itemPickupCooldownDuration_;

    // 内部辅助函数
    void updateMovement(float deltaTime);
    void updateBuffs(float deltaTime);
    void setState(ZombieState newState);
    void updateSpeedBasedOnForm();  // 根据形态更新速度

    // 添加道具到背包
    bool addItemToInventory(Item* item);

    // 植物交互
    void updatePlantInteraction(float deltaTime);
    class Plant* checkPlantCollision() const;  // 检查是否碰到植物
    void eatPlant(class Plant* plant, float deltaTime);  // 吃植物
    class Plant* currentEatingPlant_;  // 当前正在吃的植物
    float eatDamagePerSecond_;  // 每秒吃植物伤害
    float eatDamageTimer_;  // 吃植物伤害计时器（每秒结算一次）

    // 戴夫交互
    void updateDaveInteraction(float deltaTime);
    class Dave* checkDaveCollision() const;  // 检查是否碰到戴夫
    void attackDave(class Dave* dave, float deltaTime);  // 攻击戴夫
    class Dave* currentAttackingDave_;  // 当前正在攻击的戴夫
    float attackDaveTimer_;  // 攻击戴夫计时器
};

#endif // ZOMBIE_H
