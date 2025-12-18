/**
 * @file Item.h
 * @brief 道具基类
 *
 * 所有可拾取道具的基类：
 * - 在迷宫中可被拾取
 * - 使用效果
 */

#ifndef ITEM_H
#define ITEM_H

#include "Entity.h"

enum class ItemType {
    BUCKET,
    HEALTH_POTION,
    SPEED_BOOST,
    SHIELD
};

class Item : public Entity {
public:
    Item(float x, float y, ItemType type);
    virtual ~Item();

    void update(float deltaTime) override;
    void onCollision(Entity* other) override;

    virtual void use(class Zombie* zombie) = 0;

    ItemType getType() const;
    bool isPickedUp() const;

protected:
    ItemType type_;
    bool pickedUp_;
};

#endif // ITEM_H
