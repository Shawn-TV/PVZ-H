/**
 * @file PeaShooter.h
 * @brief 豌豆射手
 *
 * 功能：
 * - 向指定方向发射豌豆
 * - 直线攻击
 * - 固定攻击间隔
 * - 只能朝一个方向攻击（僵尸从另一侧来不会发射）
 * - 有待机和攻击动画
 */

#ifndef PEASHOOTER_H
#define PEASHOOTER_H

#include "../entities/Plant.h"

/**
 * 豌豆射手类
 * 特点：定向攻击，只攻击攻击方向上的敌人
 */
class PeaShooter : public Plant {
public:
    // 构造函数：位置 + 攻击方向
    PeaShooter(float x, float y, Direction shootDirection = Direction::RIGHT);
    ~PeaShooter() override;

protected:
    // 动画初始化
    void initializeAnimations() override;

    // 更新动画
    void updateAnimation() override;

    // 执行攻击（发射豌豆）
    void performAttack() override;

private:
    // 豌豆属性
    float peaSpeed_;        // 豌豆速度
    float peaDamage_;       // 豌豆伤害
};

#endif // PEASHOOTER_H
