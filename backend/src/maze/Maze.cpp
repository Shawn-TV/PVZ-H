/**
 * @file Maze.cpp
 * @brief 迷宫类实现
 */

#include "../../include/maze/Maze.h"
#include <random>
#include <algorithm>
#include <sstream>
#include <queue>
#include <ctime>

// ==================== 构造和析构 ====================

Maze::Maze(int gridWidth, int gridHeight, int cellSize)
    : gridWidth_(gridWidth),
      gridHeight_(gridHeight),
      cellSize_(cellSize),
      entranceX_(0),
      entranceY_(0),
      exitX_(0),
      exitY_(0) {

    // 初始化网格
    grid_.resize(gridHeight_);
    for (int y = 0; y < gridHeight_; y++) {
        grid_[y].resize(gridWidth_);
    }
}

Maze::~Maze() {
}

// ==================== 迷宫生成 ====================

void Maze::generate() {
    // 清空迷宫
    clear();

    // 使用递归回溯算法生成迷宫
    // 从中心点开始生成
    int startX = gridWidth_ / 2;
    int startY = gridHeight_ / 2;
    recursiveBacktrack(startX, startY);

    // 设置入口和出口
    setEntranceAndExit();

    // 确保从入口到出口有路径
    ensurePathExists();

    // 生成道具生成点（约10-15个）
    generateItemSpawns(12);
}

void Maze::clear() {
    // 重置所有格子为墙
    for (int y = 0; y < gridHeight_; y++) {
        for (int x = 0; x < gridWidth_; x++) {
            grid_[y][x].type = CellType::WALL;
            grid_[y][x].visited = false;
            grid_[y][x].hasItem = false;
            grid_[y][x].hasPlant = false;
        }
    }

    // 清空道具生成点
    itemSpawns_.clear();
}

// ==================== 递归回溯算法 ====================

void Maze::recursiveBacktrack(int x, int y) {
    // 标记当前格子为已访问的通道
    grid_[y][x].visited = true;
    grid_[y][x].type = CellType::PATH;

    // 定义四个方向：上、右、下、左
    std::vector<std::pair<int, int>> directions = {
        {0, -2}, // 上（跳过一格墙）
        {2, 0},  // 右
        {0, 2},  // 下
        {-2, 0}  // 左
    };

    // 随机打乱方向
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::shuffle(directions.begin(), directions.end(), gen);

    // 遍历所有方向
    for (const auto& dir : directions) {
        int nx = x + dir.first;
        int ny = y + dir.second;

        // 检查邻居是否在范围内且未访问
        if (isInBounds(nx, ny) && !grid_[ny][nx].visited) {
            // 打通当前格子和邻居之间的墙
            removeWallBetween(x, y, nx, ny);

            // 递归访问邻居
            recursiveBacktrack(nx, ny);
        }
    }
}

void Maze::removeWallBetween(int x1, int y1, int x2, int y2) {
    // 计算两个格子之间的墙的坐标
    int wallX = (x1 + x2) / 2;
    int wallY = (y1 + y2) / 2;

    // 将墙变为通道
    grid_[wallY][wallX].type = CellType::PATH;
    grid_[wallY][wallX].visited = true;
}

std::vector<std::pair<int, int>> Maze::getUnvisitedNeighbors(int x, int y) {
    std::vector<std::pair<int, int>> neighbors;

    // 四个方向，间隔2格（因为中间是墙）
    std::vector<std::pair<int, int>> directions = {
        {0, -2}, {2, 0}, {0, 2}, {-2, 0}
    };

    for (const auto& dir : directions) {
        int nx = x + dir.first;
        int ny = y + dir.second;

        if (isInBounds(nx, ny) && !grid_[ny][nx].visited) {
            neighbors.push_back({nx, ny});
        }
    }

    return neighbors;
}

// ==================== 入口和出口 ====================

void Maze::setEntranceAndExit() {
    // 入口设在顶部中央附近的通道格子
    // 出口设在底部中央附近的通道格子

    // 寻找顶部第一行的通道格子作为入口
    entranceX_ = gridWidth_ / 2;
    entranceY_ = 1;

    // 确保入口是通道
    for (int x = 1; x < gridWidth_ - 1; x++) {
        if (grid_[1][x].type == CellType::PATH) {
            entranceX_ = x;
            break;
        }
    }

    // 如果没找到，强制设置
    grid_[entranceY_][entranceX_].type = CellType::ENTRANCE;

    // 寻找底部最后一行的通道格子作为出口
    exitX_ = gridWidth_ / 2;
    exitY_ = gridHeight_ - 2;

    // 确保出口是通道
    for (int x = gridWidth_ - 2; x > 0; x--) {
        if (grid_[gridHeight_ - 2][x].type == CellType::PATH) {
            exitX_ = x;
            break;
        }
    }

    // 如果没找到，强制设置
    grid_[exitY_][exitX_].type = CellType::EXIT;
}

void Maze::ensurePathExists() {
    // 使用BFS确保从入口到出口有路径
    // 如果没有路径，则打通必要的墙

    std::queue<std::pair<int, int>> queue;
    std::vector<std::vector<bool>> visited(gridHeight_, std::vector<bool>(gridWidth_, false));

    queue.push({entranceX_, entranceY_});
    visited[entranceY_][entranceX_] = true;

    bool pathExists = false;

    while (!queue.empty()) {
        auto [x, y] = queue.front();
        queue.pop();

        if (x == exitX_ && y == exitY_) {
            pathExists = true;
            break;
        }

        // 检查四个方向的相邻格子
        std::vector<std::pair<int, int>> directions = {
            {0, -1}, {1, 0}, {0, 1}, {-1, 0}
        };

        for (const auto& dir : directions) {
            int nx = x + dir.first;
            int ny = y + dir.second;

            if (isInBounds(nx, ny) && !visited[ny][nx] && isPassable(nx, ny)) {
                visited[ny][nx] = true;
                queue.push({nx, ny});
            }
        }
    }

    // 如果没有路径，则强制打通一条直线路径（简单处理）
    if (!pathExists) {
        // 打通从入口到出口的垂直路径
        for (int y = entranceY_; y <= exitY_; y++) {
            grid_[y][entranceX_].type = CellType::PATH;
        }
        // 打通水平路径到出口
        int startX = std::min(entranceX_, exitX_);
        int endX = std::max(entranceX_, exitX_);
        for (int x = startX; x <= endX; x++) {
            grid_[exitY_][x].type = CellType::PATH;
        }
    }
}

// ==================== 道具生成点 ====================

void Maze::generateItemSpawns(int count) {
    itemSpawns_.clear();

    // 收集所有通道格子
    std::vector<std::pair<int, int>> pathCells;
    for (int y = 0; y < gridHeight_; y++) {
        for (int x = 0; x < gridWidth_; x++) {
            if (grid_[y][x].type == CellType::PATH) {
                pathCells.push_back({x, y});
            }
        }
    }

    // 随机选择指定数量的格子作为道具生成点
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::shuffle(pathCells.begin(), pathCells.end(), gen);

    int actualCount = std::min(count, static_cast<int>(pathCells.size()));
    for (int i = 0; i < actualCount; i++) {
        auto [x, y] = pathCells[i];
        itemSpawns_.push_back({x, y});
        grid_[y][x].type = CellType::ITEM_SPAWN;
    }
}

std::vector<Vector2D> Maze::getItemSpawnPositions() const {
    std::vector<Vector2D> positions;
    for (const auto& [x, y] : itemSpawns_) {
        positions.push_back(gridToPixel(x, y));
    }
    return positions;
}

// ==================== 查询接口 ====================

bool Maze::isWall(int gridX, int gridY) const {
    if (!isInBounds(gridX, gridY)) return true;
    return grid_[gridY][gridX].type == CellType::WALL;
}

bool Maze::isPassable(int gridX, int gridY) const {
    if (!isInBounds(gridX, gridY)) return false;
    CellType type = grid_[gridY][gridX].type;
    return type != CellType::WALL;
}

bool Maze::isPassableAtPixel(float pixelX, float pixelY) const {
    int gridX, gridY;
    pixelToGrid(pixelX, pixelY, gridX, gridY);
    return isPassable(gridX, gridY);
}

bool Maze::isInBounds(int gridX, int gridY) const {
    return gridX >= 0 && gridX < gridWidth_ && gridY >= 0 && gridY < gridHeight_;
}

CellType Maze::getCellType(int gridX, int gridY) const {
    if (!isInBounds(gridX, gridY)) return CellType::WALL;
    return grid_[gridY][gridX].type;
}

MazeCell& Maze::getCell(int gridX, int gridY) {
    return grid_[gridY][gridX];
}

const MazeCell& Maze::getCell(int gridX, int gridY) const {
    return grid_[gridY][gridX];
}

// ==================== 坐标转换 ====================

void Maze::pixelToGrid(float pixelX, float pixelY, int& gridX, int& gridY) const {
    gridX = static_cast<int>(pixelX / cellSize_);
    gridY = static_cast<int>(pixelY / cellSize_);
}

void Maze::gridToPixel(int gridX, int gridY, float& pixelX, float& pixelY) const {
    // 返回格子中心的像素坐标
    pixelX = (gridX + 0.5f) * cellSize_;
    pixelY = (gridY + 0.5f) * cellSize_;
}

Vector2D Maze::gridToPixel(int gridX, int gridY) const {
    float pixelX = (gridX + 0.5f) * cellSize_;
    float pixelY = (gridY + 0.5f) * cellSize_;
    return Vector2D(pixelX, pixelY);
}

// ==================== 入口和出口位置 ====================

Vector2D Maze::getEntrancePosition() const {
    return gridToPixel(entranceX_, entranceY_);
}

Vector2D Maze::getExitPosition() const {
    return gridToPixel(exitX_, exitY_);
}

void Maze::getEntranceGrid(int& gridX, int& gridY) const {
    gridX = entranceX_;
    gridY = entranceY_;
}

void Maze::getExitGrid(int& gridX, int& gridY) const {
    gridX = exitX_;
    gridY = exitY_;
}

// ==================== 序列化 ====================

std::string Maze::toJson() const {
    std::stringstream ss;
    ss << "{";

    // 迷宫尺寸信息
    ss << "\"gridWidth\":" << gridWidth_ << ",";
    ss << "\"gridHeight\":" << gridHeight_ << ",";
    ss << "\"cellSize\":" << cellSize_ << ",";
    ss << "\"pixelWidth\":" << getPixelWidth() << ",";
    ss << "\"pixelHeight\":" << getPixelHeight() << ",";

    // 入口和出口
    ss << "\"entrance\":{\"x\":" << entranceX_ << ",\"y\":" << entranceY_ << "},";
    ss << "\"exit\":{\"x\":" << exitX_ << ",\"y\":" << exitY_ << "},";

    // 网格数据（压缩格式）
    ss << "\"grid\":[";
    for (int y = 0; y < gridHeight_; y++) {
        if (y > 0) ss << ",";
        ss << "[";
        for (int x = 0; x < gridWidth_; x++) {
            if (x > 0) ss << ",";
            // 使用数字表示格子类型：0=WALL, 1=PATH, 2=ENTRANCE, 3=EXIT, 4=ITEM_SPAWN
            int cellValue = static_cast<int>(grid_[y][x].type);
            ss << cellValue;
        }
        ss << "]";
    }
    ss << "]";

    ss << "}";
    return ss.str();
}
