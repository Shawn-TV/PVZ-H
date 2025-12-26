/**
 * @file Plant.cpp
 * @brief 植物基类实现
 */

#include "../../include/entities/Plant.h"
#include "../../include/entities/Zombie.h"
#include "../../include/entities/EntityManager.h"
#include "../../include/maze/Maze.h"
#include <sstream>
#include <cmath>

Plant::Plant(float x, float y, PlantType plantType)
    : Entity(x, y, EntityType::PLANT),
      plantType_(plantType),
      attackCooldown_(2.0f),
      currentAttackCooldown_(0),
      attackRange_(100.0f),
      attackDamage_(10.0f),
      attackDirection_(Direction::RIGHT),
      entityManager_(nullptr),
      maze_(nullptr) {

    // 植物不移动
    speed_ = 0;

    // 设置默认尺寸（与sprite大小80x80一致）
    setSize(80, 80);
}

Plant::~Plant() {
    // 清除迷宫中的植物标记
    if (maze_) {
        int gridX, gridY;
        maze_->pixelToGrid(position_.x, position_.y, gridX, gridY);
        if (maze_->isInBounds(gridX, gridY)) {
            maze_->getCell(gridX, gridY).hasPlant = false;
        }
    }
}

void Plant::update(float deltaTime) {
    if (!alive_) return;

    // 更新攻击冷却
    if (currentAttackCooldown_ > 0) {
        currentAttackCooldown_ -= deltaTime;
    }

    // 如果冷却完成且有目标，执行攻击
    if (currentAttackCooldown_ <= 0 && hasTargetInRange()) {
        performAttack();
        currentAttackCooldown_ = attackCooldown_;
    }

    // 更新动画
    updateAnimation();
    animationController_.update(deltaTime);
}

void Plant::onCollision(Entity* other) {
    // 植物通常不响应碰撞（除了被攻击）
    // 子类可以重写此方法实现特殊逻辑
}

bool Plant::hasTargetInRange() const {
    if (!entityManager_) return false;

    // 获取范围内的所有实体
    auto entitiesInRange = entityManager_->findEntitiesInRange(position_, attackRange_);

    // 射击方向上的"线"容差（僵尸需要在同一行/列才会触发射击）
    // 参考原版PVZ：豌豆射手只检测同一行的僵尸
    // 容差设为格子大小的一半(75)，让检测更准确
    const float lineTolerance = 100.0f;  // 增大容差以确保检测到同行僵尸

    // 根据攻击方向检测僵尸
    // Direction枚举: NONE=0, UP=1, DOWN=2, LEFT=3, RIGHT=4
    for (auto* entity : entitiesInRange) {
        if (entity->getType() == EntityType::ZOMBIE && entity->isAlive()) {
            Zombie* zombie = dynamic_cast<Zombie*>(entity);
            if (!zombie) continue;

            Vector2D zombiePos = zombie->getPosition();
            Vector2D toTarget = zombiePos - position_;

            // 根据攻击方向判断僵尸是否在发射口前方
            bool inLineOfFire = false;
            switch (attackDirection_) {
                case Direction::RIGHT:
                    // 僵尸在右边，同一行
                    inLineOfFire = (toTarget.x > 0 && std::abs(toTarget.y) <= lineTolerance);
                    break;
                case Direction::LEFT:
                    // 僵尸在左边，同一行
                    inLineOfFire = (toTarget.x < 0 && std::abs(toTarget.y) <= lineTolerance);
                    break;
                case Direction::UP:
                    // 僵尸在上方，同一列
                    inLineOfFire = (toTarget.y < 0 && std::abs(toTarget.x) <= lineTolerance);
                    break;
                case Direction::DOWN:
                    // 僵尸在下方，同一列
                    inLineOfFire = (toTarget.y > 0 && std::abs(toTarget.x) <= lineTolerance);
                    break;
                default:
                    // 默认向右
                    inLineOfFire = (toTarget.x > 0 && std::abs(toTarget.y) <= lineTolerance);
                    break;
            }

            if (inLineOfFire) {
                // 检查植物和僵尸之间是否有墙阻挡
                if (maze_) {
                    bool hasWall = false;
                    float stepSize = 50.0f;  // 检查步长
                    float distance = toTarget.length();
                    int steps = static_cast<int>(distance / stepSize);

                    for (int i = 1; i <= steps; i++) {
                        float t = static_cast<float>(i) / (steps + 1);
                        float checkX = position_.x + toTarget.x * t;
                        float checkY = position_.y + toTarget.y * t;

                        if (!maze_->isPassableAtPixel(checkX, checkY)) {
                            hasWall = true;
                            break;
                        }
                    }

                    if (!hasWall) {
                        return true;  // 找到无遮挡的僵尸目标
                    }
                } else {
                    return true;  // 没有迷宫引用，默认可以攻击
                }
            }
        }
    }

    return false;
}

std::string Plant::toJson() const {
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
       << "\"alive\":" << (alive_ ? "true" : "false")
       << "}";
    return ss.str();
}
