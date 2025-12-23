/**
 * @file Projectile.cpp
 * @brief 投射物基类实现
 */

#include "../../include/entities/Projectile.h"
#include "../../include/entities/Zombie.h"
#include "../../include/entities/Dave.h"
#include "../../include/maze/Maze.h"
#include <sstream>

Projectile::Projectile(float x, float y, ProjectileType projType, Direction dir, float speed, float damage)
    : Entity(x, y, EntityType::PROJECTILE),
      projectileType_(projType),
      damage_(damage),
      lifetime_(0),
      maxLifetime_(5.0f),
      maze_(nullptr) {  // 5秒后自动销毁

    direction_ = dir;
    speed_ = speed;

    // 设置碰撞盒（投射物较小）
    setSize(16, 16);

    // 根据方向设置速度向量
    switch (dir) {
        case Direction::UP:
            velocity_ = Vector2D(0, -speed);
            break;
        case Direction::DOWN:
            velocity_ = Vector2D(0, speed);
            break;
        case Direction::LEFT:
            velocity_ = Vector2D(-speed, 0);
            break;
        case Direction::RIGHT:
            velocity_ = Vector2D(speed, 0);
            break;
        default:
            velocity_ = Vector2D(0, 0);
            break;
    }

    // 初始化动画
    initializeAnimations();
}

Projectile::~Projectile() {
}

void Projectile::update(float deltaTime) {
    if (!alive_) return;

    // 更新生命周期
    lifetime_ += deltaTime;
    if (lifetime_ >= maxLifetime_) {
        alive_ = false;
        return;
    }

    // 移动
    applyVelocity(deltaTime);

    // 检查墙壁碰撞
    if (maze_) {
        if (!maze_->isPassableAtPixel(position_.x, position_.y)) {
            // 碰到墙壁，销毁投射物
            alive_ = false;
            return;
        }
    }

    // 更新动画
    updateAnimation();
    animationController_.update(deltaTime);
}

void Projectile::onCollision(Entity* other) {
    if (!other || !alive_) return;

    // 与僵尸或戴夫碰撞时造成伤害并销毁自己
    if (other->getType() == EntityType::ZOMBIE) {
        Zombie* zombie = dynamic_cast<Zombie*>(other);
        if (zombie && zombie->isAlive()) {
            zombie->takeDamage(damage_);
            alive_ = false;  // 碰撞后销毁
        }
    } else if (other->getType() == EntityType::DAVE) {
        Dave* dave = dynamic_cast<Dave*>(other);
        if (dave && dave->isAlive()) {
            dave->takeDamage(damage_);
            alive_ = false;
        }
    }
}

void Projectile::initializeAnimations() {
    // 投射物动画（通常是旋转或飞行动画）
    AnimationClip* flyAnim = new AnimationClip("fly", true);

    switch (projectileType_) {
        case ProjectileType::PEA:
            // 豌豆飞行动画
            flyAnim->addFrame("assets/images/projectiles/pea/frame_0.png", 0.1f);
            flyAnim->addFrame("assets/images/projectiles/pea/frame_1.png", 0.1f);
            flyAnim->addFrame("assets/images/projectiles/pea/frame_2.png", 0.1f);
            flyAnim->addFrame("assets/images/projectiles/pea/frame_3.png", 0.1f);
            break;
    }

    animationController_.registerAnimation(flyAnim);
    animationController_.play("fly");
}

void Projectile::updateAnimation() {
    // 投射物只有飞行动画
    if (!animationController_.isPlaying("fly")) {
        animationController_.play("fly");
    }
}

std::string Projectile::toJson() const {
    std::stringstream ss;
    ss << "{"
       << "\"id\":" << id_ << ","
       << "\"type\":\"projectile\","
       << "\"projectileType\":" << static_cast<int>(projectileType_) << ","
       << "\"x\":" << position_.x << ","
       << "\"y\":" << position_.y << ","
       << "\"direction\":" << static_cast<int>(direction_) << ","
       << "\"damage\":" << damage_ << ","
       << "\"alive\":" << (alive_ ? "true" : "false")
       << "}";
    return ss.str();
}
