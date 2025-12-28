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
    render: {
        pixelArt: false,
        roundPixels: true,  // 防止子像素渲染导致的画面抖动
        antialias: true
    },
    physics: {
        default: 'arcade',
        arcade: {
            gravity: { y: 0 },
            debug: false
        }
    },
    // 不自动启动场景，手动启动以传递数据
    scene: []
};

export async function startGame() {
    // 创建网络客户端
    const networkClient = new NetworkClient('ws://localhost:8080');

    try {
        // 连接到服务器
        await networkClient.connect();

        // 创建Phaser游戏实例
        const game = new Phaser.Game(config);

        // 手动添加场景
        game.scene.add('MainMenuScene', MainMenuScene);
        game.scene.add('GameScene', GameScene);

        // 启动主菜单场景并传递网络客户端
        game.scene.start('MainMenuScene', { networkClient });

        return game;
    } catch (error) {
        console.error('无法连接到服务器:', error);
        alert('无法连接到服务器。请确保游戏服务器正在运行。');
        throw error;
    }
}
