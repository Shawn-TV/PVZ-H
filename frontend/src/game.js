/**
 * @file game.js
 * @brief Phaser游戏配置和启动
 */

import Phaser from 'phaser';
import { MainMenuScene } from './scenes/MainMenuScene.js';
import { GameScene } from './scenes/GameScene.js';
import { NetworkClient } from './network/client.js';

const config = {
    type: Phaser.AUTO,
    width: window.innerWidth,
    height: window.innerHeight,
    backgroundColor: '#1a4d1a',
    parent: 'game-container',
    scale: {
        mode: Phaser.Scale.FIT,
        autoCenter: Phaser.Scale.CENTER_BOTH
    },
    physics: {
        default: 'arcade',
        arcade: {
            gravity: { y: 0 },
            debug: false
        }
    },
    scene: [MainMenuScene, GameScene]
};

export async function startGame() {
    console.log('启动PVZ迷宫游戏...');

    // 创建网络客户端
    const networkClient = new NetworkClient('ws://localhost:8080');

    try {
        // 连接到服务器
        console.log('连接到服务器...');
        await networkClient.connect();
        console.log('已连接到服务器');

        // 创建Phaser游戏实例
        const game = new Phaser.Game(config);

        // 将网络客户端传递给主菜单场景
        game.scene.start('MainMenuScene', { networkClient });

        return game;
    } catch (error) {
        console.error('无法连接到服务器:', error);
        alert('无法连接到服务器。请确保游戏服务器正在运行。');
        throw error;
    }
}
