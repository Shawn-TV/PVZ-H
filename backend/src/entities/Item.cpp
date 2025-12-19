/**
 * @file Item.cpp
 * @brief 道具类实现
 */

#include "../../include/entities/Item.h"
#include "../../include/entities/Zombie.h"
#include <sstream>

// ==================== Item 基类 ====================

Item::Item(float x, float y, ItemType itemType)
    : Entity(x, y, EntityType::ITEM),
      itemType_(itemType),
      pickedUp_(false),
      lifetime_(0),
      maxLifetime_(-1) {  // 默认永久存在

    setSize(40, 40);  // 道具碰撞箱
}

Item::~Item() {
}

void Item::update(float deltaTime) {
    if (!alive_ || pickedUp_) return;

    // 更新存在时间
    if (maxLifetime_ > 0) {
        lifetime_ += deltaTime;
        if (lifetime_ >= maxLifetime_) {
            alive_ = false;
        }
    }

    // 更新动画
    updateAnimation();
    animationController_.update(deltaTime);
}

void Item::onCollision(Entity* other) {
    // 道具不处理碰撞，由僵尸处理拾取逻辑
}

void Item::initializeAnimations() {
    // 基类默认不做任何事，由子类实现
}

void Item::updateAnimation() {
    // 道具通常只有一个待机动画
    if (!animationController_.isPlaying("idle")) {
        animationController_.play("idle");
    }
}

std::string Item::toJson() const {
    std::stringstream ss;
    ss << "{"
       << "\"id\":" << id_ << ","
       << "\"type\":\"item\","
       << "\"itemType\":" << static_cast<int>(itemType_) << ","
       << "\"x\":" << position_.x << ","
       << "\"y\":" << position_.y << ","
       << "\"pickedUp\":" << (pickedUp_ ? "true" : "false")
       << "}";
    return ss.str();
}

// ==================== PoleVaultKit 撑杆跳套装 ====================

PoleVaultKit::PoleVaultKit(float x, float y)
    : Item(x, y, ItemType::POLE_VAULT_KIT) {

    initializeAnimations();
}

PoleVaultKit::~PoleVaultKit() {
}

bool PoleVaultKit::applyEffect(Zombie* zombie) {
    if (!zombie) return false;

    // 让僵尸变身为撑杆跳僵尸
    // 这个逻辑会在Zombie类中实现
    zombie->equipPoleVault();

    return true;  // 成功应用效果
}

void PoleVaultKit::initializeAnimations() {
    // 撑杆跳套装的待机动画
    AnimationClip* idleAnim = new AnimationClip("idle", true);
    idleAnim->addFrame("assets/images/items/polevault_kit/frame_0.png", 0.3f);
    idleAnim->addFrame("assets/images/items/polevault_kit/frame_1.png", 0.3f);
    animationController_.registerAnimation(idleAnim);

    animationController_.play("idle");
}

// ==================== Bucket 铁桶 ====================

Bucket::Bucket(float x, float y)
    : Item(x, y, ItemType::BUCKET),
      bucketArmor_(200.0f) {  // 铁桶提供200点护甲

    initializeAnimations();
}

Bucket::~Bucket() {
}

bool Bucket::applyEffect(Zombie* zombie) {
    if (!zombie) return false;

    // 让僵尸装备铁桶
    zombie->equipBucket(bucketArmor_);

    return true;  // 成功应用效果
}

void Bucket::initializeAnimations() {
    // 铁桶的待机动画
    AnimationClip* idleAnim = new AnimationClip("idle", true);
    idleAnim->addFrame("assets/images/items/bucket/frame_0.png", 0.3f);
    idleAnim->addFrame("assets/images/items/bucket/frame_1.png", 0.3f);
    animationController_.registerAnimation(idleAnim);

    animationController_.play("idle");
}

// ==================== HealthPotion 生命药水 ====================

HealthPotion::HealthPotion(float x, float y)
    : Item(x, y, ItemType::HEALTH_POTION),
      healAmount_(50.0f) {  // 恢复50点生命值

    initializeAnimations();
}

HealthPotion::~HealthPotion() {
}

bool HealthPotion::applyEffect(Zombie* zombie) {
    if (!zombie) return false;

    // 恢复生命值
    zombie->heal(healAmount_);

    return true;  // 成功应用效果，道具被消耗
}

void HealthPotion::initializeAnimations() {
    // 生命药水的待机动画
    AnimationClip* idleAnim = new AnimationClip("idle", true);
    idleAnim->addFrame("assets/images/items/health_potion/frame_0.png", 0.25f);
    idleAnim->addFrame("assets/images/items/health_potion/frame_1.png", 0.25f);
    idleAnim->addFrame("assets/images/items/health_potion/frame_2.png", 0.25f);
    idleAnim->addFrame("assets/images/items/health_potion/frame_3.png", 0.25f);
    animationController_.registerAnimation(idleAnim);

    animationController_.play("idle");
}
