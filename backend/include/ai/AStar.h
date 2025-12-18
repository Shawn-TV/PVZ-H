/**
 * @file AStar.h
 * @brief A*寻路算法实现
 *
 * 功能：
 * - A*路径查找算法
 * - 启发式函数（曼哈顿距离）
 * - 返回从起点到终点的路径
 */

#ifndef ASTAR_H
#define ASTAR_H

#include <vector>
#include "../physics/Vector2D.h"

class Maze;

class AStar {
public:
    // 查找从start到goal的路径
    static std::vector<Vector2D> findPath(
        const Vector2D& start,
        const Vector2D& goal,
        const Maze* maze
    );

private:
    // 节点结构
    struct Node {
        Vector2D position;
        float g; // 从起点到当前节点的代价
        float h; // 启发式估计（到终点的估计代价）
        float f; // f = g + h
        Node* parent;

        Node(Vector2D pos, float g_cost, float h_cost, Node* p = nullptr);
    };

    static float heuristic(const Vector2D& a, const Vector2D& b);
    static std::vector<Vector2D> reconstructPath(Node* endNode);
};

#endif // ASTAR_H
