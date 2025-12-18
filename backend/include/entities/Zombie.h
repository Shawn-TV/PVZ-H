/**
 * @file Zombie.h
 * @brief 僵尸类（玩家控制）
 *
 * 负责：
 * - 玩家输入响应
 * - 移动控制
 * - 拾取道具
 * - 受到植物和戴夫的攻击
 * - 装备效果（铁桶等）
 */

#ifndef ZOMBIE_H
#define ZOMBIE_H

#include "Entity.h"
#include <vector>

class Item;

class Zombie : public Entity {
public:
    Zombie(float x, float y);
    ~Zombie();

    void update(float deltaTime) override;
    void onCollision(Entity* other) override;

    // 移动控制
    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();

    // 道具系统
    void pickupItem(Item* item);
    void useItem(int slot);

    // 属性
    int getHealth() const;
    void takeDamage(int damage);
    void heal(int amount);

private:
    float speed_;
    std::vector<Item*> inventory_;
    // TODO: 装备槽
    // TODO: 移动方向
};

#endif // ZOMBIE_H
