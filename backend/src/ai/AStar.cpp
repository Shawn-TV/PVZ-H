/**
 * @file AStar.cpp
 * @brief A*寻路算法实现
 */

#include "../../include/ai/AStar.h"
#include "../../include/maze/Maze.h"
#include <cmath>

AStar::Node::Node(Vector2D pos, float g_cost, float h_cost, Node* p)
    : position(pos), g(g_cost), h(h_cost), f(g_cost + h_cost), parent(p) {
}

std::vector<Vector2D> AStar::findPath(
    const Vector2D& start,
    const Vector2D& goal,
    const Maze* maze
) {
    // TODO: 实现A*算法
    // 1. 初始化开放列表和关闭列表
    // 2. 将起点加入开放列表
    // 3. 循环直到找到终点或开放列表为空
    // 4. 从开放列表中取出f值最小的节点
    // 5. 将该节点加入关闭列表
    // 6. 检查是否到达终点
    // 7. 扩展邻居节点
    // 8. 重构路径

    return std::vector<Vector2D>();
}

float AStar::heuristic(const Vector2D& a, const Vector2D& b) {
    // 曼哈顿距离
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

std::vector<Vector2D> AStar::reconstructPath(Node* endNode) {
    // TODO: 从终点回溯到起点重构路径
    return std::vector<Vector2D>();
}
