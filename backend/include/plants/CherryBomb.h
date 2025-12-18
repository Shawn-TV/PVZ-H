/**
 * @file CherryBomb.h
 * @brief 樱桃炸弹
 *
 * 功能：
 * - 范围爆炸伤害
 * - 种植后3秒自动爆炸
 * - 对爆炸范围内所有实体造成伤害
 * - 爆炸后自动销毁
 * - 有倒计时和爆炸动画
 */

#ifndef CHERRYBOMB_H
#define CHERRYBOMB_H

#include "../entities/Plant.h"

/**
 * 樱桃炸弹类
 * 特点：定时爆炸，范围伤害，一次性使用
 */
class CherryBomb : public Plant {
public:
    CherryBomb(float x, float y);
    ~CherryBomb() override;

    // 重写update，添加倒计时逻辑
    void update(float deltaTime) override;

protected:
    // 动画初始化
    void initializeAnimations() override;

    // 更新动画
    void updateAnimation() override;

    // 樱桃炸弹不使用常规攻击
    void performAttack() override;

private:
    float explosionTimer_;      // 爆炸倒计时
    float explosionDelay_;      // 爆炸延迟（种植后多久爆炸）
    float explosionRadius_;     // 爆炸半径
    float explosionDamage_;     // 爆炸伤害
    bool hasExploded_;          // 是否已爆炸

    // 执行爆炸
    void explode();
};

#endif // CHERRYBOMB_H
