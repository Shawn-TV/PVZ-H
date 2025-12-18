/**
 * @file MazeGenerator.h
 * @brief 迷宫生成算法
 *
 * 负责：
 * - 使用算法生成迷宫（如DFS、Prim等）
 * - 确保迷宫有解
 * - 放置植物和道具
 */

#ifndef MAZEGENERATOR_H
#define MAZEGENERATOR_H

class Maze;

class MazeGenerator {
public:
    static void generateDFS(Maze& maze);
    static void generatePrim(Maze& maze);
    static void placePlants(Maze& maze, int count);
    static void placeItems(Maze& maze, int count);

private:
    // TODO: 生成算法实现
};

#endif // MAZEGENERATOR_H
