/**
 * @file DoublePeaShooter.h
 * @brief 双发射手
 *
 * 功能：
 * - 向一个方向连续发射两颗豌豆
 * - 伤害输出是普通豌豆射手的两倍
 * - 攻击间隔与普通豌豆射手相同
 * - 有待机和攻击动画
 */

#ifndef DOUBLEPEASHOOTER_H
#define DOUBLEPEASHOOTER_H

#include "../entities/Plant.h"

/**
 * 双发射手类
 * 特点：单向发射，每次发射两颗豌豆（连续发射）
 */
class DoublePeaShooter : public Plant {
public:
    // 构造函数：位置 + 攻击方向
    DoublePeaShooter(float x, float y, Direction shootDirection = Direction::RIGHT);
    ~DoublePeaShooter() override;

protected:
    // 动画初始化
    void initializeAnimations() override;

    // 更新动画
    void updateAnimation() override;

    // 执行攻击（连续发射两颗豌豆）
    void performAttack() override;

private:
    // 豌豆属性
    float peaSpeed_;        // 豌豆速度
    float peaDamage_;       // 豌豆伤害
    float peaInterval_;     // 两颗豌豆之间的间隔（秒）

    // 发射单颗豌豆的辅助方法
    void shootPea(float offsetTime = 0.0f);
};

#endif // DOUBLEPEASHOOTER_H
