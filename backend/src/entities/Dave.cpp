/**
 * @file Dave.cpp
 * @brief 戴夫NPC类实现
 */

#include "../../include/entities/Dave.h"
#include "../../include/entities/Zombie.h"
#include "../../include/entities/EntityManager.h"
#include "../../include/maze/Maze.h"
#include "../../include/ai/AStar.h"
#include "../../include/core/Config.h"
#include "../../include/plants/PeaShooter.h"
#include "../../include/plants/DoublePeaShooter.h"
#include "../../include/plants/CherryBomb.h"
#include "../../include/plants/WallNut.h"
#include <sstream>
#include <cmath>
#include <iostream>

Dave::Dave(float x, float y, Maze* maze)
    : Entity(x, y, EntityType::DAVE),
      maze_(maze),
      target_(nullptr),
      currentPathIndex_(0),
      state_(DaveState::IDLE),
      attackDamage_(15.0f),
      attackRange_(40.0f),
      attackCooldown_(1.5f),
      currentAttackCooldown_(0),
      pathUpdateTimer_(0),
      pathUpdateInterval_(0.5f),  // 每0.5秒重新计算路径
      detectionRange_(5000.0f),  // 检测范围设为很大，确保能追踪整个迷宫
      isStunned_(false),
      stunTimer_(0),
      lowHpStunTriggered_(false),
      lastTargetPosition_(0, 0),
      entityManager_(nullptr),
      plantCooldown_(5.0f),         // 种植冷却5秒
      currentPlantCooldown_(3.0f),  // 初始冷却3秒，给多人模式切换命令时间
      sunlight_(200),               // 初始阳光200（单人模式）
      sunlightTimer_(0),            // 阳光生成计时器
      sunlightInterval_(20.0f),     // 每20秒生成阳光
      sunlightPerInterval_(50),     // 每次生成50阳光
      peaShooterCooldown_(10.0f),   // 豌豆射手冷却10秒
      repeaterCooldown_(20.0f),     // 双发射手冷却20秒
      cherryBombCooldown_(30.0f),   // 樱桃炸弹冷却30秒
      wallNutCooldown_(30.0f),      // 坚果墙冷却30秒
      currentPeaShooterCooldown_(0),
      currentRepeaterCooldown_(0),
      currentCherryBombCooldown_(0),
      currentWallNutCooldown_(0),
      isPlayerControlled_(false),   // 默认AI控制
      inputDirection_(0, 0),
      isMovingInput_(false),
      lastZombiePosition_(0, 0),
      zombieStuckTimer_(0),
      hasPlacedWalnut_(false) {

    // 设置戴夫属性
    speed_ = Config::DAVE_SPEED;
    maxHealth_ = 200.0f;  // 戴夫生命值（僵尸每秒50伤害，4秒可击杀）
    health_ = maxHealth_;
    setSize(30, 30);  // 设置碰撞盒大小（更小以便通过通道）

    // 初始化动画
    initializeAnimations();
}

Dave::~Dave() {
}

void Dave::update(float deltaTime) {
    // 每帧检查生命值，立即处理死亡
    if (health_ <= 0 && alive_) {
        alive_ = false;
        // 游戏胜利判定由Game::checkWinCondition处理
    }

    if (!alive_) return;

    // 更新眩晕状态
    if (isStunned_) {
        stunTimer_ -= deltaTime;
        if (stunTimer_ <= 0) {
            isStunned_ = false;
            // 眩晕结束后恢复到最大生命值的一半
            health_ = maxHealth_ / 2.0f;
            std::cout << "[Dave] Stun ended, HP restored to " << health_ << std::endl;
            setState(DaveState::IDLE);
        }
        // 眩晕时不进行AI更新
        updateAnimation();
        animationController_.update(deltaTime);
        return;
    }

    // 更新攻击冷却
    if (currentAttackCooldown_ > 0) {
        currentAttackCooldown_ -= deltaTime;
    }

    // 更新种植冷却（AI模式全局冷却）
    if (currentPlantCooldown_ > 0) {
        currentPlantCooldown_ -= deltaTime;
    }

    // 更新各植物冷却（玩家模式单独冷却）
    if (currentPeaShooterCooldown_ > 0) {
        currentPeaShooterCooldown_ -= deltaTime;
    }
    if (currentRepeaterCooldown_ > 0) {
        currentRepeaterCooldown_ -= deltaTime;
    }
    if (currentCherryBombCooldown_ > 0) {
        currentCherryBombCooldown_ -= deltaTime;
    }
    if (currentWallNutCooldown_ > 0) {
        currentWallNutCooldown_ -= deltaTime;
    }

    // 更新阳光生成（所有模式 - AI和玩家都需要阳光）
    updateSunlightGeneration(deltaTime);

    // 根据控制模式更新
    if (isPlayerControlled_) {
        updatePlayerControl(deltaTime);
    } else {
        updateAI(deltaTime);
    }

    // 更新动画
    updateAnimation();
    animationController_.update(deltaTime);
}

void Dave::onCollision(Entity* other) {
    if (!other || !alive_) return;

    // 戴夫没有直接攻击能力，只能通过植物攻击
    // 碰撞到僵尸时不进行攻击
}

// ==================== 目标设置 ====================

void Dave::setTarget(Zombie* zombie) {
    target_ = zombie;
    if (target_) {
        lastTargetPosition_ = target_->getPosition();
    }
}

// ==================== 路径规划 ====================

void Dave::updatePath() {
    if (!target_ || !target_->isAlive() || !maze_) {
        clearPath();
        return;
    }

    // 使用A*算法计算从当前位置到目标位置的路径
    Vector2D start = position_;
    Vector2D goal = target_->getPosition();

    path_ = AStar::findPath(start, goal, maze_);
    currentPathIndex_ = 0;

    // 更新最后的目标位置
    lastTargetPosition_ = goal;
}

void Dave::clearPath() {
    path_.clear();
    currentPathIndex_ = 0;
}

// ==================== AI 逻辑 ====================

void Dave::updateAI(float deltaTime) {
    // 尝试种植植物阻挡僵尸
    updatePlantingAI(deltaTime);

    // 如果没有目标或目标已死亡，进入待机状态
    if (!target_ || !target_->isAlive()) {
        setState(DaveState::IDLE);
        clearPath();
        velocity_ = Vector2D(0, 0);
        return;
    }

    // 计算与目标的距离
    float dist = distanceToTarget();

    // 如果僵尸非常近（正在攻击戴夫），停止移动让僵尸继续攻击
    // 碰撞检测使用边界框：zombieWidth=70, daveWidth=30, zombieHeight=120, daveHeight=30
    // combinedWidth = (70+30)/2 = 50, combinedHeight = (120+30)/2 = 75
    // 最大碰撞距离 = sqrt(50² + 75²) ≈ 90，使用100确保安全
    if (dist < 100.0f) {
        velocity_ = Vector2D(0, 0);
        // 不要更新路径，保持原地
        return;
    }

    // 如果目标在检测范围外，不追踪
    if (dist > detectionRange_) {
        setState(DaveState::IDLE);
        clearPath();
        velocity_ = Vector2D(0, 0);
        return;
    }

    // 戴夫没有直接攻击能力，即使在攻击范围内也只能依靠植物
    // 保持追踪状态，尝试种植植物来攻击

    // 否则，追踪目标
    setState(DaveState::CHASING);

    // 更新路径计时器
    pathUpdateTimer_ += deltaTime;

    // 定期重新计算路径，或者目标位置发生显著变化时重新计算
    bool targetMoved = (target_->getPosition() - lastTargetPosition_).length() > 50.0f;
    bool shouldUpdatePath = pathUpdateTimer_ >= pathUpdateInterval_ || targetMoved || path_.empty();

    if (shouldUpdatePath) {
        updatePath();
        pathUpdateTimer_ = 0;
    }

    // 沿着路径移动
    if (!path_.empty()) {
        followPath(deltaTime);
    } else {
        // 如果没有路径，直接朝目标移动（备用方案）
        moveTowardsPosition(target_->getPosition(), deltaTime);
    }
}

void Dave::followPath(float deltaTime) {
    if (path_.empty() || currentPathIndex_ >= static_cast<int>(path_.size())) {
        return;
    }

    // 获取当前路径点
    Vector2D targetWaypoint = path_[currentPathIndex_];

    // 计算到路径点的距离
    float distToWaypoint = (targetWaypoint - position_).length();

    // 如果足够接近当前路径点，移动到下一个路径点
    if (distToWaypoint < 5.0f) {
        currentPathIndex_++;
        if (currentPathIndex_ >= static_cast<int>(path_.size())) {
            // 到达路径终点
            clearPath();
            return;
        }
        targetWaypoint = path_[currentPathIndex_];
    }

    // 朝路径点移动
    moveTowardsPosition(targetWaypoint, deltaTime);
}

void Dave::moveTowardsPosition(const Vector2D& targetPos, float deltaTime) {
    // 计算方向
    Vector2D direction = (targetPos - position_).normalized();

    // 设置方向枚举（用于动画）
    if (std::abs(direction.x) > std::abs(direction.y)) {
        direction_ = (direction.x > 0) ? Direction::RIGHT : Direction::LEFT;
    } else {
        direction_ = (direction.y > 0) ? Direction::DOWN : Direction::UP;
    }

    // 设置速度
    velocity_ = direction * speed_;

    // 计算新位置
    Vector2D newPosition = position_ + velocity_ * deltaTime;

    // 如果有迷宫引用，检查墙壁碰撞
    if (maze_) {
        // 获取戴夫碰撞盒的半尺寸
        float halfWidth = width_ / 2.0f;
        float halfHeight = height_ / 2.0f;

        // 检查四个角点是否会碰到墙壁
        bool canMoveX = true;
        bool canMoveY = true;

        // 分别检查X和Y方向的移动
        Vector2D testPosX(newPosition.x, position_.y);
        Vector2D testPosY(position_.x, newPosition.y);

        // 检查X方向移动
        if (velocity_.x != 0) {
            float checkX = velocity_.x > 0 ? testPosX.x + halfWidth : testPosX.x - halfWidth;
            if (!maze_->isPassableAtPixel(checkX, position_.y - halfHeight) ||
                !maze_->isPassableAtPixel(checkX, position_.y + halfHeight)) {
                canMoveX = false;
            }
        }

        // 检查Y方向移动
        if (velocity_.y != 0) {
            float checkY = velocity_.y > 0 ? testPosY.y + halfHeight : testPosY.y - halfHeight;
            if (!maze_->isPassableAtPixel(position_.x - halfWidth, checkY) ||
                !maze_->isPassableAtPixel(position_.x + halfWidth, checkY)) {
                canMoveY = false;
            }
        }

        // 只应用可移动方向的速度
        if (canMoveX) {
            position_.x = newPosition.x;
        }
        if (canMoveY) {
            position_.y = newPosition.y;
        }
    } else {
        // 没有迷宫引用，直接移动
        position_ = newPosition;
    }
}

// ==================== 攻击 ====================

void Dave::attackTarget() {
    if (!target_ || !target_->isAlive()) return;

    // 检查冷却时间
    if (currentAttackCooldown_ > 0) return;

    // 检查距离
    if (distanceToTarget() > attackRange_) return;

    // 对目标造成伤害
    target_->takeDamage(attackDamage_);

    // 重置冷却时间
    currentAttackCooldown_ = attackCooldown_;
}

// ==================== 辅助函数 ====================

bool Dave::canSeeTarget() const {
    if (!target_) return false;
    return distanceToTarget() <= detectionRange_;
}

float Dave::distanceToTarget() const {
    if (!target_) return 999999.0f;
    return position_.distance(target_->getPosition());
}

void Dave::stun(float duration) {
    isStunned_ = true;
    stunTimer_ = duration;
    setState(DaveState::STUNNED);
    velocity_ = Vector2D(0, 0);
}

void Dave::takeDamage(float damage) {
    if (!alive_) return;

    // 眩晕期间无敌，不受伤害
    if (isStunned_) {
        return;
    }

    // 调用基类的伤害处理
    float oldHealth = health_;
    health_ -= damage;

    // 检查是否触发50HP眩晕（仅触发一次）
    if (!lowHpStunTriggered_ && oldHealth > 50.0f && health_ <= 50.0f) {
        lowHpStunTriggered_ = true;
        stun(20.0f);  // 眩晕20秒
        std::cout << "[Dave] HP dropped to 50, stunned for 20 seconds!" << std::endl;
    }

    // 死亡判定由update函数处理
}

void Dave::setState(DaveState newState) {
    if (state_ != newState) {
        state_ = newState;
    }
}

// ==================== 玩家控制（多人模式） ====================

void Dave::moveUp() {
    inputDirection_.y = -1;
    isMovingInput_ = true;
}

void Dave::moveDown() {
    inputDirection_.y = 1;
    isMovingInput_ = true;
}

void Dave::moveLeft() {
    inputDirection_.x = -1;
    isMovingInput_ = true;
}

void Dave::moveRight() {
    inputDirection_.x = 1;
    isMovingInput_ = true;
}

void Dave::stopMoving() {
    inputDirection_ = Vector2D(0, 0);
    isMovingInput_ = false;
}

void Dave::updatePlayerControl(float deltaTime) {
    // 玩家控制模式下的移动逻辑
    if (isMovingInput_ && (inputDirection_.x != 0 || inputDirection_.y != 0)) {
        // 设置为追踪状态（用于播放行走动画）
        setState(DaveState::CHASING);

        // 计算移动方向
        Vector2D moveDir = inputDirection_.normalized();

        // 设置方向枚举（用于动画）
        if (std::abs(moveDir.x) > std::abs(moveDir.y)) {
            direction_ = (moveDir.x > 0) ? Direction::RIGHT : Direction::LEFT;
        } else {
            direction_ = (moveDir.y > 0) ? Direction::DOWN : Direction::UP;
        }

        // 设置速度
        velocity_ = moveDir * speed_;

        // 计算新位置
        Vector2D newPosition = position_ + velocity_ * deltaTime;

        // 墙壁碰撞检测
        if (maze_) {
            float halfWidth = width_ / 2.0f;
            float halfHeight = height_ / 2.0f;

            bool canMoveX = true;
            bool canMoveY = true;

            Vector2D testPosX(newPosition.x, position_.y);
            Vector2D testPosY(position_.x, newPosition.y);

            // 检查X方向移动
            if (velocity_.x != 0) {
                float checkX = velocity_.x > 0 ? testPosX.x + halfWidth : testPosX.x - halfWidth;
                if (!maze_->isPassableAtPixel(checkX, position_.y - halfHeight) ||
                    !maze_->isPassableAtPixel(checkX, position_.y + halfHeight)) {
                    canMoveX = false;
                }
            }

            // 检查Y方向移动
            if (velocity_.y != 0) {
                float checkY = velocity_.y > 0 ? testPosY.y + halfHeight : testPosY.y - halfHeight;
                if (!maze_->isPassableAtPixel(position_.x - halfWidth, checkY) ||
                    !maze_->isPassableAtPixel(position_.x + halfWidth, checkY)) {
                    canMoveY = false;
                }
            }

            // 只应用可移动方向的速度
            if (canMoveX) {
                position_.x = newPosition.x;
            }
            if (canMoveY) {
                position_.y = newPosition.y;
            }
        } else {
            position_ = newPosition;
        }

        // 重置输入方向（需要持续按键）
        inputDirection_ = Vector2D(0, 0);
    } else {
        // 没有移动输入，待机状态
        setState(DaveState::IDLE);
        velocity_ = Vector2D(0, 0);
    }

    // 玩家模式也要重置移动输入标志
    isMovingInput_ = false;
}

// ==================== 阳光生成系统 ====================

void Dave::updateSunlightGeneration(float deltaTime) {
    // 更新阳光生成计时器
    sunlightTimer_ += deltaTime;

    // 每10秒生成50阳光
    if (sunlightTimer_ >= sunlightInterval_) {
        sunlight_ += sunlightPerInterval_;
        sunlightTimer_ = 0;
        std::cout << "戴夫获得 " << sunlightPerInterval_ << " 阳光！当前阳光: " << sunlight_ << std::endl;
    }
}

// ==================== 玩家种植功能 ====================

int Dave::getPlantCost(int plantType) const {
    // 植物类型: 0=豌豆射手, 1=双发射手, 2=樱桃炸弹, 3=坚果墙
    switch (plantType) {
        case 0: return 100;  // 豌豆射手
        case 1: return 200;  // 双发射手
        case 2: return 200;  // 樱桃炸弹
        case 3: return 50;   // 坚果墙
        default: return 999;
    }
}

float Dave::getPlantCooldown(int plantType) const {
    // 植物类型: 0=豌豆射手, 1=双发射手, 2=樱桃炸弹, 3=坚果墙
    switch (plantType) {
        case 0: return peaShooterCooldown_;   // 10秒
        case 1: return repeaterCooldown_;     // 20秒
        case 2: return cherryBombCooldown_;   // 30秒
        case 3: return wallNutCooldown_;      // 20秒
        default: return 999.0f;
    }
}

bool Dave::canPlant(int plantType) const {
    // 检查阳光是否足够
    int cost = getPlantCost(plantType);
    if (sunlight_ < cost) {
        return false;
    }

    // 检查冷却是否完成
    switch (plantType) {
        case 0: return currentPeaShooterCooldown_ <= 0;
        case 1: return currentRepeaterCooldown_ <= 0;
        case 2: return currentCherryBombCooldown_ <= 0;
        case 3: return currentWallNutCooldown_ <= 0;
        default: return false;
    }
}

void Dave::plantAtCurrentPosition(int plantType) {
    if (!isPlayerControlled_ || !entityManager_ || !maze_) {
        return;
    }

    // 检查是否可以种植
    if (!canPlant(plantType)) {
        std::cout << "无法种植：阳光不足或冷却中" << std::endl;
        return;
    }

    // 获取当前位置对应的格子
    int gridX, gridY;
    maze_->pixelToGrid(position_.x, position_.y, gridX, gridY);

    // 检查该位置是否已经有植物
    MazeCell& cell = maze_->getCell(gridX, gridY);
    if (cell.hasPlant) {
        std::cout << "该位置已有植物" << std::endl;
        return;
    }

    // 转换回像素坐标（格子中心）
    float pixelX, pixelY;
    maze_->gridToPixel(gridX, gridY, pixelX, pixelY);

    // 根据戴夫朝向确定植物发射方向
    Direction plantDirection = direction_;

    // 种植植物 - 只有成功种植才标记hasPlant
    int cost = getPlantCost(plantType);
    bool plantSuccess = false;
    switch (plantType) {
        case 0:  // 豌豆射手
            plantSuccess = plantPeaShooter(pixelX, pixelY, plantDirection);
            if (plantSuccess) currentPeaShooterCooldown_ = peaShooterCooldown_;
            break;
        case 1:  // 双发射手
            plantSuccess = plantDoublePeaShooter(pixelX, pixelY, plantDirection);
            if (plantSuccess) currentRepeaterCooldown_ = repeaterCooldown_;
            break;
        case 2:  // 樱桃炸弹
            plantSuccess = plantCherryBomb(pixelX, pixelY);
            if (plantSuccess) currentCherryBombCooldown_ = cherryBombCooldown_;
            break;
        case 3:  // 坚果墙
            plantSuccess = plantWallNut(pixelX, pixelY);
            if (plantSuccess) currentWallNutCooldown_ = wallNutCooldown_;
            break;
    }

    // 只有成功种植才标记格子
    if (plantSuccess) {
        cell.hasPlant = true;
        std::cout << "种植成功！花费 " << cost << " 阳光，剩余: " << sunlight_ << std::endl;
    }
}

void Dave::plantAtGridPosition(int plantType, int gridX, int gridY) {
    if (!isPlayerControlled_ || !entityManager_ || !maze_) {
        return;
    }

    // 检查是否可以种植（阳光和冷却）
    if (!canPlant(plantType)) {
        return;
    }

    // 检查格子是否在范围内
    if (!maze_->isInBounds(gridX, gridY)) {
        return;
    }

    // 检查是否是通道（不能在墙壁上种植）
    if (!maze_->isPassable(gridX, gridY)) {
        return;
    }

    // 检查该位置是否已经有植物
    MazeCell& cell = maze_->getCell(gridX, gridY);
    if (cell.hasPlant) {
        return;
    }

    // 转换为像素坐标（格子中心）
    float pixelX, pixelY;
    maze_->gridToPixel(gridX, gridY, pixelX, pixelY);

    // 根据戴夫朝向确定植物发射方向
    Direction plantDirection = direction_;

    // 种植植物 - 只有成功种植才标记hasPlant
    bool plantSuccess = false;
    switch (plantType) {
        case 0:  // 豌豆射手
            plantSuccess = plantPeaShooter(pixelX, pixelY, plantDirection);
            if (plantSuccess) currentPeaShooterCooldown_ = peaShooterCooldown_;
            break;
        case 1:  // 双发射手
            plantSuccess = plantDoublePeaShooter(pixelX, pixelY, plantDirection);
            if (plantSuccess) currentRepeaterCooldown_ = repeaterCooldown_;
            break;
        case 2:  // 樱桃炸弹
            plantSuccess = plantCherryBomb(pixelX, pixelY);
            if (plantSuccess) currentCherryBombCooldown_ = cherryBombCooldown_;
            break;
        case 3:  // 坚果墙
            plantSuccess = plantWallNut(pixelX, pixelY);
            if (plantSuccess) currentWallNutCooldown_ = wallNutCooldown_;
            break;
        default:
            return;
    }

    // 只有成功种植才标记格子
    if (plantSuccess) {
        cell.hasPlant = true;
    }
}

// ==================== 动画系统 ====================

void Dave::initializeAnimations() {
    // ===== 注册所有戴夫动画 =====
    // 当你提供动画帧图片后，取消注释并填写正确的路径

    // 待机动画（循环）
    AnimationClip* idleAnim = new AnimationClip("idle", true);
    idleAnim->addFrame("assets/images/dave/idle/frame_0.png", 0.2f);
    idleAnim->addFrame("assets/images/dave/idle/frame_1.png", 0.2f);
    idleAnim->addFrame("assets/images/dave/idle/frame_2.png", 0.2f);
    idleAnim->addFrame("assets/images/dave/idle/frame_3.png", 0.2f);
    animationController_.registerAnimation(idleAnim);

    // 行走/追踪动画（循环）
    AnimationClip* walkAnim = new AnimationClip("walk", true);
    walkAnim->addFrame("assets/images/dave/walk/frame_0.png", 0.12f);
    walkAnim->addFrame("assets/images/dave/walk/frame_1.png", 0.12f);
    walkAnim->addFrame("assets/images/dave/walk/frame_2.png", 0.12f);
    walkAnim->addFrame("assets/images/dave/walk/frame_3.png", 0.12f);
    walkAnim->addFrame("assets/images/dave/walk/frame_4.png", 0.12f);
    walkAnim->addFrame("assets/images/dave/walk/frame_5.png", 0.12f);
    animationController_.registerAnimation(walkAnim);

    // 攻击动画（不循环）
    AnimationClip* attackAnim = new AnimationClip("attack", false);
    attackAnim->addFrame("assets/images/dave/attack/frame_0.png", 0.1f);
    attackAnim->addFrame("assets/images/dave/attack/frame_1.png", 0.1f);
    attackAnim->addFrame("assets/images/dave/attack/frame_2.png", 0.15f);
    attackAnim->addFrame("assets/images/dave/attack/frame_3.png", 0.1f);
    attackAnim->addFrame("assets/images/dave/attack/frame_4.png", 0.1f);
    animationController_.registerAnimation(attackAnim);

    // 眩晕动画（循环）
    AnimationClip* stunnedAnim = new AnimationClip("stunned", true);
    stunnedAnim->addFrame("assets/images/dave/stunned/frame_0.png", 0.15f);
    stunnedAnim->addFrame("assets/images/dave/stunned/frame_1.png", 0.15f);
    stunnedAnim->addFrame("assets/images/dave/stunned/frame_2.png", 0.15f);
    animationController_.registerAnimation(stunnedAnim);

    // 默认播放待机动画
    animationController_.play("idle");
}

void Dave::updateAnimation() {
    // 根据当前状态选择播放对应的动画

    switch (state_) {
        case DaveState::IDLE:
            if (!animationController_.isPlaying("idle")) {
                animationController_.play("idle");
            }
            break;

        case DaveState::CHASING:
            if (!animationController_.isPlaying("walk")) {
                animationController_.play("walk");
            }
            break;

        case DaveState::ATTACKING:
            if (!animationController_.isPlaying("attack")) {
                animationController_.play("attack", true);
            }
            // 攻击动画播放完毕后回到追踪或待机
            if (animationController_.isCurrentAnimationFinished()) {
                if (target_ && target_->isAlive() && distanceToTarget() > attackRange_) {
                    setState(DaveState::CHASING);
                } else {
                    setState(DaveState::IDLE);
                }
            }
            break;

        case DaveState::STUNNED:
            if (!animationController_.isPlaying("stunned")) {
                animationController_.play("stunned");
            }
            break;
    }
}

// ==================== 序列化 ====================

std::string Dave::toJson() const {
    std::stringstream ss;
    ss << "{"
       << "\"id\":" << id_ << ","
       << "\"type\":\"dave\","
       << "\"x\":" << position_.x << ","
       << "\"y\":" << position_.y << ","
       << "\"vx\":" << velocity_.x << ","
       << "\"vy\":" << velocity_.y << ","
       << "\"health\":" << health_ << ","
       << "\"maxHealth\":" << maxHealth_ << ","
       << "\"state\":\"" << static_cast<int>(state_) << "\","
       << "\"direction\":\"" << static_cast<int>(direction_) << "\","
       << "\"isStunned\":" << (isStunned_ ? "true" : "false") << ","
       << "\"alive\":" << (alive_ ? "true" : "false") << ",";

    // 添加目标信息
    if (target_ && target_->isAlive()) {
        ss << "\"targetId\":" << target_->getId() << ","
           << "\"targetX\":" << target_->getPosition().x << ","
           << "\"targetY\":" << target_->getPosition().y << ",";
    } else {
        ss << "\"targetId\":null,";
    }

    // 添加路径信息
    ss << "\"hasPath\":" << (hasPath() ? "true" : "false") << ",";

    // 添加阳光信息（多人模式）
    ss << "\"sunlight\":" << sunlight_ << ","
       << "\"isPlayerControlled\":" << (isPlayerControlled_ ? "true" : "false") << ","

       // 添加植物冷却时间信息（最大冷却）
       << "\"peaShooterCooldown\":" << peaShooterCooldown_ << ","
       << "\"repeaterCooldown\":" << repeaterCooldown_ << ","
       << "\"cherryBombCooldown\":" << cherryBombCooldown_ << ","
       << "\"wallNutCooldown\":" << wallNutCooldown_ << ","

       // 添加当前冷却时间（用于前端显示冷却进度）
       << "\"currentPeaShooterCooldown\":" << currentPeaShooterCooldown_ << ","
       << "\"currentRepeaterCooldown\":" << currentRepeaterCooldown_ << ","
       << "\"currentCherryBombCooldown\":" << currentCherryBombCooldown_ << ","
       << "\"currentWallNutCooldown\":" << currentWallNutCooldown_;

    ss << "}";
    return ss.str();
}

// ==================== 种植植物功能 ====================

bool Dave::plantPeaShooter(float x, float y, Direction shootDirection) {
    const int COST = 100;

    // 玩家模式不检查全局冷却（使用单独冷却）
    // AI模式检查全局冷却
    if (!isPlayerControlled_ && currentPlantCooldown_ > 0) {
        return false;
    }

    // 检查阳光是否足够
    if (!canAffordPlant(COST)) {
        return false;
    }

    if (!entityManager_) {
        return false;
    }

    // 创建豌豆射手
    PeaShooter* peaShooter = new PeaShooter(x, y, shootDirection);
    peaShooter->setEntityManager(entityManager_);
    peaShooter->setMaze(maze_);

    // 添加到实体管理器
    entityManager_->addPlant(peaShooter);

    // 扣除阳光
    sunlight_ -= COST;

    // 重置冷却（AI模式全局冷却，玩家模式在调用处设置单独冷却）
    if (!isPlayerControlled_) {
        currentPlantCooldown_ = plantCooldown_;
    }

    // 播放种植动画（如果有）
    setState(DaveState::PLANTING);

    return true;
}


bool Dave::plantDoublePeaShooter(float x, float y, Direction shootDirection) {
    const int COST = 200;

    // 玩家模式不检查全局冷却（使用单独冷却）
    if (!isPlayerControlled_ && currentPlantCooldown_ > 0) {
        return false;
    }

    // 检查阳光是否足够
    if (!canAffordPlant(COST)) {
        return false;
    }

    if (!entityManager_) {
        return false;
    }

    // 创建双发射手
    DoublePeaShooter* doublePeaShooter = new DoublePeaShooter(x, y, shootDirection);
    doublePeaShooter->setEntityManager(entityManager_);
    doublePeaShooter->setMaze(maze_);

    // 添加到实体管理器
    entityManager_->addPlant(doublePeaShooter);

    // 扣除阳光
    sunlight_ -= COST;

    // 重置冷却（AI模式）
    if (!isPlayerControlled_) {
        currentPlantCooldown_ = plantCooldown_;
    }

    // 播放种植动画（如果有）
    setState(DaveState::PLANTING);

    return true;
}
bool Dave::plantCherryBomb(float x, float y) {
    const int COST = 200;

    // 玩家模式不检查全局冷却（使用单独冷却）
    if (!isPlayerControlled_ && currentPlantCooldown_ > 0) {
        return false;
    }

    // 检查阳光是否足够
    if (!canAffordPlant(COST)) {
        return false;
    }

    if (!entityManager_) {
        return false;
    }

    // 创建樱桃炸弹
    CherryBomb* cherryBomb = new CherryBomb(x, y);
    cherryBomb->setEntityManager(entityManager_);
    cherryBomb->setMaze(maze_);

    // 添加到实体管理器
    entityManager_->addPlant(cherryBomb);

    // 扣除阳光
    sunlight_ -= COST;

    // 重置冷却（AI模式）
    if (!isPlayerControlled_) {
        currentPlantCooldown_ = plantCooldown_;
    }

    // 播放种植动画（如果有）
    setState(DaveState::PLANTING);

    return true;
}

bool Dave::plantWallNut(float x, float y) {
    const int COST = 50;

    // 玩家模式不检查全局冷却（使用单独冷却）
    if (!isPlayerControlled_ && currentPlantCooldown_ > 0) {
        return false;
    }

    // 检查阳光是否足够
    if (!canAffordPlant(COST)) {
        return false;
    }

    if (!entityManager_) {
        return false;
    }

    // 创建坚果墙
    WallNut* wallNut = new WallNut(x, y);
    wallNut->setEntityManager(entityManager_);
    wallNut->setMaze(maze_);

    // 添加到实体管理器
    entityManager_->addPlant(wallNut);

    // 扣除阳光
    sunlight_ -= COST;

    // 重置冷却（AI模式）
    if (!isPlayerControlled_) {
        currentPlantCooldown_ = plantCooldown_;
    }

    // 播放种植动画（如果有）
    setState(DaveState::PLANTING);

    return true;
}

// ==================== 植物种植AI ====================

void Dave::updatePlantingAI(float deltaTime) {
    // 安全检查：如果是玩家控制模式，完全禁止AI种植
    if (isPlayerControlled_) {
        return;
    }

    // 如果冷却未完成，不尝试种植
    if (currentPlantCooldown_ > 0) {
        return;
    }

    // 如果没有目标或迷宫，不种植
    if (!target_ || !target_->isAlive() || !maze_) {
        return;
    }

    // 更新僵尸卡住检测
    Vector2D zombiePos = target_->getPosition();
    float zombieMoveDist = (zombiePos - lastZombiePosition_).length();
    if (zombieMoveDist < 5.0f) {
        zombieStuckTimer_ += deltaTime;
    } else {
        zombieStuckTimer_ = 0;
        hasPlacedWalnut_ = false;  // 僵尸移动了，重置坚果标记
    }
    lastZombiePosition_ = zombiePos;

    // 获取戴夫和僵尸的格子位置
    int daveGridX, daveGridY;
    maze_->pixelToGrid(position_.x, position_.y, daveGridX, daveGridY);

    int zombieGridX, zombieGridY;
    maze_->pixelToGrid(zombiePos.x, zombiePos.y, zombieGridX, zombieGridY);

    // 计算与僵尸的格子距离
    int gridDistX = std::abs(daveGridX - zombieGridX);
    int gridDistY = std::abs(daveGridY - zombieGridY);
    float distToZombie = position_.distance(zombiePos);

    // ==================== 策略1：樱桃炸弹（僵尸在3x3区域内） ====================
    if (gridDistX <= 1 && gridDistY <= 1 && canAffordPlant(200)) {
        // 在僵尸位置种樱桃炸弹
        float pixelX, pixelY;
        maze_->gridToPixel(zombieGridX, zombieGridY, pixelX, pixelY);

        MazeCell& cell = maze_->getCell(zombieGridX, zombieGridY);
        if (!cell.hasPlant && maze_->isPassable(zombieGridX, zombieGridY)) {
            if (plantCherryBomb(pixelX, pixelY)) {
                cell.hasPlant = true;
                return;
            }
        }
    }

    // ==================== 策略2：僵尸被坚果卡住，在远离僵尸一侧放射手 ====================
    if (zombieStuckTimer_ > 1.0f) {  // 僵尸卡住超过1秒
        // 寻找僵尸远离的一侧放置射手
        int dx = daveGridX - zombieGridX;  // 戴夫相对僵尸的方向
        int dy = daveGridY - zombieGridY;

        // 在僵尸的远侧（戴夫方向）放置射手
        int shooterGridX = zombieGridX + (dx > 0 ? 2 : (dx < 0 ? -2 : 0));
        int shooterGridY = zombieGridY + (dy > 0 ? 2 : (dy < 0 ? -2 : 0));

        // 确保在迷宫范围内
        shooterGridX = std::max(0, std::min(maze_->getGridWidth() - 1, shooterGridX));
        shooterGridY = std::max(0, std::min(maze_->getGridHeight() - 1, shooterGridY));

        if (maze_->isPassable(shooterGridX, shooterGridY)) {
            MazeCell& cell = maze_->getCell(shooterGridX, shooterGridY);
            if (!cell.hasPlant) {
                float pixelX, pixelY;
                maze_->gridToPixel(shooterGridX, shooterGridY, pixelX, pixelY);

                // 朝向僵尸的方向
                Direction shootDir;
                if (std::abs(dx) > std::abs(dy)) {
                    shootDir = (dx > 0) ? Direction::LEFT : Direction::RIGHT;
                } else {
                    shootDir = (dy > 0) ? Direction::UP : Direction::DOWN;
                }

                // 优先双发射手
                if (canAffordPlant(200)) {
                    if (plantDoublePeaShooter(pixelX, pixelY, shootDir)) {
                        cell.hasPlant = true;
                        return;
                    }
                } else if (canAffordPlant(100)) {
                    if (plantPeaShooter(pixelX, pixelY, shootDir)) {
                        cell.hasPlant = true;
                        return;
                    }
                }
            }
        }
    }

    // ==================== 策略3：僵尸追踪戴夫时，在中间放坚果 ====================
    if (distToZombie < 300.0f && distToZombie > 100.0f && !hasPlacedWalnut_ && canAffordPlant(50)) {
        // 找到戴夫和僵尸之间的位置
        int midGridX, midGridY;
        if (findPositionBetween(midGridX, midGridY, position_, zombiePos)) {
            MazeCell& cell = maze_->getCell(midGridX, midGridY);
            if (!cell.hasPlant && isCorridorCell(midGridX, midGridY)) {
                float pixelX, pixelY;
                maze_->gridToPixel(midGridX, midGridY, pixelX, pixelY);

                if (plantWallNut(pixelX, pixelY)) {
                    cell.hasPlant = true;
                    hasPlacedWalnut_ = true;
                    return;
                }
            }
        }
    }

    // ==================== 策略4：戴夫靠近僵尸，先放坚果再放射手 ====================
    if (distToZombie < 150.0f && !hasPlacedWalnut_ && canAffordPlant(50)) {
        // 在戴夫和僵尸之间放坚果
        int dx = zombieGridX - daveGridX;
        int dy = zombieGridY - daveGridY;

        // 在戴夫朝向僵尸方向的相邻格子放坚果
        int nutGridX = daveGridX + (dx > 0 ? 1 : (dx < 0 ? -1 : 0));
        int nutGridY = daveGridY + (dy > 0 ? 1 : (dy < 0 ? -1 : 0));

        if (maze_->isPassable(nutGridX, nutGridY)) {
            MazeCell& cell = maze_->getCell(nutGridX, nutGridY);
            if (!cell.hasPlant) {
                float pixelX, pixelY;
                maze_->gridToPixel(nutGridX, nutGridY, pixelX, pixelY);

                if (plantWallNut(pixelX, pixelY)) {
                    cell.hasPlant = true;
                    hasPlacedWalnut_ = true;
                    return;
                }
            }
        }
    }

    // ==================== 策略5：远距离时种植射手 ====================
    if (distToZombie >= 200.0f) {
        int plantGridX, plantGridY;
        Direction plantDirection;

        if (findOptimalPlantLocation(plantGridX, plantGridY, plantDirection)) {
            MazeCell& cell = maze_->getCell(plantGridX, plantGridY);
            if (!cell.hasPlant) {
                float pixelX, pixelY;
                maze_->gridToPixel(plantGridX, plantGridY, pixelX, pixelY);

                if (canAffordPlant(200)) {
                    if (plantDoublePeaShooter(pixelX, pixelY, plantDirection)) {
                        cell.hasPlant = true;
                        return;
                    }
                } else if (canAffordPlant(100)) {
                    if (plantPeaShooter(pixelX, pixelY, plantDirection)) {
                        cell.hasPlant = true;
                        return;
                    }
                }
            }
        }
    }
}

bool Dave::findOptimalPlantLocation(int& gridX, int& gridY, Direction& plantDirection) {
    if (!maze_ || !target_) return false;

    // 获取僵尸的格子位置
    int zombieGridX, zombieGridY;
    maze_->pixelToGrid(target_->getPosition().x, target_->getPosition().y, zombieGridX, zombieGridY);

    // 获取戴夫的格子位置
    int daveGridX, daveGridY;
    maze_->pixelToGrid(position_.x, position_.y, daveGridX, daveGridY);

    // 获取出口的格子位置
    int exitGridX, exitGridY;
    maze_->getExitGrid(exitGridX, exitGridY);

    int bestScore = -1;
    int bestGridX = -1, bestGridY = -1;
    Direction bestDirection = Direction::RIGHT;

    // 搜索范围：僵尸到出口之间的区域
    int searchMinX = std::min(zombieGridX, exitGridX);
    int searchMaxX = std::max(zombieGridX, exitGridX);
    int searchMinY = std::min(zombieGridY, exitGridY);
    int searchMaxY = std::max(zombieGridY, exitGridY);

    // 扩大搜索范围
    searchMinX = std::max(0, searchMinX - 3);
    searchMaxX = std::min(maze_->getGridWidth() - 1, searchMaxX + 3);
    searchMinY = std::max(0, searchMinY - 3);
    searchMaxY = std::min(maze_->getGridHeight() - 1, searchMaxY + 3);

    for (int y = searchMinY; y <= searchMaxY; y++) {
        for (int x = searchMinX; x <= searchMaxX; x++) {
            // 检查是否是可通过的格子
            if (!maze_->isPassable(x, y)) continue;

            // 检查是否已经有植物
            const MazeCell& cell = maze_->getCell(x, y);
            if (cell.hasPlant) continue;

            // 计算该位置的得分
            int score = calculatePlantScore(x, y);

            if (score > bestScore) {
                bestScore = score;
                bestGridX = x;
                bestGridY = y;

                // 确定植物朝向 - 根据通道类型决定
                // 检查左右和上下是否有墙
                bool leftBlocked = !maze_->isPassable(x - 1, y);
                bool rightBlocked = !maze_->isPassable(x + 1, y);
                bool upBlocked = !maze_->isPassable(x, y - 1);
                bool downBlocked = !maze_->isPassable(x, y + 1);

                // 判断通道类型
                bool isVerticalCorridor = leftBlocked && rightBlocked && !upBlocked && !downBlocked;
                bool isHorizontalCorridor = !leftBlocked && !rightBlocked && upBlocked && downBlocked;

                if (isVerticalCorridor) {
                    // 竖向通道（左右有墙）- 朝向僵尸所在的上下方向
                    float dy = target_->getPosition().y - maze_->gridToPixel(x, y).y;
                    bestDirection = (dy > 0) ? Direction::DOWN : Direction::UP;
                } else if (isHorizontalCorridor) {
                    // 横向通道（上下有墙）- 朝向僵尸所在的左右方向
                    float dx = target_->getPosition().x - maze_->gridToPixel(x, y).x;
                    bestDirection = (dx > 0) ? Direction::RIGHT : Direction::LEFT;
                } else {
                    // 其他情况（交叉口等）- 优先朝向僵尸
                    float dx = target_->getPosition().x - maze_->gridToPixel(x, y).x;
                    float dy = target_->getPosition().y - maze_->gridToPixel(x, y).y;
                    if (std::abs(dx) > std::abs(dy)) {
                        bestDirection = (dx > 0) ? Direction::RIGHT : Direction::LEFT;
                    } else {
                        bestDirection = (dy > 0) ? Direction::DOWN : Direction::UP;
                    }
                }
            }
        }
    }

    if (bestScore > 0) {
        gridX = bestGridX;
        gridY = bestGridY;
        plantDirection = bestDirection;
        return true;
    }

    return false;
}

bool Dave::isCorridorCell(int gridX, int gridY) const {
    if (!maze_ || !maze_->isPassable(gridX, gridY)) return false;

    // 计算相邻的可通过格子数量
    int passableNeighbors = 0;
    bool leftPassable = maze_->isPassable(gridX - 1, gridY);
    bool rightPassable = maze_->isPassable(gridX + 1, gridY);
    bool upPassable = maze_->isPassable(gridX, gridY - 1);
    bool downPassable = maze_->isPassable(gridX, gridY + 1);

    if (leftPassable) passableNeighbors++;
    if (rightPassable) passableNeighbors++;
    if (upPassable) passableNeighbors++;
    if (downPassable) passableNeighbors++;

    // 只有2个相邻通道的是走廊（最佳种植位置）
    // 或者是L型拐角（也是不错的位置）
    return passableNeighbors == 2;
}

bool Dave::isCellOnZombiePath(int gridX, int gridY) const {
    if (!target_ || !maze_) return false;

    // 获取僵尸和出口的位置
    int zombieGridX, zombieGridY;
    maze_->pixelToGrid(target_->getPosition().x, target_->getPosition().y, zombieGridX, zombieGridY);

    int exitGridX, exitGridY;
    maze_->getExitGrid(exitGridX, exitGridY);

    // 简单判断：该格子是否在僵尸和出口之间的矩形区域内
    int minX = std::min(zombieGridX, exitGridX);
    int maxX = std::max(zombieGridX, exitGridX);
    int minY = std::min(zombieGridY, exitGridY);
    int maxY = std::max(zombieGridY, exitGridY);

    return gridX >= minX && gridX <= maxX && gridY >= minY && gridY <= maxY;
}

int Dave::calculatePlantScore(int gridX, int gridY) const {
    if (!maze_ || !target_) return 0;

    int score = 0;

    // 基础分：是走廊格子
    if (isCorridorCell(gridX, gridY)) {
        score += 50;
    }

    // 加分：在僵尸到出口的路径上
    if (isCellOnZombiePath(gridX, gridY)) {
        score += 30;
    }

    // 计算与僵尸的距离
    Vector2D cellPos = maze_->gridToPixel(gridX, gridY);
    float distToZombie = cellPos.distance(target_->getPosition());

    // 加分：距离僵尸适中（100-300像素）
    if (distToZombie >= 100.0f && distToZombie <= 300.0f) {
        score += 40;
    } else if (distToZombie > 300.0f && distToZombie <= 500.0f) {
        score += 20;
    }

    // 减分：太近或太远
    if (distToZombie < 50.0f) {
        score -= 30;  // 太近，可能来不及种
    }
    if (distToZombie > 600.0f) {
        score -= 20;  // 太远，效果不好
    }

    // 加分：在僵尸前进方向上
    Vector2D zombieVel = target_->getVelocity();
    if (zombieVel.length() > 0.1f) {
        Vector2D toCell = cellPos - target_->getPosition();
        float dot = zombieVel.normalized().dot(toCell.normalized());
        if (dot > 0.5f) {
            score += 25;  // 在僵尸前进方向上
        }
    }

    return std::max(0, score);
}

// ==================== 新增辅助函数 ====================

bool Dave::findPositionBetween(int& gridX, int& gridY, const Vector2D& pos1, const Vector2D& pos2) const {
    if (!maze_) return false;

    // 获取两个位置的格子坐标
    int grid1X, grid1Y, grid2X, grid2Y;
    maze_->pixelToGrid(pos1.x, pos1.y, grid1X, grid1Y);
    maze_->pixelToGrid(pos2.x, pos2.y, grid2X, grid2Y);

    // 计算中点
    int midX = (grid1X + grid2X) / 2;
    int midY = (grid1Y + grid2Y) / 2;

    // 尝试找到中间区域的可通过格子
    // 搜索半径
    int searchRadius = 2;

    for (int r = 0; r <= searchRadius; r++) {
        for (int dy = -r; dy <= r; dy++) {
            for (int dx = -r; dx <= r; dx++) {
                int testX = midX + dx;
                int testY = midY + dy;

                // 检查是否在迷宫范围内
                if (testX < 0 || testX >= maze_->getGridWidth() ||
                    testY < 0 || testY >= maze_->getGridHeight()) {
                    continue;
                }

                // 检查是否可通过
                if (!maze_->isPassable(testX, testY)) {
                    continue;
                }

                // 检查是否已有植物
                const MazeCell& cell = maze_->getCell(testX, testY);
                if (cell.hasPlant) {
                    continue;
                }

                // 确保不是在pos1或pos2的位置
                if ((testX == grid1X && testY == grid1Y) ||
                    (testX == grid2X && testY == grid2Y)) {
                    continue;
                }

                // 找到了合适的位置
                gridX = testX;
                gridY = testY;
                return true;
            }
        }
    }

    return false;
}

bool Dave::isZombieStuckByWalnut() const {
    // 如果僵尸没有移动超过一定时间，认为被卡住了
    return zombieStuckTimer_ > 1.0f;
}

bool Dave::hasWalnutOnZombiePath() const {
    if (!target_ || !maze_ || !entityManager_) return false;

    // 获取僵尸和出口的位置
    int zombieGridX, zombieGridY;
    maze_->pixelToGrid(target_->getPosition().x, target_->getPosition().y, zombieGridX, zombieGridY);

    int exitGridX, exitGridY;
    maze_->getExitGrid(exitGridX, exitGridY);

    // 检查僵尸到出口路径上是否有坚果
    // 简化检查：检查僵尸附近几格内是否有坚果
    int searchRadius = 5;
    for (int dy = -searchRadius; dy <= searchRadius; dy++) {
        for (int dx = -searchRadius; dx <= searchRadius; dx++) {
            int checkX = zombieGridX + dx;
            int checkY = zombieGridY + dy;

            if (checkX < 0 || checkX >= maze_->getGridWidth() ||
                checkY < 0 || checkY >= maze_->getGridHeight()) {
                continue;
            }

            const MazeCell& cell = maze_->getCell(checkX, checkY);
            if (cell.hasPlant) {
                // 这里可以进一步检查是否是坚果
                // 但简化起见，有植物就认为可能有坚果
                return true;
            }
        }
    }

    return false;
}
