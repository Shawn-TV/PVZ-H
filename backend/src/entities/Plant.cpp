/**
 * @file Plant.cpp
 * @brief 植物基类实现
 */

#include "../../include/entities/Plant.h"
#include "../../include/entities/Zombie.h"
#include "../../include/entities/EntityManager.h"
#include <sstream>

Plant::Plant(float x, float y, PlantType plantType)
    : Entity(x, y, EntityType::PLANT),
      plantType_(plantType),
      attackCooldown_(2.0f),
      currentAttackCooldown_(0),
      attackRange_(100.0f),
      attackDamage_(10.0f),
      attackDirection_(Direction::RIGHT),
      entityManager_(nullptr) {

    // 植物不移动
    speed_ = 0;

    // 设置默认尺寸
    setSize(32, 32);
}

Plant::~Plant() {
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

    // 检查是否有僵尸在攻击方向上
    for (auto* entity : entitiesInRange) {
        if (entity->getType() == EntityType::ZOMBIE && entity->isAlive()) {
            Zombie* zombie = dynamic_cast<Zombie*>(entity);
            if (!zombie) continue;

            Vector2D toTarget = zombie->getPosition() - position_;

            // 根据攻击方向判断目标是否在正确的方向上
            switch (attackDirection_) {
                case Direction::UP:
                    if (toTarget.y < 0) return true;
                    break;
                case Direction::DOWN:
                    if (toTarget.y > 0) return true;
                    break;
                case Direction::LEFT:
                    if (toTarget.x < 0) return true;
                    break;
                case Direction::RIGHT:
                    if (toTarget.x > 0) return true;
                    break;
                case Direction::NONE:
                    return true;  // 全方向攻击
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
