/**
 * @file Maze.h
 * @brief 迷宫类
 *
 * 负责：
 * - 存储迷宫结构数据
 * - 提供迷宫查询接口
 * - 管理迷宫中的静态元素（墙壁、入口、出口）
 */

#ifndef MAZE_H
#define MAZE_H

#include <vector>

class Cell; // 前向声明

class Maze {
public:
    Maze(int width, int height);
    ~Maze();

    void generate();
    bool isWall(int x, int y) const;
    bool isPassable(int x, int y) const;

private:
    int width_;
    int height_;
    // TODO: 迷宫单元格数据
    // TODO: 入口和出口坐标
};

#endif // MAZE_H
