/**
 * @file Zombie.cpp
 * @brief 僵尸类实现
 */

#include "../../include/entities/Zombie.h"
#include "../../include/entities/Item.h"
#include "../../include/entities/Plant.h"
#include "../../include/entities/EntityManager.h"
#include "../../include/maze/Maze.h"
#include "../../include/core/Config.h"
#include <sstream>
#include <algorithm>

Zombie::Zombie(float x, float y)
    : Entity(x, y, EntityType::ZOMBIE),
      state_(ZombieState::IDLE),
      form_(ZombieForm::NORMAL),
      isMoving_(false),
      inputDirection_(0, 0),
      normalSpeed_(Config::ZOMBIE_SPEED),
      poleVaultSpeed_(Config::ZOMBIE_SPEED * 0.85f),  // 撑杆跳速度调整为普通的0.85倍
      maxInventorySize_(6),
      hasBucket_(false),
      hasPoleVault_(false),
      poleVaultJumped_(false),
      poleVaultJumping_(false),
      jumpAnimationTimer_(0),
      jumpAnimationDuration_(1.75f),  // 42帧 / 24fps = 1.75秒（与普通僵尸走路帧率一致）
      jumpDistance_(300.0f),  // 跳跃距离300像素
      jumpDirection_(Direction::RIGHT),  // 默认向右跳
      armor_(0),
      maxArmor_(200),
      entityManager_(nullptr),
      maze_(nullptr),
      shieldActive_(false),
      shieldTimer_(0),
      speedBoostMultiplier_(1.0f),
      speedBoostTimer_(0),
      damageInvulnerabilityTimer_(0),
      damageInvulnerabilityDuration_(0.5f),
      currentEatingPlant_(nullptr),
      eatDamagePerSecond_(Config::ZOMBIE_EAT_DPS),
      eatDamageTimer_(0.0f) {

    // 设置僵尸属性
    speed_ = normalSpeed_;
    health_ = Config::INITIAL_ZOMBIE_HEALTH;
    maxHealth_ = Config::INITIAL_ZOMBIE_HEALTH;
    setSize(60, 80);  // 碰撞盒大小

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

    // 处理撑杆跳跃动画
    if (poleVaultJumping_) {
        jumpAnimationTimer_ += deltaTime;

        // 计算本帧移动距离（跳跃过程中逐渐移动，使动画和位移同步）
        float jumpSpeed = jumpDistance_ / jumpAnimationDuration_;  // 像素/秒
        float frameMoveDistance = jumpSpeed * deltaTime;

        // 根据跳跃方向计算移动方向（支持四个方向跳跃）
        float moveX = 0;
        float moveY = 0;
        switch (jumpDirection_) {
            case Direction::UP:    moveY = -frameMoveDistance; break;
            case Direction::DOWN:  moveY = frameMoveDistance; break;
            case Direction::LEFT:  moveX = -frameMoveDistance; break;
            case Direction::RIGHT: moveX = frameMoveDistance; break;
            default: moveX = frameMoveDistance; break;  // 默认向右
        }

        // 应用本帧位移（检查边界和墙壁碰撞）
        if (maze_) {
            float newX = position_.x + moveX;
            float newY = position_.y + moveY;
            float margin = 50.0f;

            // 边界检查
            newX = std::max(margin, std::min(maze_->getPixelWidth() - margin, newX));
            newY = std::max(margin, std::min(maze_->getPixelHeight() - margin, newY));

            // 墙壁碰撞检测 - 检查目标位置是否可通行
            // 检查碰撞盒的四个角和中心
            float halfWidth = 25.0f;
            float halfHeight = 40.0f;

            bool canMove = true;
            // 检查目标位置的多个点
            if (!maze_->isPassableAtPixel(newX, newY) ||
                !maze_->isPassableAtPixel(newX - halfWidth, newY) ||
                !maze_->isPassableAtPixel(newX + halfWidth, newY) ||
                !maze_->isPassableAtPixel(newX, newY - halfHeight) ||
                !maze_->isPassableAtPixel(newX, newY + halfHeight)) {
                canMove = false;
            }

            if (canMove) {
                position_.x = newX;
                position_.y = newY;
            } else {
                // 遇到墙壁，停止跳跃
                poleVaultJumping_ = false;
                poleVaultJumped_ = true;
                jumpAnimationTimer_ = 0;
                speed_ = normalSpeed_;
                form_ = ZombieForm::NORMAL;
                setState(ZombieState::WALKING);
                return;  // 提前结束跳跃
            }
        } else {
            position_.x += moveX;
            position_.y += moveY;
        }

        // 跳跃动画完成
        if (jumpAnimationTimer_ >= jumpAnimationDuration_) {
            poleVaultJumping_ = false;
            poleVaultJumped_ = true;
            // hasPoleVault_ 保持为true，让前端继续使用pole_walk动画
            // 但form_变为NORMAL，速度恢复正常
            jumpAnimationTimer_ = 0;

            // 跳跃后速度恢复正常，形态变为普通（不再是撑杆跳僵尸）
            speed_ = normalSpeed_;
            form_ = ZombieForm::NORMAL;

            setState(ZombieState::WALKING);
        } else {
            // 跳跃中，设置状态
            setState(ZombieState::JUMPING);
        }
    } else {
        // 检查植物交互（吃植物会阻止移动）
        updatePlantInteraction(deltaTime);

        // 如果不在吃植物，正常更新移动
        if (state_ != ZombieState::EATING) {
            updateMovement(deltaTime);
        }
    }

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
    // 跳跃期间不接受移动输入
    if (poleVaultJumping_) return;
    inputDirection_.y = -1;
    direction_ = Direction::UP;
    isMoving_ = true;
}

void Zombie::moveDown() {
    // 跳跃期间不接受移动输入
    if (poleVaultJumping_) return;
    inputDirection_.y = 1;
    direction_ = Direction::DOWN;
    isMoving_ = true;
}

void Zombie::moveLeft() {
    // 跳跃期间不接受移动输入
    if (poleVaultJumping_) return;
    inputDirection_.x = -1;
    direction_ = Direction::LEFT;
    isMoving_ = true;
}

void Zombie::moveRight() {
    // 跳跃期间不接受移动输入
    if (poleVaultJumping_) return;
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

        // 计算新位置
        Vector2D newPosition = position_ + velocity_ * deltaTime;

        // 如果有迷宫引用，检查墙壁碰撞
        if (maze_) {
            int cellSize = maze_->getCellSize();

            // 碰撞检测参数
            // 精灵原点在底部中心(0.5, 1)，所以position表示脚底中心
            // 视觉精灵向上延伸，需要检查整个视觉边界
            float halfWidth = 25.0f;   // 碰撞宽度的一半
            float spriteHeight = 90.0f; // 精灵视觉高度（从脚到头）

            bool canMoveX = true;
            bool canMoveY = true;

            // 检查X方向移动 - 检查移动方向边缘的上中下三个点
            if (velocity_.x != 0) {
                float checkX = velocity_.x > 0 ? newPosition.x + halfWidth : newPosition.x - halfWidth;
                // 检查三个垂直点：脚、腰、头
                float checkYBottom = newPosition.y - 5.0f;  // 脚部附近
                float checkYMiddle = newPosition.y - spriteHeight * 0.5f;  // 中间
                float checkYTop = newPosition.y - spriteHeight + 5.0f;  // 头部附近

                if (!maze_->isPassableAtPixel(checkX, checkYBottom) ||
                    !maze_->isPassableAtPixel(checkX, checkYMiddle) ||
                    !maze_->isPassableAtPixel(checkX, checkYTop)) {
                    canMoveX = false;
                }
            }

            // 检查Y方向移动
            if (velocity_.y != 0) {
                // 向上移动 - 检查头部（精灵顶部）
                // 向下移动 - 检查脚部（精灵底部）
                float checkY = velocity_.y < 0 ? newPosition.y - spriteHeight : newPosition.y;
                // 检查三个水平点：左、中、右
                float checkXLeft = newPosition.x - halfWidth;
                float checkXMiddle = newPosition.x;
                float checkXRight = newPosition.x + halfWidth;

                if (!maze_->isPassableAtPixel(checkXLeft, checkY) ||
                    !maze_->isPassableAtPixel(checkXMiddle, checkY) ||
                    !maze_->isPassableAtPixel(checkXRight, checkY)) {
                    canMoveY = false;
                }
            }

            // 应用可移动方向的速度
            if (canMoveX) {
                position_.x = newPosition.x;
            }
            if (canMoveY) {
                position_.y = newPosition.y;
            }

            // 确保僵尸不会走出迷宫边界（考虑精灵高度）
            float marginX = halfWidth + 10.0f;
            float marginYBottom = 10.0f;
            float marginYTop = spriteHeight + 10.0f;
            position_.x = std::max(marginX, std::min(maze_->getPixelWidth() - marginX, position_.x));
            position_.y = std::max(marginYTop, std::min(maze_->getPixelHeight() - marginYBottom, position_.y));
        } else {
            // 没有迷宫引用，直接移动
            position_ = newPosition;
        }

        // 根据形态设置状态为行走或跑动
        // 撑杆跳僵尸跳跃后应该像普通僵尸一样走路
        if (form_ == ZombieForm::POLE_VAULTER && !poleVaultJumped_) {
            if (state_ != ZombieState::RUNNING) {
                setState(ZombieState::RUNNING);
            }
        } else {
            if (state_ != ZombieState::WALKING) {
                setState(ZombieState::WALKING);
            }
        }
    } else {
        // 停止移动
        velocity_ = Vector2D(0, 0);

        // 设置状态为待机
        if (state_ == ZombieState::WALKING || state_ == ZombieState::RUNNING) {
            setState(ZombieState::IDLE);
        }
    }

    // 重置输入方向（每帧清空，等待新的输入）
    inputDirection_ = Vector2D(0, 0);
}

// ==================== 道具系统 ====================

void Zombie::pickupItem(Item* item) {
    if (!item || !item->isAlive()) return;
    if (!item->canBePickedUp()) return;  // 检查是否可以拾取（包括免疫时间检查）

    // 直接应用道具效果（撑杆跳套装和铁桶）
    bool consumed = item->applyEffect(this);

    if (consumed) {
        // 标记道具已被拾取并销毁
        item->setPickedUp(true);
        item->setHealth(0);
    }
}

void Zombie::useItem(int slot) {
    if (slot < 0 || slot >= maxInventorySize_) return;

    Item* item = inventory_[slot];
    if (item) {
        item->applyEffect(this);

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

void Zombie::equipBucket(float armorValue) {
    // 只有在持有撑杆跳且还没跳过的时候，才会掉落撑杆跳套装
    // 跳过之后撑杆跳已经用掉了，不需要掉落
    if (hasPoleVault_ && !poleVaultJumped_) {
        dropEquipmentAtPosition(ZombieForm::POLE_VAULTER);
        removePoleVault();
    }

    hasBucket_ = true;
    armor_ = armorValue;
    maxArmor_ = armorValue;
    form_ = ZombieForm::BUCKET;
    updateSpeedBasedOnForm();
}

void Zombie::equipPoleVault() {
    // 如果已经装备铁桶，先掉落铁桶
    if (hasBucket_) {
        dropEquipmentAtPosition(ZombieForm::BUCKET);
        removeBucket();
    }

    hasPoleVault_ = true;
    poleVaultJumped_ = false;  // 重置跳跃状态
    form_ = ZombieForm::POLE_VAULTER;
    updateSpeedBasedOnForm();
}

void Zombie::performPoleVaultJump() {
    // 只有持有撑杆、未跳跃过、且当前不在跳跃中才能执行
    if (!hasPoleVault_ || poleVaultJumped_ || poleVaultJumping_) return;

    // 记录跳跃方向（使用当前朝向）
    // 支持四个方向跳跃：上、下、左、右
    jumpDirection_ = direction_;

    // 开始跳跃动画
    poleVaultJumping_ = true;
    jumpAnimationTimer_ = 0;

    // 跳跃中停止移动
    velocity_ = Vector2D(0, 0);
    inputDirection_ = Vector2D(0, 0);

    setState(ZombieState::JUMPING);
}

void Zombie::removeBucket() {
    hasBucket_ = false;
    armor_ = 0;

    // 如果同时没有撑杆跳，恢复普通形态
    if (!hasPoleVault_) {
        form_ = ZombieForm::NORMAL;
    }

    updateSpeedBasedOnForm();
}

void Zombie::removePoleVault() {
    hasPoleVault_ = false;
    poleVaultJumped_ = false;   // 重置跳跃状态，以便再次获得撑杆时可以使用
    poleVaultJumping_ = false;  // 重置跳跃中状态
    jumpAnimationTimer_ = 0;

    // 如果同时没有铁桶，恢复普通形态
    if (!hasBucket_) {
        form_ = ZombieForm::NORMAL;
    }

    updateSpeedBasedOnForm();
}

void Zombie::dropEquipmentAtPosition(ZombieForm formToDrop) {
    if (!entityManager_) return;

    // 在当前位置稍微偏移的地方生成掉落的道具（避免立即碰撞）
    float offsetX = (rand() % 2 == 0) ? 50.0f : -50.0f;
    float offsetY = (rand() % 2 == 0) ? 50.0f : -50.0f;

    Item* droppedItem = nullptr;

    if (formToDrop == ZombieForm::BUCKET) {
        // 掉落铁桶时保留当前护甲值
        droppedItem = new Bucket(position_.x + offsetX, position_.y + offsetY, armor_);
    } else if (formToDrop == ZombieForm::POLE_VAULTER) {
        droppedItem = new PoleVaultKit(position_.x + offsetX, position_.y + offsetY);
    }

    if (droppedItem) {
        // 设置1秒的拾取免疫时间，防止立即被拾取
        droppedItem->setPickupImmunity(1.0f);
        entityManager_->addItem(droppedItem);
    }
}

void Zombie::updateSpeedBasedOnForm() {
    // 根据形态设置移动速度
    // 撑杆跳僵尸跳跃后使用普通速度
    if (form_ == ZombieForm::POLE_VAULTER && !poleVaultJumped_) {
        speed_ = poleVaultSpeed_;
    } else {
        speed_ = normalSpeed_;
    }
}

// ==================== 增益效果 ====================

void Zombie::applySpeedBoost(float multiplier, float duration) {
    // 如果已有相同倍率的加速效果，叠加持续时间
    if (speedBoostMultiplier_ == multiplier && speedBoostTimer_ > 0) {
        speedBoostTimer_ += duration;
    } else {
        // 新的加速效果
        speedBoostMultiplier_ = multiplier;
        speedBoostTimer_ = duration;
    }
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

// ==================== 治疗 ====================

void Zombie::heal(float amount) {
    health_ += amount;
    if (health_ > maxHealth_) {
        health_ = maxHealth_;
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

    // 跑动动画（循环，撑杆跳僵尸专用）
    AnimationClip* runAnim = new AnimationClip("run", true);
    runAnim->addFrame("assets/images/zombies/run/frame_0.png", 0.08f);
    runAnim->addFrame("assets/images/zombies/run/frame_1.png", 0.08f);
    runAnim->addFrame("assets/images/zombies/run/frame_2.png", 0.08f);
    runAnim->addFrame("assets/images/zombies/run/frame_3.png", 0.08f);
    runAnim->addFrame("assets/images/zombies/run/frame_4.png", 0.08f);
    runAnim->addFrame("assets/images/zombies/run/frame_5.png", 0.08f);
    animationController_.registerAnimation(runAnim);

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

        case ZombieState::RUNNING:
            // 撑杆跳僵尸使用跑动动画
            if (!animationController_.isPlaying("run")) {
                animationController_.play("run");
            }
            break;

        case ZombieState::DAMAGED:
            if (!animationController_.isPlaying("damaged")) {
                animationController_.play("damaged", true);
            }
            // 受伤动画播放完毕后回到待机或行走/跑动
            if (animationController_.isCurrentAnimationFinished()) {
                if (isMoving_) {
                    if (form_ == ZombieForm::POLE_VAULTER) {
                        setState(ZombieState::RUNNING);
                    } else {
                        setState(ZombieState::WALKING);
                    }
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
       << "\"vx\":" << velocity_.x << ","
       << "\"vy\":" << velocity_.y << ","
       << "\"health\":" << health_ << ","
       << "\"maxHealth\":" << maxHealth_ << ","
       << "\"armor\":" << armor_ << ","
       << "\"maxArmor\":" << maxArmor_ << ","
       << "\"form\":" << static_cast<int>(form_) << ","
       << "\"equipment\":\"" << (hasBucket_ ? "bucket" : (hasPoleVault_ ? "pole_vault" : "normal")) << "\","
       << "\"hasBucket\":" << (hasBucket_ ? "true" : "false") << ","
       << "\"hasPoleVault\":" << (hasPoleVault_ ? "true" : "false") << ","
       << "\"poleVaultJumped\":" << (poleVaultJumped_ ? "true" : "false") << ","
       << "\"poleVaultJumping\":" << (poleVaultJumping_ ? "true" : "false") << ","
       << "\"jumpDirection\":" << static_cast<int>(jumpDirection_) << ","
       << "\"hasShield\":" << (shieldActive_ ? "true" : "false") << ","
       << "\"speedBoost\":" << speedBoostMultiplier_ << ","
       << "\"direction\":\"" << static_cast<int>(direction_) << "\","
       << "\"state\":\"" << static_cast<int>(state_) << "\","
       << "\"isMoving\":" << (isMoving_ ? "true" : "false") << ","
       << "\"isEating\":" << (state_ == ZombieState::EATING ? "true" : "false") << ","
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

// ==================== 植物交互 ====================

void Zombie::updatePlantInteraction(float deltaTime) {
    // 检查是否碰到植物
    Plant* collidingPlant = checkPlantCollision();

    if (collidingPlant) {
        // 如果是新的植物，重置计时器
        if (currentEatingPlant_ != collidingPlant) {
            eatDamageTimer_ = 0.0f;
        }

        // 碰到植物，开始吃
        currentEatingPlant_ = collidingPlant;
        setState(ZombieState::EATING);
        velocity_ = Vector2D(0, 0);  // 停止移动

        // 对植物造成伤害
        eatPlant(collidingPlant, deltaTime);
    } else {
        // 没有碰到植物
        if (state_ == ZombieState::EATING) {
            // 之前在吃植物，现在恢复行走
            currentEatingPlant_ = nullptr;
            eatDamageTimer_ = 0.0f;  // 重置计时器
            if (isMoving_) {
                // 撑杆跳僵尸跳跃后使用普通速度
                if (form_ == ZombieForm::POLE_VAULTER && !poleVaultJumped_) {
                    setState(ZombieState::RUNNING);
                } else {
                    setState(ZombieState::WALKING);
                }
            } else {
                setState(ZombieState::IDLE);
            }
        }
    }
}

Plant* Zombie::checkPlantCollision() const {
    if (!entityManager_) return nullptr;

    // 获取附近的所有植物
    float checkRange = 100.0f;  // 碰撞检测范围（植物尺寸增大到80x80后需要更大范围）
    auto entities = entityManager_->findEntitiesInRange(position_, checkRange);

    for (Entity* entity : entities) {
        if (entity->getType() == EntityType::PLANT && entity->isAlive()) {
            Plant* plant = dynamic_cast<Plant*>(entity);
            if (plant) {
                // 检测碰撞盒重叠
                float dx = std::abs(position_.x - plant->getPosition().x);
                float dy = std::abs(position_.y - plant->getPosition().y);

                float combinedWidth = (width_ + plant->getWidth()) / 2.0f;
                float combinedHeight = (height_ + plant->getHeight()) / 2.0f;

                if (dx < combinedWidth && dy < combinedHeight) {
                    return plant;
                }
            }
        }
    }

    return nullptr;
}

void Zombie::eatPlant(Plant* plant, float deltaTime) {
    if (!plant || !plant->isAlive()) return;

    // 累积吃植物时间
    eatDamageTimer_ += deltaTime;

    // 每秒结算一次伤害（秒末结算）
    if (eatDamageTimer_ >= 1.0f) {
        // 造成一次完整的每秒伤害
        plant->takeDamage(eatDamagePerSecond_);
        eatDamageTimer_ -= 1.0f;  // 减去一秒，保留多余时间
    }

    // 如果植物被吃掉了
    if (!plant->isAlive()) {
        currentEatingPlant_ = nullptr;
        eatDamageTimer_ = 0.0f;  // 重置计时器

        // 清除迷宫中的植物标记
        if (maze_) {
            int gridX, gridY;
            maze_->pixelToGrid(plant->getPosition().x, plant->getPosition().y, gridX, gridY);
            if (maze_->isInBounds(gridX, gridY)) {
                maze_->getCell(gridX, gridY).hasPlant = false;
            }
        }
    }
}
