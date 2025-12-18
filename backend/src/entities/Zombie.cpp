/**
 * @file Zombie.cpp
 * @brief 僵尸类实现
 */

#include "../../include/entities/Zombie.h"
#include "../../include/entities/Item.h"
#include "../../include/core/Config.h"
#include <sstream>
#include <algorithm>

Zombie::Zombie(float x, float y)
    : Entity(x, y, EntityType::ZOMBIE),
      state_(ZombieState::IDLE),
      isMoving_(false),
      inputDirection_(0, 0),
      maxInventorySize_(6),
      hasBucket_(false),
      armor_(0),
      maxArmor_(50),
      shieldActive_(false),
      shieldTimer_(0),
      speedBoostMultiplier_(1.0f),
      speedBoostTimer_(0),
      damageInvulnerabilityTimer_(0),
      damageInvulnerabilityDuration_(0.5f) {

    // 设置僵尸属性
    speed_ = Config::ZOMBIE_SPEED;
    health_ = Config::INITIAL_ZOMBIE_HEALTH;
    maxHealth_ = Config::INITIAL_ZOMBIE_HEALTH;
    setSize(32, 32);  // 设置碰撞盒大小

    // 初始化道具栏
    inventory_.resize(maxInventorySize_, nullptr);

    // 初始化动画
    initializeAnimations();
}

Zombie::~Zombie() {
    // 清理道具
    for (auto* item : inventory_) {
        if (item) {
            delete item;
        }
    }
    inventory_.clear();
}

void Zombie::update(float deltaTime) {
    if (!alive_) {
        setState(ZombieState::DEAD);
        return;
    }

    // 更新移动
    updateMovement(deltaTime);

    // 更新增益效果
    updateBuffs(deltaTime);

    // 更新无敌时间
    if (damageInvulnerabilityTimer_ > 0) {
        damageInvulnerabilityTimer_ -= deltaTime;
    }

    // 更新动画
    updateAnimation();
    animationController_.update(deltaTime);
}

void Zombie::onCollision(Entity* other) {
    if (!other || !alive_) return;

    switch (other->getType()) {
        case EntityType::ITEM:
            // 道具自动拾取
            pickupItem(dynamic_cast<Item*>(other));
            break;

        case EntityType::DAVE:
        case EntityType::PLANT:
        case EntityType::PROJECTILE:
            // 受到伤害由对方实体处理
            break;

        default:
            break;
    }
}

// ==================== 移动控制 ====================

void Zombie::moveUp() {
    inputDirection_.y = -1;
    direction_ = Direction::UP;
    isMoving_ = true;
}

void Zombie::moveDown() {
    inputDirection_.y = 1;
    direction_ = Direction::DOWN;
    isMoving_ = true;
}

void Zombie::moveLeft() {
    inputDirection_.x = -1;
    direction_ = Direction::LEFT;
    isMoving_ = true;
}

void Zombie::moveRight() {
    inputDirection_.x = 1;
    direction_ = Direction::RIGHT;
    isMoving_ = true;
}

void Zombie::stopMoving() {
    inputDirection_ = Vector2D(0, 0);
    velocity_ = Vector2D(0, 0);
    isMoving_ = false;
}

void Zombie::updateMovement(float deltaTime) {
    if (inputDirection_.lengthSquared() > 0) {
        // 标准化方向向量
        Vector2D normalizedDir = inputDirection_.normalized();

        // 应用速度增益
        float effectiveSpeed = speed_ * speedBoostMultiplier_;

        // 设置速度
        velocity_ = normalizedDir * effectiveSpeed;

        // 应用速度到位置
        applyVelocity(deltaTime);

        // 设置状态为行走
        if (state_ != ZombieState::WALKING) {
            setState(ZombieState::WALKING);
        }
    } else {
        // 停止移动
        velocity_ = Vector2D(0, 0);

        // 设置状态为待机
        if (state_ == ZombieState::WALKING) {
            setState(ZombieState::IDLE);
        }
    }

    // 重置输入方向（每帧清空，等待新的输入）
    inputDirection_ = Vector2D(0, 0);
}

// ==================== 道具系统 ====================

void Zombie::pickupItem(Item* item) {
    if (!item || !item->isAlive()) return;

    if (addItemToInventory(item)) {
        // 道具已添加到背包
        item->setHealth(0);  // 标记道具已被拾取
    }
}

void Zombie::useItem(int slot) {
    if (slot < 0 || slot >= maxInventorySize_) return;

    Item* item = inventory_[slot];
    if (item) {
        item->use(this);

        // 使用后移除道具
        delete item;
        inventory_[slot] = nullptr;
    }
}

bool Zombie::hasItem(int slot) const {
    if (slot < 0 || slot >= maxInventorySize_) return false;
    return inventory_[slot] != nullptr;
}

Item* Zombie::getItem(int slot) const {
    if (slot < 0 || slot >= maxInventorySize_) return nullptr;
    return inventory_[slot];
}

bool Zombie::addItemToInventory(Item* item) {
    // 寻找空位
    for (int i = 0; i < maxInventorySize_; ++i) {
        if (inventory_[i] == nullptr) {
            inventory_[i] = item;
            return true;
        }
    }

    // 背包已满
    return false;
}

// ==================== 装备系统 ====================

void Zombie::equipBucket() {
    hasBucket_ = true;
    armor_ = maxArmor_;
}

void Zombie::removeBucket() {
    hasBucket_ = false;
    armor_ = 0;
}

// ==================== 增益效果 ====================

void Zombie::applySpeedBoost(float multiplier, float duration) {
    speedBoostMultiplier_ = multiplier;
    speedBoostTimer_ = duration;
}

void Zombie::applyShield(float duration) {
    shieldActive_ = true;
    shieldTimer_ = duration;
}

void Zombie::updateBuffs(float deltaTime) {
    // 更新速度增益
    if (speedBoostTimer_ > 0) {
        speedBoostTimer_ -= deltaTime;
        if (speedBoostTimer_ <= 0) {
            speedBoostMultiplier_ = 1.0f;
        }
    }

    // 更新护盾
    if (shieldTimer_ > 0) {
        shieldTimer_ -= deltaTime;
        if (shieldTimer_ <= 0) {
            shieldActive_ = false;
        }
    }
}

// ==================== 动画系统 ====================

void Zombie::initializeAnimations() {
    // ===== 注册所有僵尸动画 =====
    // 当你提供动画帧图片后，取消注释并填写正确的路径

    // 待机动画（循环）
    AnimationClip* idleAnim = new AnimationClip("idle", true);
    idleAnim->addFrame("assets/images/zombies/idle/frame_0.png", 0.15f);
    idleAnim->addFrame("assets/images/zombies/idle/frame_1.png", 0.15f);
    idleAnim->addFrame("assets/images/zombies/idle/frame_2.png", 0.15f);
    idleAnim->addFrame("assets/images/zombies/idle/frame_3.png", 0.15f);
    animationController_.registerAnimation(idleAnim);

    // 行走动画（循环）
    AnimationClip* walkAnim = new AnimationClip("walk", true);
    walkAnim->addFrame("assets/images/zombies/walk/frame_0.png", 0.1f);
    walkAnim->addFrame("assets/images/zombies/walk/frame_1.png", 0.1f);
    walkAnim->addFrame("assets/images/zombies/walk/frame_2.png", 0.1f);
    walkAnim->addFrame("assets/images/zombies/walk/frame_3.png", 0.1f);
    walkAnim->addFrame("assets/images/zombies/walk/frame_4.png", 0.1f);
    walkAnim->addFrame("assets/images/zombies/walk/frame_5.png", 0.1f);
    animationController_.registerAnimation(walkAnim);

    // 受伤动画（不循环）
    AnimationClip* damageAnim = new AnimationClip("damaged", false);
    damageAnim->addFrame("assets/images/zombies/damaged/frame_0.png", 0.1f);
    damageAnim->addFrame("assets/images/zombies/damaged/frame_1.png", 0.1f);
    damageAnim->addFrame("assets/images/zombies/damaged/frame_2.png", 0.1f);
    animationController_.registerAnimation(damageAnim);

    // 死亡动画（不循环）
    AnimationClip* deathAnim = new AnimationClip("death", false);
    deathAnim->addFrame("assets/images/zombies/death/frame_0.png", 0.15f);
    deathAnim->addFrame("assets/images/zombies/death/frame_1.png", 0.15f);
    deathAnim->addFrame("assets/images/zombies/death/frame_2.png", 0.15f);
    deathAnim->addFrame("assets/images/zombies/death/frame_3.png", 0.15f);
    deathAnim->addFrame("assets/images/zombies/death/frame_4.png", 0.15f);
    animationController_.registerAnimation(deathAnim);

    // 默认播放待机动画
    animationController_.play("idle");
}

void Zombie::updateAnimation() {
    // 根据当前状态选择播放对应的动画

    switch (state_) {
        case ZombieState::IDLE:
            if (!animationController_.isPlaying("idle")) {
                animationController_.play("idle");
            }
            break;

        case ZombieState::WALKING:
            if (!animationController_.isPlaying("walk")) {
                animationController_.play("walk");
            }
            break;

        case ZombieState::DAMAGED:
            if (!animationController_.isPlaying("damaged")) {
                animationController_.play("damaged", true);
            }
            // 受伤动画播放完毕后回到待机或行走
            if (animationController_.isCurrentAnimationFinished()) {
                if (isMoving_) {
                    setState(ZombieState::WALKING);
                } else {
                    setState(ZombieState::IDLE);
                }
            }
            break;

        case ZombieState::DEAD:
            if (!animationController_.isPlaying("death")) {
                animationController_.play("death", true);
            }
            break;
    }
}

void Zombie::setState(ZombieState newState) {
    if (state_ != newState) {
        state_ = newState;
    }
}

// ==================== 受伤和治疗 ====================

void Zombie::takeDamage(float damage) {
    // 无敌时间内不受伤
    if (damageInvulnerabilityTimer_ > 0) return;

    // 护盾激活时免疫伤害
    if (shieldActive_) return;

    // 优先消耗护甲
    if (armor_ > 0) {
        armor_ -= damage;
        if (armor_ < 0) {
            // 护甲破碎，剩余伤害作用于生命值
            Entity::takeDamage(-armor_);
            armor_ = 0;
            hasBucket_ = false;
        }
    } else {
        // 直接扣除生命值
        Entity::takeDamage(damage);
    }

    // 设置受伤状态
    if (alive_ && state_ != ZombieState::DAMAGED) {
        setState(ZombieState::DAMAGED);
    }

    // 设置无敌时间
    damageInvulnerabilityTimer_ = damageInvulnerabilityDuration_;
}

// ==================== 序列化 ====================

std::string Zombie::toJson() const {
    std::stringstream ss;
    ss << "{"
       << "\"id\":" << id_ << ","
       << "\"type\":\"zombie\","
       << "\"x\":" << position_.x << ","
       << "\"y\":" << position_.y << ","
       << "\"health\":" << health_ << ","
       << "\"maxHealth\":" << maxHealth_ << ","
       << "\"armor\":" << armor_ << ","
       << "\"hasBucket\":" << (hasBucket_ ? "true" : "false") << ","
       << "\"hasShield\":" << (shieldActive_ ? "true" : "false") << ","
       << "\"speedBoost\":" << speedBoostMultiplier_ << ","
       << "\"direction\":\"" << static_cast<int>(direction_) << "\","
       << "\"state\":\"" << static_cast<int>(state_) << "\","
       << "\"alive\":" << (alive_ ? "true" : "false") << ","
       << "\"inventory\":[";

    // 序列化道具栏
    for (size_t i = 0; i < inventory_.size(); ++i) {
        if (inventory_[i]) {
            ss << "{\"slot\":" << i << ",\"item\":" << inventory_[i]->toJson() << "}";
        } else {
            ss << "null";
        }
        if (i < inventory_.size() - 1) ss << ",";
    }

    ss << "]}";
    return ss.str();
}
