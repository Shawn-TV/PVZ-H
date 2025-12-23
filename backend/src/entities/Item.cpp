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
      maxLifetime_(-1),  // 默认永久存在
      pickupImmunityTimer_(0) {

    setSize(60, 60);  // 道具碰撞箱（与sprite大小一致，便于拾取）
}

Item::~Item() {
}

void Item::update(float deltaTime) {
    if (!alive_ || pickedUp_) return;

    // 更新拾取免疫时间
    if (pickupImmunityTimer_ > 0) {
        pickupImmunityTimer_ -= deltaTime;
    }

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

    // 获取道具类型字符串
    std::string itemTypeStr;
    switch (itemType_) {
        case ItemType::POLE_VAULT_KIT:
            itemTypeStr = "pole";
            break;
        case ItemType::BUCKET:
            itemTypeStr = "bucket";
            break;
        case ItemType::HEALTH_POTION:
            itemTypeStr = "health_potion";
            break;
        case ItemType::SPEED_POTION:
            itemTypeStr = "speed_potion";
            break;
        default:
            itemTypeStr = "unknown";
            break;
    }

    ss << "{"
       << "\"id\":" << id_ << ","
       << "\"type\":\"item\","
       << "\"itemType\":\"" << itemTypeStr << "\","
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

Bucket::Bucket(float x, float y, float armorValue)
    : Item(x, y, ItemType::BUCKET),
      bucketArmor_(armorValue) {  // 使用传入的护甲值（默认200点）

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
      healPercentage_(0.4f) {  // 恢复40%最大生命值

    initializeAnimations();
}

HealthPotion::~HealthPotion() {
}

bool HealthPotion::applyEffect(Zombie* zombie) {
    if (!zombie) return false;

    // 恢复40%最大生命值
    float healAmount = zombie->getMaxHealth() * healPercentage_;
    zombie->heal(healAmount);

    return true;  // 成功应用效果，道具被消耗
}

void HealthPotion::initializeAnimations() {
    // 生命药水的待机动画（红色药水）
    AnimationClip* idleAnim = new AnimationClip("idle", true);
    idleAnim->addFrame("assets/images/items/health_potion/frame_0.png", 0.25f);
    idleAnim->addFrame("assets/images/items/health_potion/frame_1.png", 0.25f);
    idleAnim->addFrame("assets/images/items/health_potion/frame_2.png", 0.25f);
    idleAnim->addFrame("assets/images/items/health_potion/frame_3.png", 0.25f);
    animationController_.registerAnimation(idleAnim);

    animationController_.play("idle");
}

// ==================== SpeedPotion 速度药水 ====================

SpeedPotion::SpeedPotion(float x, float y)
    : Item(x, y, ItemType::SPEED_POTION),
      speedMultiplier_(1.5f),  // 1.5倍速度
      duration_(30.0f) {       // 持续30秒

    initializeAnimations();
}

SpeedPotion::~SpeedPotion() {
}

bool SpeedPotion::applyEffect(Zombie* zombie) {
    if (!zombie) return false;

    // 应用速度加成（如果已有相同加成效果，会叠加持续时间）
    zombie->applySpeedBoost(speedMultiplier_, duration_);

    return true;  // 成功应用效果，道具被消耗
}

void SpeedPotion::initializeAnimations() {
    // 速度药水的待机动画（绿色药水）
    AnimationClip* idleAnim = new AnimationClip("idle", true);
    idleAnim->addFrame("assets/images/items/speed_potion/frame_0.png", 0.25f);
    idleAnim->addFrame("assets/images/items/speed_potion/frame_1.png", 0.25f);
    idleAnim->addFrame("assets/images/items/speed_potion/frame_2.png", 0.25f);
    idleAnim->addFrame("assets/images/items/speed_potion/frame_3.png", 0.25f);
    animationController_.registerAnimation(idleAnim);

    animationController_.play("idle");
}
