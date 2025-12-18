/**
 * @file PathFinder.h
 * @brief 路径查找管理器
 *
 * 功能：
 * - 封装A*算法调用
 * - 路径平滑和优化
 * - 动态障碍物处理
 */

#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <vector>
#include "../physics/Vector2D.h"

class Maze;

class PathFinder {
public:
    PathFinder(const Maze* maze);

    std::vector<Vector2D> findPath(const Vector2D& start, const Vector2D& goal);
    void smoothPath(std::vector<Vector2D>& path);

private:
    const Maze* maze_;
};

#endif // PATHFINDER_H
