/**
 * @file WallNut.h
 * @brief 坚果墙
 *
 * 功能：
 * - 高生命值
 * - 阻挡僵尸移动
 * - 不攻击，纯防御
 */

#ifndef WALLNUT_H
#define WALLNUT_H

#include "../entities/Plant.h"

class WallNut : public Plant {
public:
    WallNut(float x, float y);
    ~WallNut();

    void update(float deltaTime) override;

private:
    // 坚果墙主要通过高生命值和碰撞来阻挡
};

#endif // WALLNUT_H
