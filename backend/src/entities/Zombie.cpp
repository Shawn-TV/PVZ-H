/**
 * @file Zombie.cpp
 * @brief 僵尸类实现
 */

#include "../../include/entities/Zombie.h"

Zombie::Zombie(float x, float y) : Entity(x, y), speed_(2.0f) {
}

Zombie::~Zombie() {
}

void Zombie::update(float deltaTime) {
    // TODO: 更新僵尸状态
}

void Zombie::onCollision(Entity* other) {
    // TODO: 处理碰撞
}

void Zombie::moveUp() {
    // TODO: 向上移动
}

void Zombie::moveDown() {
    // TODO: 向下移动
}

void Zombie::moveLeft() {
    // TODO: 向左移动
}

void Zombie::moveRight() {
    // TODO: 向右移动
}

void Zombie::pickupItem(Item* item) {
    // TODO: 拾取道具
}

void Zombie::useItem(int slot) {
    // TODO: 使用道具
}

int Zombie::getHealth() const {
    return static_cast<int>(health_);
}

void Zombie::takeDamage(int damage) {
    // TODO: 受到伤害
}

void Zombie::heal(int amount) {
    // TODO: 治疗
}
