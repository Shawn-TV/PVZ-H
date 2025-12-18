/**
 * @file Explosion.cpp
 * @brief 爆炸效果实现
 */

#include "../../include/entities/Explosion.h"
#include "../../include/entities/Zombie.h"
#include "../../include/entities/Dave.h"
#include "../../include/entities/EntityManager.h"
#include <sstream>

Explosion::Explosion(float x, float y, float radius, float damage)
    : Entity(x, y, EntityType::PROJECTILE),  // 爆炸也算投射物类型
      radius_(radius),
      damage_(damage),
      lifetime_(0),
      maxLifetime_(0.5f),  // 爆炸效果持续0.5秒
      hasDealtDamage_(false) {

    // 设置碰撞盒大小为爆炸半径
    setSize(radius * 2, radius * 2);

    // 初始化动画
    initializeAnimations();
}

Explosion::~Explosion() {
}

void Explosion::update(float deltaTime) {
    if (!alive_) return;

    // 更新生命周期
    lifetime_ += deltaTime;

    // 爆炸动画播放完毕或超时后销毁
    if (lifetime_ >= maxLifetime_ || animationController_.isCurrentAnimationFinished()) {
        alive_ = false;
        return;
    }

    // 更新动画
    updateAnimation();
    animationController_.update(deltaTime);
}

void Explosion::onCollision(Entity* other) {
    // 爆炸不响应碰撞
    // 伤害由创建爆炸的实体（如CherryBomb）通过EntityManager处理
}

void Explosion::initializeAnimations() {
    // 爆炸动画（不循环）
    AnimationClip* explodeAnim = new AnimationClip("explode", false);
    explodeAnim->addFrame("assets/images/effects/explosion/frame_0.png", 0.05f);
    explodeAnim->addFrame("assets/images/effects/explosion/frame_1.png", 0.05f);
    explodeAnim->addFrame("assets/images/effects/explosion/frame_2.png", 0.1f);
    explodeAnim->addFrame("assets/images/effects/explosion/frame_3.png", 0.1f);
    explodeAnim->addFrame("assets/images/effects/explosion/frame_4.png", 0.1f);
    explodeAnim->addFrame("assets/images/effects/explosion/frame_5.png", 0.1f);
    animationController_.registerAnimation(explodeAnim);

    animationController_.play("explode");
}

void Explosion::updateAnimation() {
    // 爆炸只有一个动画
    if (!animationController_.isPlaying("explode")) {
        animationController_.play("explode", true);
    }
}

std::string Explosion::toJson() const {
    std::stringstream ss;
    ss << "{"
       << "\"id\":" << id_ << ","
       << "\"type\":\"explosion\","
       << "\"x\":" << position_.x << ","
       << "\"y\":" << position_.y << ","
       << "\"radius\":" << radius_ << ","
       << "\"damage\":" << damage_ << ","
       << "\"alive\":" << (alive_ ? "true" : "false")
       << "}";
    return ss.str();
}
