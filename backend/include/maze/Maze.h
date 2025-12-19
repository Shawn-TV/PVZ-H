/**
 * @file Maze.h
 * @brief 迷宫类 - 基于格子的迷宫系统
 *
 * 设计说明：
 * - 迷宫采用格子系统，每个格子大小固定（类似PVZ的PlantingPlot）
 * - 格子可以是墙、通道、入口、出口等类型
 * - 墙的宽度 = 一个格子宽度
 * - 植物和僵尸都位于格子中心
 * - 迷宫足够大，玩家需要约2分钟探索完成
 */

#ifndef MAZE_H
#define MAZE_H

#include <vector>
#include <string>
#include "../physics/Vector2D.h"

/**
 * 格子类型枚举
 */
enum class CellType {
    WALL,           // 墙壁（不可通过）
    PATH,           // 通道（可通过）
    ENTRANCE,       // 入口
    EXIT,           // 出口
    ITEM_SPAWN      // 道具生成点
};

/**
 * 迷宫格子结构
 */
struct MazeCell {
    CellType type;              // 格子类型
    bool visited;               // 用于生成算法的访问标记
    bool hasItem;               // 是否有道具
    bool hasPlant;              // 是否有植物

    MazeCell()
        : type(CellType::WALL),
          visited(false),
          hasItem(false),
          hasPlant(false) {}
};

/**
 * 迷宫类
 *
 * 迷宫尺寸设计：
 * - 格子大小：80x80 像素（CELL_SIZE）
 * - 迷宫网格：宽度约21格，高度约31格（可配置）
 * - 实际像素尺寸：约1680x2480像素
 * - 路径长度：足够支持2分钟的探索时间
 */
class Maze {
public:
    /**
     * 构造函数
     * @param gridWidth 迷宫网格宽度（格子数，推荐21）
     * @param gridHeight 迷宫网格高度（格子数，推荐31）
     * @param cellSize 每个格子的像素大小（推荐80）
     */
    Maze(int gridWidth = 21, int gridHeight = 31, int cellSize = 80);
    ~Maze();

    // ==================== 迷宫生成 ====================

    /**
     * 生成迷宫
     * 使用递归回溯算法生成迷宫，确保有唯一路径从入口到出口
     */
    void generate();

    /**
     * 清空迷宫（重新生成前调用）
     */
    void clear();

    // ==================== 查询接口 ====================

    /**
     * 检查指定格子是否是墙
     * @param gridX 格子X坐标
     * @param gridY 格子Y坐标
     */
    bool isWall(int gridX, int gridY) const;

    /**
     * 检查指定格子是否可通过
     * @param gridX 格子X坐标
     * @param gridY 格子Y坐标
     */
    bool isPassable(int gridX, int gridY) const;

    /**
     * 检查指定像素位置是否可通过
     * @param pixelX 像素X坐标
     * @param pixelY 像素Y坐标
     */
    bool isPassableAtPixel(float pixelX, float pixelY) const;

    /**
     * 检查指定格子是否在迷宫范围内
     */
    bool isInBounds(int gridX, int gridY) const;

    /**
     * 获取指定格子的类型
     */
    CellType getCellType(int gridX, int gridY) const;

    /**
     * 获取指定格子的引用（用于修改）
     */
    MazeCell& getCell(int gridX, int gridY);
    const MazeCell& getCell(int gridX, int gridY) const;

    // ==================== 坐标转换 ====================

    /**
     * 像素坐标转格子坐标
     */
    void pixelToGrid(float pixelX, float pixelY, int& gridX, int& gridY) const;

    /**
     * 格子坐标转像素坐标（格子中心）
     */
    void gridToPixel(int gridX, int gridY, float& pixelX, float& pixelY) const;

    /**
     * 格子坐标转像素坐标（返回Vector2D）
     */
    Vector2D gridToPixel(int gridX, int gridY) const;

    // ==================== 入口和出口 ====================

    /**
     * 获取入口位置（像素坐标）
     */
    Vector2D getEntrancePosition() const;

    /**
     * 获取出口位置（像素坐标）
     */
    Vector2D getExitPosition() const;

    /**
     * 获取入口格子坐标
     */
    void getEntranceGrid(int& gridX, int& gridY) const;

    /**
     * 获取出口格子坐标
     */
    void getExitGrid(int& gridX, int& gridY) const;

    // ==================== 道具生成点 ====================

    /**
     * 获取所有道具生成点的像素坐标
     */
    std::vector<Vector2D> getItemSpawnPositions() const;

    /**
     * 在迷宫中随机选择N个通道格子作为道具生成点
     * @param count 道具生成点数量
     */
    void generateItemSpawns(int count);

    // ==================== 属性获取 ====================

    int getGridWidth() const { return gridWidth_; }
    int getGridHeight() const { return gridHeight_; }
    int getCellSize() const { return cellSize_; }
    int getPixelWidth() const { return gridWidth_ * cellSize_; }
    int getPixelHeight() const { return gridHeight_ * cellSize_; }

    // ==================== 序列化 ====================

    /**
     * 将迷宫转换为JSON格式（用于发送给前端）
     */
    std::string toJson() const;

private:
    // 迷宫尺寸
    int gridWidth_;             // 网格宽度（格子数）
    int gridHeight_;            // 网格高度（格子数）
    int cellSize_;              // 每个格子的像素大小

    // 迷宫数据
    std::vector<std::vector<MazeCell>> grid_;  // 二维格子数组

    // 入口和出口
    int entranceX_, entranceY_; // 入口格子坐标
    int exitX_, exitY_;         // 出口格子坐标

    // 道具生成点
    std::vector<std::pair<int, int>> itemSpawns_;  // 道具生成点格子坐标列表

    // ==================== 内部生成算法 ====================

    /**
     * 递归回溯算法生成迷宫
     * @param x 当前格子X坐标
     * @param y 当前格子Y坐标
     */
    void recursiveBacktrack(int x, int y);

    /**
     * 设置入口和出口
     * 入口在顶部，出口在底部，尽量远离
     */
    void setEntranceAndExit();

    /**
     * 打通从入口到出口的路径（确保可达）
     */
    void ensurePathExists();

    /**
     * 获取指定格子的未访问邻居（用于生成算法）
     */
    std::vector<std::pair<int, int>> getUnvisitedNeighbors(int x, int y);

    /**
     * 移除两个格子之间的墙
     */
    void removeWallBetween(int x1, int y1, int x2, int y2);

    /**
     * 检查指定格子是否是死路尽头
     * 死路尽头定义：只有一个相邻通道的通道格子
     */
    bool isDeadEnd(int x, int y) const;
};

#endif // MAZE_H
