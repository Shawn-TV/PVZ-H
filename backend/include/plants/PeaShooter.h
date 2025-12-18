/**
 * @file PeaShooter.h
 * @brief 豌豆射手
 *
 * 功能：
 * - 向僵尸发射豌豆
 * - 直线攻击
 * - 固定攻击间隔
 */

#ifndef PEASHOOTER_H
#define PEASHOOTER_H

#include "../entities/Plant.h"

class PeaShooter : public Plant {
public:
    PeaShooter(float x, float y);
    ~PeaShooter();

    void update(float deltaTime) override;

private:
    void shoot();
    // TODO: 子弹管理
};

#endif // PEASHOOTER_H
