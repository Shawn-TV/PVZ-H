/**
 * @file Entity.cpp
 * @brief 实体基类实现
 */

#include "../../include/entities/Entity.h"

Entity::Entity(float x, float y) : position_(x, y), health_(100.0f), alive_(true) {
}

Entity::~Entity() {
}

Vector2D Entity::getPosition() const {
    return position_;
}

void Entity::setPosition(float x, float y) {
    position_ = Vector2D(x, y);
}

bool Entity::isAlive() const {
    return alive_;
}
