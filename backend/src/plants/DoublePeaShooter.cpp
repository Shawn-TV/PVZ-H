/**
 * @file DoublePeaShooter.cpp
 * @brief 双发射手实现
 */

#include "../../include/plants/DoublePeaShooter.h"
#include "../../include/entities/Projectile.h"
#include "../../include/entities/EntityManager.h"

DoublePeaShooter::DoublePeaShooter(float x, float y, bool horizontal)
    : Plant(x, y, PlantType::PEA_SHOOTER),  // 使用豌豆射手类型
      peaSpeed_(200.0f),
      peaDamage_(10.0f),
      isHorizontal_(horizontal) {

    // 设置两个发射方向
    if (isHorizontal_) {
        direction1_ = Direction::LEFT;
        direction2_ = Direction::RIGHT;
        attackDirection_ = Direction::NONE;  // 双向，不限制
    } else {
        direction1_ = Direction::UP;
        direction2_ = Direction::DOWN;
        attackDirection_ = Direction::NONE;  // 双向，不限制
    }

    // 双发射手属性
    attackCooldown_ = 2.5f;      // 比普通豌豆射手稍慢（发射两发）
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
    // 同时向两个方向发射豌豆
    shootPea(direction1_);
    shootPea(direction2_);
}

void DoublePeaShooter::shootPea(Direction dir) {
    if (!entityManager_) return;

    // 计算豌豆生成位置（根据发射方向偏移）
    Vector2D spawnOffset(0, 0);
    switch (dir) {
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
        dir,
        peaSpeed_,
        peaDamage_
    );

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

    // 攻击动画（不循环，双向发射）
    AnimationClip* attackAnim = new AnimationClip("attack", false);
    attackAnim->addFrame("assets/images/plants/doublepeashooter/attack/frame_0.png", 0.1f);
    attackAnim->addFrame("assets/images/plants/doublepeashooter/attack/frame_1.png", 0.1f);
    attackAnim->addFrame("assets/images/plants/doublepeashooter/attack/frame_2.png", 0.15f);  // 双发射帧
    attackAnim->addFrame("assets/images/plants/doublepeashooter/attack/frame_3.png", 0.1f);
    attackAnim->addFrame("assets/images/plants/doublepeashooter/attack/frame_4.png", 0.1f);
    animationController_.registerAnimation(attackAnim);

    // 根据是否水平方向，可以有不同的动画变体
    if (isHorizontal_) {
        // 水平版本动画（左右发射）
        AnimationClip* attackHAnim = new AnimationClip("attack_horizontal", false);
        attackHAnim->addFrame("assets/images/plants/doublepeashooter/attack_h/frame_0.png", 0.1f);
        attackHAnim->addFrame("assets/images/plants/doublepeashooter/attack_h/frame_1.png", 0.1f);
        attackHAnim->addFrame("assets/images/plants/doublepeashooter/attack_h/frame_2.png", 0.15f);
        attackHAnim->addFrame("assets/images/plants/doublepeashooter/attack_h/frame_3.png", 0.1f);
        attackHAnim->addFrame("assets/images/plants/doublepeashooter/attack_h/frame_4.png", 0.1f);
        animationController_.registerAnimation(attackHAnim);
    } else {
        // 垂直版本动画（上下发射）
        AnimationClip* attackVAnim = new AnimationClip("attack_vertical", false);
        attackVAnim->addFrame("assets/images/plants/doublepeashooter/attack_v/frame_0.png", 0.1f);
        attackVAnim->addFrame("assets/images/plants/doublepeashooter/attack_v/frame_1.png", 0.1f);
        attackVAnim->addFrame("assets/images/plants/doublepeashooter/attack_v/frame_2.png", 0.15f);
        attackVAnim->addFrame("assets/images/plants/doublepeashooter/attack_v/frame_3.png", 0.1f);
        attackVAnim->addFrame("assets/images/plants/doublepeashooter/attack_v/frame_4.png", 0.1f);
        animationController_.registerAnimation(attackVAnim);
    }

    // 默认播放待机动画
    animationController_.play("idle");
}

void DoublePeaShooter::updateAnimation() {
    // 根据是否正在攻击选择动画

    // 如果攻击冷却很小，说明刚刚攻击过，播放攻击动画
    if (currentAttackCooldown_ > attackCooldown_ - 0.6f) {
        // 根据方向选择不同的攻击动画
        std::string attackAnimName = isHorizontal_ ? "attack_horizontal" : "attack_vertical";

        if (!animationController_.isPlaying(attackAnimName)) {
            animationController_.play(attackAnimName, true);
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
