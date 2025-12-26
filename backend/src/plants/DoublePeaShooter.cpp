/**
 * @file DoublePeaShooter.cpp
 * @brief 双发射手实现
 */

#include "../../include/plants/DoublePeaShooter.h"
#include "../../include/entities/Projectile.h"
#include "../../include/entities/EntityManager.h"
#include "../../include/core/Config.h"

DoublePeaShooter::DoublePeaShooter(float x, float y, Direction shootDirection)
    : Plant(x, y, PlantType::DOUBLE_PEA_SHOOTER),
      peaSpeed_(Config::PEA_SPEED),
      peaDamage_(static_cast<float>(Config::INITIAL_ZOMBIE_HEALTH) / 10.0f),  // 1/10 of zombie max health
      peaInterval_(0.167f),   // 两颗豌豆间隔约1/6秒（快速二连发）
      secondShotPending_(false),
      secondShotTimer_(0.0f) {

    // 根据通道方向设置攻击方向
    // 水平通道：向左或向右发射
    // 竖直通道：向上或向下发射
    attackDirection_ = shootDirection;
    direction_ = shootDirection;

    // 双发射手属性
    attackCooldown_ = 1.0f;      // 每秒一轮双发
    attackRange_ = 3000.0f;      // 攻击范围扩大，确保能检测到同行远处的僵尸
    attackDamage_ = peaDamage_;
    health_ = 100.0f;
    maxHealth_ = 100.0f;

    // 初始化动画
    initializeAnimations();
}

DoublePeaShooter::~DoublePeaShooter() {
}

void DoublePeaShooter::update(float deltaTime) {
    if (!alive_) return;

    // 处理延迟的第二发豌豆
    if (secondShotPending_) {
        secondShotTimer_ -= deltaTime;
        if (secondShotTimer_ <= 0) {
            shootPea();
            secondShotPending_ = false;
        }
    }

    // 调用基类的update（处理攻击冷却和目标检测）
    Plant::update(deltaTime);
}

void DoublePeaShooter::performAttack() {
    // 发射第一颗豌豆
    shootPea();

    // 设置第二颗豌豆的延迟发射
    secondShotPending_ = true;
    secondShotTimer_ = peaInterval_;
}

void DoublePeaShooter::shootPea() {
    if (!entityManager_) return;

    // 计算豌豆生成位置（根据发射方向偏移）
    // 嘴部位置约在精灵中心上方15像素处
    Vector2D spawnOffset(0, 0);
    const float mouthOffsetY = -15.0f;

    // 根据方向设置基础偏移
    switch (attackDirection_) {
        case Direction::UP:
            spawnOffset = Vector2D(0, -25);
            break;
        case Direction::DOWN:
            spawnOffset = Vector2D(0, 25);
            break;
        case Direction::LEFT:
            spawnOffset = Vector2D(-25, mouthOffsetY);
            break;
        case Direction::RIGHT:
            spawnOffset = Vector2D(25, mouthOffsetY);
            break;
        default:
            break;
    }

    Vector2D spawnPos = position_ + spawnOffset;

    // 创建豌豆
    Projectile* pea = new Projectile(
        spawnPos.x,
        spawnPos.y,
        ProjectileType::PEA,
        attackDirection_,
        peaSpeed_,
        peaDamage_
    );

    // 设置迷宫引用（用于墙壁碰撞检测）
    if (maze_) {
        pea->setMaze(maze_);
    }

    // 添加到实体管理器
    entityManager_->addEntity(pea);
}

void DoublePeaShooter::initializeAnimations() {
    // ===== 双发射手动画 =====

    // 待机动画（循环）
    AnimationClip* idleAnim = new AnimationClip("idle", true);
    idleAnim->addFrame("assets/images/plants/doublepeashooter/idle/frame_0.png", 0.2f);
    idleAnim->addFrame("assets/images/plants/doublepeashooter/idle/frame_1.png", 0.2f);
    idleAnim->addFrame("assets/images/plants/doublepeashooter/idle/frame_2.png", 0.2f);
    idleAnim->addFrame("assets/images/plants/doublepeashooter/idle/frame_3.png", 0.2f);
    animationController_.registerAnimation(idleAnim);

    // 攻击动画（不循环，双发射）
    AnimationClip* attackAnim = new AnimationClip("attack", false);
    attackAnim->addFrame("assets/images/plants/doublepeashooter/attack/frame_0.png", 0.1f);
    attackAnim->addFrame("assets/images/plants/doublepeashooter/attack/frame_1.png", 0.1f);
    attackAnim->addFrame("assets/images/plants/doublepeashooter/attack/frame_2.png", 0.08f);  // 第一发
    attackAnim->addFrame("assets/images/plants/doublepeashooter/attack/frame_3.png", 0.08f);  // 第二发
    attackAnim->addFrame("assets/images/plants/doublepeashooter/attack/frame_4.png", 0.1f);
    animationController_.registerAnimation(attackAnim);

    // 默认播放待机动画
    animationController_.play("idle");
}

void DoublePeaShooter::updateAnimation() {
    // 根据是否正在攻击选择动画

    // 如果攻击冷却很小，说明刚刚攻击过，播放攻击动画
    if (currentAttackCooldown_ > attackCooldown_ - 0.5f) {
        if (!animationController_.isPlaying("attack")) {
            animationController_.play("attack", true);
        }
    } else {
        // 否则播放待机动画
        if (!animationController_.isPlaying("idle")) {
            // 如果攻击动画播放完了，回到待机
            if (animationController_.isCurrentAnimationFinished()) {
                animationController_.play("idle");
            }
        }
    }
}
