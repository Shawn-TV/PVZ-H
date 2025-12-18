/**
 * @file Dave.h
 * @brief 戴夫NPC类
 *
 * 负责：
 * - 使用A*算法追踪僵尸
 * - 自动移动和攻击
 * - 路径规划和导航
 */

#ifndef DAVE_H
#define DAVE_H

#include "Entity.h"
#include <vector>

class Zombie;
class Maze;

class Dave : public Entity {
public:
    Dave(float x, float y, Maze* maze);
    ~Dave();

    void update(float deltaTime) override;
    void onCollision(Entity* other) override;

    void setTarget(Zombie* zombie);
    void updatePath();

private:
    Maze* maze_;
    Zombie* target_;
    std::vector<Vector2D> path_;
    int currentPathIndex_;
    float speed_;
    int attackDamage_;
    float attackRange_;
    float attackCooldown_;

    void followPath(float deltaTime);
    void attackTarget();
};

#endif // DAVE_H
