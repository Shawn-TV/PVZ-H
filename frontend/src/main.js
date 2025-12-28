/**
 * @file main.js
 * @brief 前端主入口文件
 *
 * 负责：
 * - 初始化Phaser游戏引擎
 * - 建立与后端的WebSocket连接
 * - 启动游戏
 */

import { startGame } from './game.js';

// 启动游戏
window.addEventListener('load', async () => {
    try {
        await startGame();
    } catch (error) {
        console.error('游戏启动失败:', error);
    }
});
