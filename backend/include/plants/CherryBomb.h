/**
 * @file CherryBomb.h
 * @brief 樱桃炸弹
 *
 * 功能：
 * - 范围爆炸伤害
 * - 一定时间后自动爆炸
 * - 对区域内所有实体造成伤害
 */

#ifndef CHERRYBOMB_H
#define CHERRYBOMB_H

#include "../entities/Plant.h"

class CherryBomb : public Plant {
public:
    CherryBomb(float x, float y);
    ~CherryBomb();

    void update(float deltaTime) override;

private:
    float explosionTimer_;
    float explosionRadius_;
    int explosionDamage_;

    void explode();
};

#endif // CHERRYBOMB_H
