/**
 * @file WallNut.h
 * @brief 坚果墙
 *
 * 功能：
 * - 超高生命值（300点）
 * - 阻挡僵尸移动
 * - 不攻击，纯防御
 * - 有不同受损程度的外观（完好、轻伤、重伤）
 */

#ifndef WALLNUT_H
#define WALLNUT_H

#include "../entities/Plant.h"

/**
 * 坚果墙类
 * 特点：高血量，阻挡作用，不攻击
 */
class WallNut : public Plant {
public:
    WallNut(float x, float y);
    ~WallNut() override;

protected:
    // 动画初始化
    void initializeAnimations() override;

    // 更新动画（根据生命值显示不同外观）
    void updateAnimation() override;

    // 坚果墙不攻击
    void performAttack() override;

private:
    // 根据生命值百分比获取外观状态
    int getDamageState() const;
};

#endif // WALLNUT_H
