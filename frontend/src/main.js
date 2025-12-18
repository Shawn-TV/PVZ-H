/**
 * @file main.js
 * @brief 前端主入口文件
 *
 * 负责：
 * - 初始化游戏渲染引擎
 * - 建立与后端的WebSocket连接
 * - 启动游戏循环
 */

import { GameRenderer } from './game/renderer.js';
import { Camera } from './game/camera.js';
import { NetworkClient } from './network/client.js';
import { InputHandler } from './utils/input.js';
import { HUD } from './ui/hud.js';

class GameClient {
    constructor() {
        this.canvas = document.getElementById('game-canvas');
        this.ctx = this.canvas.getContext('2d');

        // 设置画布大小
        this.canvas.width = 1280;
        this.canvas.height = 720;

        // 初始化各个系统
        this.renderer = new GameRenderer(this.ctx);
        this.camera = new Camera(this.canvas.width, this.canvas.height);
        this.network = new NetworkClient('ws://localhost:8080');
        this.input = new InputHandler();
        this.hud = new HUD();

        this.gameState = null;
    }

    async initialize() {
        // TODO: 连接服务器
        // TODO: 加载资源
        // TODO: 初始化输入处理
        console.log('游戏客户端初始化...');
    }

    start() {
        // TODO: 启动游戏循环
        this.gameLoop();
    }

    gameLoop() {
        // TODO: 游戏循环
        // 1. 处理输入
        // 2. 更新游戏状态
        // 3. 渲染画面
        requestAnimationFrame(() => this.gameLoop());
    }
}

// 启动游戏
window.addEventListener('load', () => {
    const game = new GameClient();
    game.initialize().then(() => {
        game.start();
    });
});
