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
      detectionRange_(500.0f),
      isStunned_(false),
      stunTimer_(0),
      lastTargetPosition_(0, 0),
      entityManager_(nullptr),
      plantCooldown_(5.0f),         // 种植冷却5秒
      currentPlantCooldown_(0) {

    // 设置戴夫属性
    speed_ = Config::DAVE_SPEED;
    maxHealth_ = 999.0f;  // 戴夫无法被击杀
    health_ = maxHealth_;
    setSize(32, 32);

    // 初始化动画
    initializeAnimations();
}

Dave::~Dave() {
}

void Dave::update(float deltaTime) {
    if (!alive_) return;

    // 更新眩晕状态
    if (isStunned_) {
        stunTimer_ -= deltaTime;
        if (stunTimer_ <= 0) {
            isStunned_ = false;
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

    // 更新种植冷却
    if (currentPlantCooldown_ > 0) {
        currentPlantCooldown_ -= deltaTime;
    }

    // 更新AI逻辑
    updateAI(deltaTime);

    // 更新动画
    updateAnimation();
    animationController_.update(deltaTime);
}

void Dave::onCollision(Entity* other) {
    if (!other || !alive_) return;

    // 碰撞到僵尸，尝试攻击
    if (other->getType() == EntityType::ZOMBIE) {
        Zombie* zombie = dynamic_cast<Zombie*>(other);
        if (zombie && zombie->isAlive()) {
            attackTarget();
        }
    }
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
    // 如果没有目标或目标已死亡，进入待机状态
    if (!target_ || !target_->isAlive()) {
        setState(DaveState::IDLE);
        clearPath();
        velocity_ = Vector2D(0, 0);
        return;
    }

    // 计算与目标的距离
    float dist = distanceToTarget();

    // 如果目标在检测范围外，不追踪
    if (dist > detectionRange_) {
        setState(DaveState::IDLE);
        clearPath();
        velocity_ = Vector2D(0, 0);
        return;
    }

    // 如果目标在攻击范围内，进行攻击
    if (dist <= attackRange_) {
        setState(DaveState::ATTACKING);
        velocity_ = Vector2D(0, 0);
        attackTarget();
        return;
    }

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

    // 应用速度
    applyVelocity(deltaTime);
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

void Dave::setState(DaveState newState) {
    if (state_ != newState) {
        state_ = newState;
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
       << "\"state\":\"" << static_cast<int>(state_) << "\","
       << "\"direction\":\"" << static_cast<int>(direction_) << "\","
       << "\"isStunned\":" << (isStunned_ ? "true" : "false") << ",";

    // 添加目标信息
    if (target_ && target_->isAlive()) {
        ss << "\"targetId\":" << target_->getId() << ","
           << "\"targetX\":" << target_->getPosition().x << ","
           << "\"targetY\":" << target_->getPosition().y << ",";
    } else {
        ss << "\"targetId\":null,";
    }

    // 添加路径信息（如果需要在前端显示）
    ss << "\"hasPath\":" << (hasPath() ? "true" : "false");

    ss << "}";
    return ss.str();
}

// ==================== 种植植物功能 ====================

void Dave::plantPeaShooter(float x, float y, Direction shootDirection) {
    // 检查是否可以种植（冷却完成）
    if (currentPlantCooldown_ > 0) {
        return;
    }

    if (!entityManager_) {
        return;
    }

    // 创建豌豆射手
    PeaShooter* peaShooter = new PeaShooter(x, y, shootDirection);
    peaShooter->setEntityManager(entityManager_);

    // 添加到实体管理器
    entityManager_->addPlant(peaShooter);

    // 重置冷却
    currentPlantCooldown_ = plantCooldown_;

    // 播放种植动画（如果有）
    setState(DaveState::PLANTING);
}


void Dave::plantDoublePeaShooter(float x, float y, bool horizontal) {
    // 检查是否可以种植（冷却完成）
    if (currentPlantCooldown_ > 0) {
        return;
    }

    if (!entityManager_) {
        return;
    }

    // 创建双发射手
    DoublePeaShooter* doublePeaShooter = new DoublePeaShooter(x, y, horizontal);
    doublePeaShooter->setEntityManager(entityManager_);

    // 添加到实体管理器
    entityManager_->addPlant(doublePeaShooter);

    // 重置冷却
    currentPlantCooldown_ = plantCooldown_;

    // 播放种植动画（如果有）
    setState(DaveState::PLANTING);
}
void Dave::plantCherryBomb(float x, float y) {
    // 检查是否可以种植（冷却完成）
    if (currentPlantCooldown_ > 0) {
        return;
    }

    if (!entityManager_) {
        return;
    }

    // 创建樱桃炸弹
    CherryBomb* cherryBomb = new CherryBomb(x, y);
    cherryBomb->setEntityManager(entityManager_);

    // 添加到实体管理器
    entityManager_->addPlant(cherryBomb);

    // 重置冷却
    currentPlantCooldown_ = plantCooldown_;

    // 播放种植动画（如果有）
    setState(DaveState::PLANTING);
}

void Dave::plantWallNut(float x, float y) {
    // 检查是否可以种植（冷却完成）
    if (currentPlantCooldown_ > 0) {
        return;
    }

    if (!entityManager_) {
        return;
    }

    // 创建坚果墙
    WallNut* wallNut = new WallNut(x, y);
    wallNut->setEntityManager(entityManager_);

    // 添加到实体管理器
    entityManager_->addPlant(wallNut);

    // 重置冷却
    currentPlantCooldown_ = plantCooldown_;

    // 播放种植动画（如果有）
    setState(DaveState::PLANTING);
}
