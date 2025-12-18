/**
 * @file renderer.js
 * @brief 游戏渲染器
 *
 * 负责：
 * - 渲染迷宫
 * - 渲染所有实体（僵尸、植物、戴夫、道具）
 * - 应用摄像机变换
 */

export class GameRenderer {
    constructor(ctx) {
        this.ctx = ctx;
        this.sprites = new Map(); // 精灵图集合
    }

    loadSprites() {
        // TODO: 加载所有游戏精灵图
    }

    render(gameState, camera) {
        // TODO: 清空画布
        // TODO: 应用摄像机变换
        // TODO: 渲染迷宫
        // TODO: 渲染实体
    }

    renderMaze(maze, camera) {
        // TODO: 渲染迷宫单元格
    }

    renderEntity(entity, camera) {
        // TODO: 渲染单个实体
    }
}
