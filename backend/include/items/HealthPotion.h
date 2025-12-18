/**
 * @file HealthPotion.h
 * @brief 生命药水道具
 *
 * 功能：
 * - 恢复僵尸生命值
 * - 立即使用型道具
 */

#ifndef HEALTHPOTION_H
#define HEALTHPOTION_H

#include "../entities/Item.h"

class HealthPotion : public Item {
public:
    HealthPotion(float x, float y);
    ~HealthPotion();

    void use(class Zombie* zombie) override;

private:
    int healAmount_;
};

#endif // HEALTHPOTION_H
