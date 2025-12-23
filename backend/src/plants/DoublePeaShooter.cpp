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
      peaInterval_(0.15f) {  // 两颗豌豆间隔0.15秒

    // 根据通道方向设置攻击方向
    // 水平通道：向左或向右发射
    // 竖直通道：向上或向下发射
    attackDirection_ = shootDirection;
    direction_ = shootDirection;

    // 双发射手属性
    attackCooldown_ = 2.0f;      // 与普通豌豆射手相同
    attackRange_ = 300.0f;       // 攻击范围300像素
    attackDamage_ = peaDamage_;
    health_ = 100.0f;
    maxHealth_ = 100.0f;

    // 初始化动画
    initializeAnimations();
}

DoublePeaShooter::~DoublePeaShooter() {
}

void DoublePeaShooter::performAttack() {
    // 连续发射两颗豌豆
    // 第一颗立即发射
    shootPea(0.0f);

    // 第二颗稍后发射（通过位置偏移模拟）
    shootPea(peaInterval_);
}

void DoublePeaShooter::shootPea(float offsetTime) {
    if (!entityManager_) return;

    // 计算豌豆生成位置（根据发射方向偏移）
    Vector2D spawnOffset(0, 0);

    // 根据方向设置基础偏移
    switch (attackDirection_) {
        case Direction::UP:
            spawnOffset = Vector2D(0, -20);
            break;
        case Direction::DOWN:
            spawnOffset = Vector2D(0, 20);
            break;
        case Direction::LEFT:
            spawnOffset = Vector2D(-20, 0);
            break;
        case Direction::RIGHT:
            spawnOffset = Vector2D(20, 0);
            break;
        default:
            break;
    }

    // 如果是第二颗豌豆，根据方向再偏移一点距离
    // 模拟连续发射效果
    if (offsetTime > 0) {
        float extraOffset = offsetTime * peaSpeed_ * 0.5f;  // 第二颗在后面一点
        switch (attackDirection_) {
            case Direction::UP:
                spawnOffset.y -= extraOffset;
                break;
            case Direction::DOWN:
                spawnOffset.y += extraOffset;
                break;
            case Direction::LEFT:
                spawnOffset.x -= extraOffset;
                break;
            case Direction::RIGHT:
                spawnOffset.x += extraOffset;
                break;
            default:
                break;
        }
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
