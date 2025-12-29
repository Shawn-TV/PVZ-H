/**
 * @file Zombie.cpp
 * @brief 僵尸类实现
 */

#include "../../include/entities/Zombie.h"
#include "../../include/entities/Item.h"
#include "../../include/entities/Plant.h"
#include "../../include/entities/Dave.h"
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
      jumpAnimationDuration_(1.75f),   // 42帧 / 24fps = 1.75秒（与前端动画同步）
      jumpDistance_(175.0f),  // 跳跃距离400像素，匹配动画帧内的视觉位移
      jumpStartPosition_(0, 0),  // 跳跃起始位置
      jumpDirection_(Direction::LEFT),  // 默认向左跳（与走路方向一致）
      armor_(0),
      maxArmor_(200),
      entityManager_(nullptr),
      maze_(nullptr),
      shieldActive_(false),
      shieldTimer_(0),
      speedBoostMultiplier_(1.0f),
      speedBoostTimer_(0),
      damageInvulnerabilityTimer_(0),
      damageInvulnerabilityDuration_(0.1f),  // 减少无敌时间，允许双发射手两颗豌豆都造成伤害
      attackStunTimer_(0),
      attackStunDuration_(0.5f),  // 被攻击后0.5秒无法对植物造成伤害
      currentEatingPlant_(nullptr),
      eatDamagePerSecond_(Config::ZOMBIE_EAT_DPS),
      eatDamageTimer_(0.0f),
      currentAttackingDave_(nullptr),
      attackDaveTimer_(0.0f),
      itemPickupCooldown_(0.0f),
      itemPickupCooldownDuration_(5.0f) {  // 5秒拾取冷却，防止重复拾取/放下

    // 设置僵尸属性
    speed_ = normalSpeed_;
    health_ = Config::INITIAL_ZOMBIE_HEALTH;
    maxHealth_ = Config::INITIAL_ZOMBIE_HEALTH;
    setSize(70, 120);  // 碰撞盒大小，匹配精灵尺寸(100x139 * 0.9)

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
    // 每帧检查生命值，立即处理死亡
    if (health_ <= 0 && alive_) {
        alive_ = false;
        setState(ZombieState::DEAD);
        // 游戏结束判定由Game::checkLoseCondition处理
    }

    if (!alive_) {
        setState(ZombieState::DEAD);
        return;
    }

    // 处理撑杆跳跃动画
    // 人物位置在动画期间保持不变，动画结束时瞬移到目标
    // 摄像机通过前端独立控制，平滑移动到目标位置
    if (poleVaultJumping_) {
        jumpAnimationTimer_ += deltaTime;

        // 计算目标位置偏移
        Vector2D jumpOffset(0, 0);
        switch (jumpDirection_) {
            case Direction::UP:    jumpOffset.y = -jumpDistance_; break;
            case Direction::DOWN:  jumpOffset.y = jumpDistance_; break;
            case Direction::LEFT:  jumpOffset.x = -jumpDistance_; break;
            case Direction::RIGHT: jumpOffset.x = jumpDistance_; break;
            default: jumpOffset.x = jumpDistance_; break;
        }

        // 动画期间位置保持不变，让帧内偏移提供视觉效果
        // position_ 保持为 jumpStartPosition_

        // 跳跃动画完成
        if (jumpAnimationTimer_ >= jumpAnimationDuration_) {
            poleVaultJumping_ = false;
            poleVaultJumped_ = true;
            jumpAnimationTimer_ = 0;
            velocity_ = Vector2D(0, 0);

            // 确保最终位置精确
            Vector2D finalPosition = jumpStartPosition_ + jumpOffset;

            // 边界检查
            if (maze_) {
                float margin = 25.0f;
                finalPosition.x = std::max(margin, std::min(maze_->getPixelWidth() - margin, finalPosition.x));
                finalPosition.y = std::max(margin, std::min(maze_->getPixelHeight() - margin, finalPosition.y));
            }

            // 检查最终位置是否在墙里
            if (maze_ && !maze_->isPassableAtPixel(finalPosition.x, finalPosition.y)) {
                // 如果在墙里，向反方向回退找到可通行位置
                float stepSize = 10.0f;
                float maxBacktrack = jumpDistance_;
                bool foundValidPosition = false;

                for (float backtrack = stepSize; backtrack <= maxBacktrack; backtrack += stepSize) {
                    float testX = finalPosition.x;
                    float testY = finalPosition.y;

                    switch (jumpDirection_) {
                        case Direction::UP:    testY += backtrack; break;
                        case Direction::DOWN:  testY -= backtrack; break;
                        case Direction::LEFT:  testX += backtrack; break;
                        case Direction::RIGHT: testX -= backtrack; break;
                        default: testX -= backtrack; break;
                    }

                    if (maze_->isPassableAtPixel(testX, testY)) {
                        finalPosition.x = testX;
                        finalPosition.y = testY;
                        foundValidPosition = true;
                        break;
                    }
                }

                if (!foundValidPosition) {
                    finalPosition = jumpStartPosition_;
                }
            }

            position_ = finalPosition;

            // 跳跃后速度恢复正常，形态变为普通
            speed_ = normalSpeed_;
            form_ = ZombieForm::NORMAL;
            setState(ZombieState::WALKING);
        } else {
            setState(ZombieState::JUMPING);
        }
    } else {
        // 如果玩家按方向键移动，可能退出攻击状态
        bool brokeOutOfEating = false;
        if (isMoving_ && state_ == ZombieState::EATING) {
            // 注意：攻击戴夫时不允许通过移动来中断攻击
            // 只有吃植物时才能通过移动来中断
            if (currentAttackingDave_ == nullptr && currentEatingPlant_ != nullptr) {
                // 正在吃植物，允许玩家移动来中断
                currentEatingPlant_ = nullptr;
                setState(ZombieState::WALKING);
                brokeOutOfEating = true;
                // 先执行移动让僵尸离开碰撞区域
                updateMovement(deltaTime);
            }
            // 如果正在攻击戴夫，不中断攻击，继续造成伤害直到戴夫死亡
        }

        // 如果刚从攻击状态退出，跳过本帧的交互检测，给僵尸移动的机会
        if (!brokeOutOfEating) {
            // 检查戴夫交互（攻击戴夫优先于吃植物）
            // 即使在EATING状态也要继续调用updateDaveInteraction来持续造成伤害
            // 修复：如果在EATING状态但既不攻击Dave也不吃植物，也需要检测Dave
            bool shouldCheckDave = (state_ != ZombieState::EATING) ||
                                   (currentAttackingDave_ != nullptr) ||
                                   (currentEatingPlant_ == nullptr);  // 没在吃植物时也要检测
            if (shouldCheckDave) {
                updateDaveInteraction(deltaTime);
            }

            // 如果不在攻击戴夫，检查植物交互
            if (state_ != ZombieState::EATING || currentAttackingDave_ == nullptr) {
                updatePlantInteraction(deltaTime);
            }

            // 如果不在吃植物或攻击戴夫，正常更新移动
            if (state_ != ZombieState::EATING) {
                updateMovement(deltaTime);
            }
        }
    }

    // 更新增益效果
    updateBuffs(deltaTime);

    // 更新无敌时间
    if (damageInvulnerabilityTimer_ > 0) {
        damageInvulnerabilityTimer_ -= deltaTime;
    }

    // 更新攻击硬直时间
    if (attackStunTimer_ > 0) {
        attackStunTimer_ -= deltaTime;
    }

    // 更新道具拾取冷却
    if (itemPickupCooldown_ > 0) {
        itemPickupCooldown_ -= deltaTime;
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
    // 如果正在攻击戴夫，完全停止移动
    if (currentAttackingDave_ != nullptr) {
        velocity_ = Vector2D(0, 0);
        return;
    }

    // 如果正在吃植物，也停止移动
    if (currentEatingPlant_ != nullptr) {
        velocity_ = Vector2D(0, 0);
        return;
    }

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

            // 检查植物碰撞（僵尸不能穿过植物）
            if (entityManager_ && (canMoveX || canMoveY)) {
                auto plants = entityManager_->getPlants();
                for (auto* plant : plants) {
                    if (!plant || !plant->isAlive()) continue;

                    // 计算僵尸新位置的边界框
                    float zombieLeft = (canMoveX ? newPosition.x : position_.x) - halfWidth;
                    float zombieRight = (canMoveX ? newPosition.x : position_.x) + halfWidth;
                    float zombieTop = (canMoveY ? newPosition.y : position_.y) - spriteHeight;
                    float zombieBottom = (canMoveY ? newPosition.y : position_.y);

                    // 获取植物边界框（植物原点在中心）
                    float plantHalfWidth = plant->getWidth() / 2.0f;
                    float plantHalfHeight = plant->getHeight() / 2.0f;
                    float plantLeft = plant->getPosition().x - plantHalfWidth;
                    float plantRight = plant->getPosition().x + plantHalfWidth;
                    float plantTop = plant->getPosition().y - plantHalfHeight;
                    float plantBottom = plant->getPosition().y + plantHalfHeight;

                    // AABB碰撞检测
                    bool collision = zombieLeft < plantRight &&
                                   zombieRight > plantLeft &&
                                   zombieTop < plantBottom &&
                                   zombieBottom > plantTop;

                    if (collision) {
                        // 检查是X还是Y方向的移动导致碰撞
                        float oldZombieLeft = position_.x - halfWidth;
                        float oldZombieRight = position_.x + halfWidth;
                        float oldZombieTop = position_.y - spriteHeight;
                        float oldZombieBottom = position_.y;

                        // 只检查X方向
                        bool xCollision = (position_.x - halfWidth < plantRight && position_.x + halfWidth > plantLeft) == false &&
                                         newPosition.x - halfWidth < plantRight && newPosition.x + halfWidth > plantLeft &&
                                         oldZombieTop < plantBottom && oldZombieBottom > plantTop;
                        // 只检查Y方向
                        bool yCollision = (position_.y - spriteHeight < plantBottom && position_.y > plantTop) == false &&
                                         newPosition.y - spriteHeight < plantBottom && newPosition.y > plantTop &&
                                         oldZombieLeft < plantRight && oldZombieRight > plantLeft;

                        if (canMoveX && xCollision) {
                            canMoveX = false;
                        }
                        if (canMoveY && yCollision) {
                            canMoveY = false;
                        }
                    }
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

    // 检查拾取冷却
    if (itemPickupCooldown_ > 0) return;

    // 直接应用道具效果（撑杆跳套装和铁桶）
    bool consumed = item->applyEffect(this);

    if (consumed) {
        // 标记道具已被拾取并销毁
        item->setPickedUp(true);
        item->setHealth(0);

        // 设置拾取冷却
        itemPickupCooldown_ = itemPickupCooldownDuration_;
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
    // 如果已经有铁桶，先掉落旧铁桶（保留当前防御值）
    if (hasBucket_ && armor_ > 0) {
        dropEquipmentAtPosition(ZombieForm::BUCKET);
    }

    // 如果持有撑杆跳且还没跳过，掉落撑杆跳
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

    // 记录跳跃起始位置（用于动画结束时计算目标位置）
    jumpStartPosition_ = position_;

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
        // 设置0.2秒的短暂拾取免疫时间，让物品显示出来后可以快速拾取
        droppedItem->setPickupImmunity(0.2f);
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

        case ZombieState::EATING:
            // 吃植物或攻击戴夫时使用吃的动画
            if (!animationController_.isPlaying("eat")) {
                animationController_.play("eat");
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

    // 设置攻击硬直时间（被攻击时无法对植物造成伤害）
    attackStunTimer_ = attackStunDuration_;
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
    float checkRange = 150.0f;  // 碰撞检测范围
    auto entities = entityManager_->findEntitiesInRange(position_, checkRange);

    // 僵尸的碰撞盒 - 原点在底部中心
    // 使用与updateMovement相同的参数以保持一致性
    float zombieHalfWidth = 25.0f;  // 与updateMovement一致
    float spriteHeight = 90.0f;  // 与updateMovement保持一致

    // 添加攻击范围缓冲，使僵尸能在接近时就开始攻击
    // 这解决了移动碰撞阻止僵尸接近植物的问题
    float attackBuffer = 10.0f;

    float zombieTop = position_.y - spriteHeight - attackBuffer;  // 头部（扩大范围）
    float zombieBottom = position_.y + attackBuffer;  // 脚部（扩大范围）
    float zombieLeft = position_.x - zombieHalfWidth - attackBuffer;
    float zombieRight = position_.x + zombieHalfWidth + attackBuffer;

    for (Entity* entity : entities) {
        if (entity->getType() == EntityType::PLANT && entity->isAlive()) {
            Plant* plant = dynamic_cast<Plant*>(entity);
            if (plant) {
                // 植物的碰撞盒 - 原点在中心
                float plantHalfWidth = plant->getWidth() / 2.0f;
                float plantHalfHeight = plant->getHeight() / 2.0f;
                float plantLeft = plant->getPosition().x - plantHalfWidth;
                float plantRight = plant->getPosition().x + plantHalfWidth;
                float plantTop = plant->getPosition().y - plantHalfHeight;
                float plantBottom = plant->getPosition().y + plantHalfHeight;

                // AABB碰撞检测（带攻击缓冲范围）
                bool collision = zombieLeft < plantRight &&
                               zombieRight > plantLeft &&
                               zombieTop < plantBottom &&
                               zombieBottom > plantTop;

                if (collision) {
                    return plant;
                }
            }
        }
    }

    return nullptr;
}

void Zombie::eatPlant(Plant* plant, float deltaTime) {
    if (!plant || !plant->isAlive()) return;

    // 如果正在受到攻击（攻击硬直中），不能对植物造成伤害
    if (attackStunTimer_ > 0) return;

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

// ==================== 戴夫交互 ====================

void Zombie::updateDaveInteraction(float deltaTime) {
    // 检查是否碰到戴夫
    Dave* collidingDave = checkDaveCollision();

    if (collidingDave) {
        // 碰到戴夫，开始攻击（优先于吃植物）
        currentAttackingDave_ = collidingDave;
        setState(ZombieState::EATING);  // 使用吃的动画
        velocity_ = Vector2D(0, 0);  // 停止移动

        // 对戴夫造成伤害
        attackDave(collidingDave, deltaTime);
    } else {
        // 没有直接碰到戴夫
        if (currentAttackingDave_ != nullptr) {
            // 之前在攻击戴夫，检查戴夫是否已死亡、眩晕或太远了
            Dave* dave = currentAttackingDave_;

            // 如果戴夫已死亡或眩晕，停止攻击并重置状态
            if (!dave->isAlive() || dave->getHealth() <= 0 || dave->isStunned()) {
                currentAttackingDave_ = nullptr;
                attackDaveTimer_ = 0.0f;
                // 重置状态为待机或行走（修复状态卡在EATING的问题）
                if (currentEatingPlant_ == nullptr) {
                    if (isMoving_) {
                        if (form_ == ZombieForm::POLE_VAULTER && !poleVaultJumped_) {
                            setState(ZombieState::RUNNING);
                        } else {
                            setState(ZombieState::WALKING);
                        }
                    } else {
                        setState(ZombieState::IDLE);
                    }
                }
            } else {
                // 戴夫还活着，检查距离
                // 使用更大的"脱离攻击"范围，防止因微小移动导致攻击中断
                float dx = std::abs(position_.x - dave->getPosition().x);
                float dy = std::abs(position_.y - dave->getPosition().y);

                // 脱离范围比碰撞范围大30像素，形成滞后区间
                // 碰撞范围: width (70+30)/2=50, height (120+30)/2=75
                // 脱离范围: 80, 105
                float breakWidth = (width_ + dave->getWidth()) / 2.0f + 30.0f;
                float breakHeight = (height_ + dave->getHeight()) / 2.0f + 30.0f;

                if (dx < breakWidth && dy < breakHeight) {
                    // 仍在攻击范围内，继续攻击（不重置计时器！）
                    setState(ZombieState::EATING);
                    velocity_ = Vector2D(0, 0);
                    attackDave(dave, deltaTime);
                } else {
                    // 戴夫逃离了攻击范围，停止攻击
                    currentAttackingDave_ = nullptr;
                    attackDaveTimer_ = 0.0f;

                    // 恢复行走状态
                    if (currentEatingPlant_ == nullptr) {
                        if (isMoving_) {
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
        }
    }
}

Dave* Zombie::checkDaveCollision() const {
    if (!entityManager_) return nullptr;

    Dave* dave = entityManager_->getDave();
    if (!dave) return nullptr;

    // 如果戴夫已死亡，不再攻击
    if (!dave->isAlive() || dave->getHealth() <= 0) {
        return nullptr;
    }

    // 如果戴夫处于眩晕状态，不再攻击（让僵尸继续前进）
    if (dave->isStunned()) {
        return nullptr;
    }

    // 检测碰撞盒重叠
    float dx = std::abs(position_.x - dave->getPosition().x);
    float dy = std::abs(position_.y - dave->getPosition().y);

    float combinedWidth = (width_ + dave->getWidth()) / 2.0f;
    float combinedHeight = (height_ + dave->getHeight()) / 2.0f;

    if (dx < combinedWidth && dy < combinedHeight) {
        return dave;
    }

    return nullptr;
}

void Zombie::attackDave(Dave* dave, float deltaTime) {
    if (!dave) return;

    // 如果戴夫已死亡，停止攻击并清除状态
    if (!dave->isAlive() || dave->getHealth() <= 0) {
        currentAttackingDave_ = nullptr;
        attackDaveTimer_ = 0.0f;
        // 重置状态（修复状态卡在EATING的问题）
        if (currentEatingPlant_ == nullptr && state_ == ZombieState::EATING) {
            if (isMoving_) {
                if (form_ == ZombieForm::POLE_VAULTER && !poleVaultJumped_) {
                    setState(ZombieState::RUNNING);
                } else {
                    setState(ZombieState::WALKING);
                }
            } else {
                setState(ZombieState::IDLE);
            }
        }
        return;
    }

    // 累积攻击时间
    attackDaveTimer_ += deltaTime;

    // 每秒结算一次伤害
    if (attackDaveTimer_ >= 1.0f) {
        // 造成伤害（使用和吃植物相同的伤害）
        dave->takeDamage(eatDamagePerSecond_);
        attackDaveTimer_ -= 1.0f;
    }
}
