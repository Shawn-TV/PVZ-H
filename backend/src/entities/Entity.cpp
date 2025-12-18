/**
 * @file Entity.cpp
 * @brief 实体基类实现
 */

#include "../../include/entities/Entity.h"
#include <sstream>

// 静态成员初始化
int Entity::nextId_ = 1;

Entity::Entity(float x, float y, EntityType type)
    : id_(nextId_++), type_(type),
      position_(x, y), velocity_(0, 0), speed_(0), direction_(Direction::NONE),
      health_(100.0f), maxHealth_(100.0f), alive_(true),
      width_(32.0f), height_(32.0f) {
}

Entity::~Entity() {
}

void Entity::takeDamage(float damage) {
    if (!alive_) return;

    health_ -= damage;

    if (health_ <= 0) {
        health_ = 0;
        alive_ = false;
        // 可以在这里播放死亡动画
    }
}

void Entity::heal(float amount) {
    if (!alive_) return;

    health_ += amount;
    if (health_ > maxHealth_) {
        health_ = maxHealth_;
    }
}

void Entity::move(const Vector2D& offset) {
    position_ = position_ + offset;
}

void Entity::move(float dx, float dy) {
    position_.x += dx;
    position_.y += dy;
}

void Entity::setVelocity(const Vector2D& velocity) {
    velocity_ = velocity;
}

void Entity::setVelocity(float vx, float vy) {
    velocity_ = Vector2D(vx, vy);
}

void Entity::setPosition(float x, float y) {
    position_ = Vector2D(x, y);
}

void Entity::setPosition(const Vector2D& pos) {
    position_ = pos;
}

void Entity::setHealth(float health) {
    health_ = health;
    if (health_ <= 0) {
        health_ = 0;
        alive_ = false;
    } else if (health_ > maxHealth_) {
        health_ = maxHealth_;
    }
}

void Entity::setSize(float width, float height) {
    width_ = width;
    height_ = height;
}

void Entity::applyVelocity(float deltaTime) {
    position_ = position_ + velocity_ * deltaTime;
}

std::string Entity::toJson() const {
    std::stringstream ss;
    ss << "{"
       << "\"id\":" << id_ << ","
       << "\"type\":" << static_cast<int>(type_) << ","
       << "\"x\":" << position_.x << ","
       << "\"y\":" << position_.y << ","
       << "\"health\":" << health_ << ","
       << "\"maxHealth\":" << maxHealth_ << ","
       << "\"alive\":" << (alive_ ? "true" : "false")
       << "}";
    return ss.str();
}
