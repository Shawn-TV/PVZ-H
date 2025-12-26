/**
 * @file CherryBomb.cpp
 * @brief 樱桃炸弹实现
 */

#include "../../include/plants/CherryBomb.h"
#include "../../include/entities/Explosion.h"
#include "../../include/entities/Zombie.h"
#include "../../include/entities/Dave.h"
#include "../../include/entities/EntityManager.h"
#include <sstream>

CherryBomb::CherryBomb(float x, float y)
    : Plant(x, y, PlantType::CHERRY_BOMB),
      explosionTimer_(0),
      explosionDelay_(3.0f),         // 触发后延迟（现在作为后备）
      explosionRadius_(150.0f),       // 爆炸半径150像素（1格）
      explosionDamage_(9999.0f),      // 爆炸伤害极高，确保秒杀
      hasExploded_(false),
      triggerRadius_(150.0f),         // 触发半径：1格 = 150像素
      isTriggered_(false),            // 是否已被触发
      isSwelling_(false),             // 是否正在膨胀
      swellingTimer_(0),
      swellingDuration_(0.78f) {       // 膨胀持续0.78秒，与动画同步(14帧/18fps)

    // 樱桃炸弹属性
    health_ = 9999.0f;   // 地雷生命值很高，不容易被破坏
    maxHealth_ = 9999.0f;
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

    // 地雷模式：检测僵尸是否在触发范围内
    if (!isTriggered_ && entityManager_) {
        auto entitiesInRange = entityManager_->findEntitiesInRange(position_, triggerRadius_);
        for (auto* entity : entitiesInRange) {
            if (!entity || !entity->isAlive()) continue;
            // 只对僵尸触发
            if (entity->getType() == EntityType::ZOMBIE) {
                // 僵尸进入触发范围，开始膨胀阶段
                isTriggered_ = true;
                isSwelling_ = true;
                swellingTimer_ = 0;
                animationController_.play("countdown");
                break;
            }
        }
    }

    // 膨胀阶段
    if (isSwelling_ && !hasExploded_) {
        swellingTimer_ += deltaTime;
        // 膨胀结束后爆炸
        if (swellingTimer_ >= swellingDuration_) {
            isSwelling_ = false;
            explode();
            hasExploded_ = true;
            alive_ = false;  // 爆炸后销毁
            return;
        }
    }

    // 更新动画
    updateAnimation();
    animationController_.update(deltaTime);
}

float CherryBomb::getSwellingProgress() const {
    if (!isSwelling_) return 0.0f;
    return swellingTimer_ / swellingDuration_;
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

        // 对僵尸直接秒杀（设置为不活跃，让它们瞬间消失）
        if (entity->getType() == EntityType::ZOMBIE) {
            Zombie* zombie = dynamic_cast<Zombie*>(entity);
            if (zombie) {
                // 直接杀死僵尸，不通过伤害（瞬间消失）
                zombie->setHealth(0);
                zombie->setAlive(false);
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

    // 静止动画（单帧，等待触发）
    AnimationClip* idleAnim = new AnimationClip("idle", true);
    idleAnim->addFrame("assets/images/plants/cherrybomb/countdown/frame_0.png", 1.0f);
    animationController_.registerAnimation(idleAnim);

    // 倒计时动画（循环，越来越快）- 触发后播放
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

    // 默认播放静止动画（等待僵尸靠近）
    animationController_.play("idle");
}

void CherryBomb::updateAnimation() {
    // 如果被触发，播放倒计时动画（在update中已设置）
    // 如果未触发，保持idle动画
    if (!isTriggered_ && !animationController_.isPlaying("idle")) {
        animationController_.play("idle");
    }
}

std::string CherryBomb::toJson() const {
    std::stringstream ss;
    ss << "{"
       << "\"id\":" << id_ << ","
       << "\"type\":\"plant\","
       << "\"plantType\":" << static_cast<int>(plantType_) << ","
       << "\"x\":" << position_.x << ","
       << "\"y\":" << position_.y << ","
       << "\"health\":" << health_ << ","
       << "\"maxHealth\":" << maxHealth_ << ","
       << "\"attackDirection\":" << static_cast<int>(attackDirection_) << ","
       << "\"alive\":" << (alive_ ? "true" : "false") << ","
       << "\"isTriggered\":" << (isTriggered_ ? "true" : "false") << ","
       << "\"isSwelling\":" << (isSwelling_ ? "true" : "false") << ","
       << "\"swellingProgress\":" << getSwellingProgress()
       << "}";
    return ss.str();
}
