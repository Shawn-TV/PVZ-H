/**
 * @file AStar.cpp
 * @brief 寻路算法实现 (使用BFS，简单有效)
 */

#include "../../include/ai/AStar.h"
#include "../../include/maze/Maze.h"
#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_set>
#include <unordered_map>

// 用于哈希pair<int,int>
struct PairHash {
    std::size_t operator()(const std::pair<int, int>& p) const {
        return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 16);
    }
};

AStar::Node::Node(Vector2D pos, float g_cost, float h_cost, Node* p)
    : position(pos), g(g_cost), h(h_cost), f(g_cost + h_cost), parent(p) {
}

std::vector<Vector2D> AStar::findPath(
    const Vector2D& start,
    const Vector2D& goal,
    const Maze* maze
) {
    if (!maze) {
        return std::vector<Vector2D>();
    }

    int cellSize = maze->getCellSize();

    // 将像素坐标转换为网格坐标
    int startGridX, startGridY, goalGridX, goalGridY;
    maze->pixelToGrid(start.x, start.y, startGridX, startGridY);
    maze->pixelToGrid(goal.x, goal.y, goalGridX, goalGridY);

    // 如果起点或终点不可通过，直接返回空路径
    if (!maze->isPassable(startGridX, startGridY)) {
        // 尝试找到附近的可通过格子作为起点
        bool found = false;
        for (int dy = -1; dy <= 1 && !found; dy++) {
            for (int dx = -1; dx <= 1 && !found; dx++) {
                if (maze->isPassable(startGridX + dx, startGridY + dy)) {
                    startGridX += dx;
                    startGridY += dy;
                    found = true;
                }
            }
        }
        if (!found) return std::vector<Vector2D>();
    }

    if (!maze->isPassable(goalGridX, goalGridY)) {
        // 尝试找到附近的可通过格子作为终点
        bool found = false;
        for (int dy = -1; dy <= 1 && !found; dy++) {
            for (int dx = -1; dx <= 1 && !found; dx++) {
                if (maze->isPassable(goalGridX + dx, goalGridY + dy)) {
                    goalGridX += dx;
                    goalGridY += dy;
                    found = true;
                }
            }
        }
        if (!found) return std::vector<Vector2D>();
    }

    // BFS寻路
    std::queue<std::pair<int, int>> queue;
    std::unordered_set<std::pair<int, int>, PairHash> visited;
    std::unordered_map<std::pair<int, int>, std::pair<int, int>, PairHash> parent;

    std::pair<int, int> startCell = {startGridX, startGridY};
    std::pair<int, int> goalCell = {goalGridX, goalGridY};

    queue.push(startCell);
    visited.insert(startCell);
    parent[startCell] = {-1, -1};  // 起点没有父节点

    // 四个方向：上、右、下、左
    const int dx[] = {0, 1, 0, -1};
    const int dy[] = {-1, 0, 1, 0};

    bool found = false;

    while (!queue.empty() && !found) {
        auto current = queue.front();
        queue.pop();

        if (current == goalCell) {
            found = true;
            break;
        }

        // 扩展四个方向的邻居
        for (int i = 0; i < 4; i++) {
            int nx = current.first + dx[i];
            int ny = current.second + dy[i];
            std::pair<int, int> neighbor = {nx, ny};

            // 检查是否可通过且未访问
            if (maze->isPassable(nx, ny) && visited.find(neighbor) == visited.end()) {
                visited.insert(neighbor);
                parent[neighbor] = current;
                queue.push(neighbor);

                if (neighbor == goalCell) {
                    found = true;
                    break;
                }
            }
        }
    }

    // 如果没找到路径，返回空
    if (!found) {
        return std::vector<Vector2D>();
    }

    // 重构路径（从终点回溯到起点）
    std::vector<Vector2D> path;
    std::pair<int, int> current = goalCell;

    while (current.first != -1 && current.second != -1) {
        // 将网格坐标转换为像素坐标（格子中心）
        float pixelX = (current.first + 0.5f) * cellSize;
        float pixelY = (current.second + 0.5f) * cellSize;
        path.push_back(Vector2D(pixelX, pixelY));

        if (parent.find(current) != parent.end()) {
            current = parent[current];
        } else {
            break;
        }
    }

    // 反转路径（从起点到终点）
    std::reverse(path.begin(), path.end());

    // 移除起点（因为Dave已经在起点了）
    if (!path.empty()) {
        path.erase(path.begin());
    }

    return path;
}

float AStar::heuristic(const Vector2D& a, const Vector2D& b) {
    // 曼哈顿距离
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

std::vector<Vector2D> AStar::reconstructPath(Node* endNode) {
    std::vector<Vector2D> path;
    Node* current = endNode;

    while (current != nullptr) {
        path.push_back(current->position);
        current = current->parent;
    }

    std::reverse(path.begin(), path.end());
    return path;
}
