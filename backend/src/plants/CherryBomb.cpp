/**
 * @file CherryBomb.cpp
 * @brief 樱桃炸弹实现
 */

#include "../../include/plants/CherryBomb.h"
#include "../../include/entities/Explosion.h"
#include "../../include/entities/Zombie.h"
#include "../../include/entities/Dave.h"
#include "../../include/entities/EntityManager.h"

CherryBomb::CherryBomb(float x, float y)
    : Plant(x, y, PlantType::CHERRY_BOMB),
      explosionTimer_(0),
      explosionDelay_(3.0f),     // 3秒后爆炸
      explosionRadius_(100.0f),   // 爆炸半径100像素
      explosionDamage_(100.0f),   // 爆炸伤害100点
      hasExploded_(false) {

    // 樱桃炸弹属性
    health_ = 50.0f;
    maxHealth_ = 50.0f;
    attackDirection_ = Direction::NONE;  // 全方向爆炸

    // 樱桃炸弹不需要攻击冷却，因为它只爆炸一次
    attackCooldown_ = 0;
    attackRange_ = explosionRadius_;

    // 初始化动画
    initializeAnimations();
}

CherryBomb::~CherryBomb() {
}

void CherryBomb::update(float deltaTime) {
    if (!alive_ || hasExploded_) return;

    // 更新倒计时
    explosionTimer_ += deltaTime;

    // 倒计时结束，爆炸
    if (explosionTimer_ >= explosionDelay_) {
        explode();
        hasExploded_ = true;
        alive_ = false;  // 爆炸后销毁
        return;
    }

    // 更新动画
    updateAnimation();
    animationController_.update(deltaTime);
}

void CherryBomb::performAttack() {
    // 樱桃炸弹的"攻击"就是爆炸
    // 但爆炸是由倒计时触发，不是由performAttack触发
}

void CherryBomb::explode() {
    if (!entityManager_) return;

    // 创建爆炸效果
    Explosion* explosion = new Explosion(
        position_.x,
        position_.y,
        explosionRadius_,
        explosionDamage_
    );
    entityManager_->addEntity(explosion);

    // 对爆炸范围内的所有实体造成伤害
    auto entitiesInRange = entityManager_->findEntitiesInRange(position_, explosionRadius_);

    for (auto* entity : entitiesInRange) {
        if (!entity || !entity->isAlive()) continue;
        if (entity == this) continue;  // 不对自己造成伤害

        // 对僵尸造成伤害
        if (entity->getType() == EntityType::ZOMBIE) {
            Zombie* zombie = dynamic_cast<Zombie*>(entity);
            if (zombie) {
                zombie->takeDamage(explosionDamage_);
            }
        }
        // 对戴夫造成伤害（如果戴夫在范围内）
        else if (entity->getType() == EntityType::DAVE) {
            Dave* dave = dynamic_cast<Dave*>(entity);
            if (dave) {
                dave->takeDamage(explosionDamage_);
                dave->stun(2.0f);  // 眩晕2秒
            }
        }
    }
}

void CherryBomb::initializeAnimations() {
    // ===== 樱桃炸弹动画 =====

    // 倒计时动画（循环，越来越快）
    AnimationClip* countdownAnim = new AnimationClip("countdown", true);
    countdownAnim->addFrame("assets/images/plants/cherrybomb/countdown/frame_0.png", 0.3f);
    countdownAnim->addFrame("assets/images/plants/cherrybomb/countdown/frame_1.png", 0.25f);
    countdownAnim->addFrame("assets/images/plants/cherrybomb/countdown/frame_2.png", 0.2f);
    countdownAnim->addFrame("assets/images/plants/cherrybomb/countdown/frame_3.png", 0.15f);
    animationController_.registerAnimation(countdownAnim);

    // 爆炸动画（不循环）
    AnimationClip* explodeAnim = new AnimationClip("explode", false);
    explodeAnim->addFrame("assets/images/plants/cherrybomb/explode/frame_0.png", 0.05f);
    explodeAnim->addFrame("assets/images/plants/cherrybomb/explode/frame_1.png", 0.05f);
    explodeAnim->addFrame("assets/images/plants/cherrybomb/explode/frame_2.png", 0.1f);
    animationController_.registerAnimation(explodeAnim);

    // 默认播放倒计时动画
    animationController_.play("countdown");
}

void CherryBomb::updateAnimation() {
    // 倒计时快结束时（最后0.5秒）播放爆炸动画
    if (explosionTimer_ >= explosionDelay_ - 0.5f) {
        if (!animationController_.isPlaying("explode")) {
            animationController_.play("explode", true);
        }
    } else {
        // 否则播放倒计时动画
        if (!animationController_.isPlaying("countdown")) {
            animationController_.play("countdown");
        }
    }
}
