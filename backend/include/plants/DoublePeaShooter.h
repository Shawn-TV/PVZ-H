/**
 * @file DoublePeaShooter.h
 * @brief 双发射手
 *
 * 功能：
 * - 同时向两个相反方向发射豌豆
 * - 可以攻击两侧的敌人
 * - 攻击间隔比普通豌豆射手稍长
 * - 有待机和攻击动画
 */

#ifndef DOUBLEPEASHOOTER_H
#define DOUBLEPEASHOOTER_H

#include "../entities/Plant.h"

/**
 * 双发射手类
 * 特点：双向同时攻击，可以防守两个方向
 */
class DoublePeaShooter : public Plant {
public:
    // 构造函数：位置 + 攻击轴向（水平或垂直）
    // horizontal=true: 左右发射
    // horizontal=false: 上下发射
    DoublePeaShooter(float x, float y, bool horizontal = true);
    ~DoublePeaShooter() override;

protected:
    // 动画初始化
    void initializeAnimations() override;

    // 更新动画
    void updateAnimation() override;

    // 执行攻击（发射两发豌豆）
    void performAttack() override;

private:
    // 豌豆属性
    float peaSpeed_;        // 豌豆速度
    float peaDamage_;       // 豌豆伤害
    bool isHorizontal_;     // true=左右发射, false=上下发射
    Direction direction1_;  // 第一个发射方向
    Direction direction2_;  // 第二个发射方向（相反方向）

    // 发射单个豌豆的辅助方法
    void shootPea(Direction dir);
};

#endif // DOUBLEPEASHOOTER_H
