/**
 * @file Cell.h
 * @brief 迷宫单元格
 *
 * 表示迷宫中的一个格子：
 * - 墙壁或通道
 * - 是否有植物
 * - 是否有道具
 */

#ifndef CELL_H
#define CELL_H

enum class CellType {
    WALL,
    PATH,
    ENTRANCE,
    EXIT
};

class Cell {
public:
    Cell();

    CellType getType() const;
    void setType(CellType type);

    bool hasPlant() const;
    bool hasItem() const;

private:
    CellType type_;
    // TODO: 植物指针
    // TODO: 道具指针
};

#endif // CELL_H
