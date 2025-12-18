/**
 * @file PeaShooter.cpp
 * @brief 豌豆射手实现
 */

#include "../../include/plants/PeaShooter.h"
#include "../../include/entities/Projectile.h"
#include "../../include/entities/EntityManager.h"

PeaShooter::PeaShooter(float x, float y, Direction shootDirection)
    : Plant(x, y, PlantType::PEA_SHOOTER),
      peaSpeed_(200.0f),
      peaDamage_(10.0f) {

    // 设置攻击方向
    attackDirection_ = shootDirection;
    direction_ = shootDirection;

    // 豌豆射手属性
    attackCooldown_ = 2.0f;      // 每2秒发射一次
    attackRange_ = 300.0f;       // 攻击范围300像素
    attackDamage_ = peaDamage_;
    health_ = 100.0f;
    maxHealth_ = 100.0f;

    // 初始化动画
    initializeAnimations();
}

PeaShooter::~PeaShooter() {
}

void PeaShooter::performAttack() {
    // 发射豌豆
    if (!entityManager_) return;

    // 创建豌豆投射物
    // 豌豆生成在豌豆射手前方一点的位置
    Vector2D spawnOffset(0, 0);
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

    // 添加到实体管理器
    entityManager_->addEntity(pea);
}

void PeaShooter::initializeAnimations() {
    // ===== 豌豆射手动画 =====

    // 待机动画（循环）
    AnimationClip* idleAnim = new AnimationClip("idle", true);
    idleAnim->addFrame("assets/images/plants/peashooter/idle/frame_0.png", 0.2f);
    idleAnim->addFrame("assets/images/plants/peashooter/idle/frame_1.png", 0.2f);
    idleAnim->addFrame("assets/images/plants/peashooter/idle/frame_2.png", 0.2f);
    idleAnim->addFrame("assets/images/plants/peashooter/idle/frame_3.png", 0.2f);
    animationController_.registerAnimation(idleAnim);

    // 攻击动画（不循环）
    AnimationClip* attackAnim = new AnimationClip("attack", false);
    attackAnim->addFrame("assets/images/plants/peashooter/attack/frame_0.png", 0.1f);
    attackAnim->addFrame("assets/images/plants/peashooter/attack/frame_1.png", 0.1f);
    attackAnim->addFrame("assets/images/plants/peashooter/attack/frame_2.png", 0.15f);  // 发射帧
    attackAnim->addFrame("assets/images/plants/peashooter/attack/frame_3.png", 0.1f);
    attackAnim->addFrame("assets/images/plants/peashooter/attack/frame_4.png", 0.1f);
    animationController_.registerAnimation(attackAnim);

    // 默认播放待机动画
    animationController_.play("idle");
}

void PeaShooter::updateAnimation() {
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
