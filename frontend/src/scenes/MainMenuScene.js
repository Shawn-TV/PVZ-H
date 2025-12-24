/**
 * @file MainMenuScene.js
 * @brief 主菜单场景
 */

import Phaser from 'phaser';

export class MainMenuScene extends Phaser.Scene {
    constructor() {
        super({ key: 'MainMenuScene' });
    }

    init(data) {
        this.networkClient = data.networkClient;
    }

    preload() {
        // 加载背景图片（如果有的话）
        // this.load.image('menu_bg', 'assets/images/ui/menu_background.png');
    }

    create() {
        const centerX = this.cameras.main.width / 2;
        const centerY = this.cameras.main.height / 2;

        // 设置背景
        this.cameras.main.setBackgroundColor('#1a4d1a');

        // 标题
        const title = this.add.text(centerX, centerY - 150, 'PVZ 迷宫', {
            fontSize: '64px',
            color: '#ffffff',
            fontStyle: 'bold',
            stroke: '#000000',
            strokeThickness: 6
        });
        title.setOrigin(0.5);

        // 副标题
        const subtitle = this.add.text(centerX, centerY - 80, '植物大战僵尸 - 迷宫模式', {
            fontSize: '24px',
            color: '#88ff88',
            fontStyle: 'bold'
        });
        subtitle.setOrigin(0.5);

        // 单人游戏按钮
        this.createButton(centerX, centerY + 20, '单人游戏', () => {
            this.startGame(false);
        });

        // 多人游戏按钮
        this.createButton(centerX, centerY + 90, '多人游戏', () => {
            this.startGame(true);
        });

        // 操作说明
        const instructions = this.add.text(centerX, centerY + 180, [
            '操作说明:',
            '单人模式: WASD/方向键 移动僵尸, Tab 小地图',
            '多人模式: WASD 戴夫, 方向键/小键盘 僵尸'
        ], {
            fontSize: '16px',
            color: '#cccccc',
            align: 'center',
            lineSpacing: 8
        });
        instructions.setOrigin(0.5);
    }

    createButton(x, y, text, callback) {
        const buttonWidth = 200;
        const buttonHeight = 50;

        // 按钮背景
        const button = this.add.graphics();
        button.fillStyle(0x4a7c3f, 1);
        button.fillRoundedRect(x - buttonWidth / 2, y - buttonHeight / 2, buttonWidth, buttonHeight, 10);
        button.lineStyle(3, 0x2d5a27);
        button.strokeRoundedRect(x - buttonWidth / 2, y - buttonHeight / 2, buttonWidth, buttonHeight, 10);

        // 按钮文字
        const buttonText = this.add.text(x, y, text, {
            fontSize: '24px',
            color: '#ffffff',
            fontStyle: 'bold'
        });
        buttonText.setOrigin(0.5);

        // 创建交互区域（设置透明度使其可见但几乎不可见）
        const hitArea = this.add.rectangle(x, y, buttonWidth, buttonHeight, 0xffffff, 0.001)
            .setInteractive({ useHandCursor: true });

        // 鼠标悬停效果
        hitArea.on('pointerover', () => {
            button.clear();
            button.fillStyle(0x5a9c4f, 1);
            button.fillRoundedRect(x - buttonWidth / 2, y - buttonHeight / 2, buttonWidth, buttonHeight, 10);
            button.lineStyle(3, 0x3d7a37);
            button.strokeRoundedRect(x - buttonWidth / 2, y - buttonHeight / 2, buttonWidth, buttonHeight, 10);
        });

        hitArea.on('pointerout', () => {
            button.clear();
            button.fillStyle(0x4a7c3f, 1);
            button.fillRoundedRect(x - buttonWidth / 2, y - buttonHeight / 2, buttonWidth, buttonHeight, 10);
            button.lineStyle(3, 0x2d5a27);
            button.strokeRoundedRect(x - buttonWidth / 2, y - buttonHeight / 2, buttonWidth, buttonHeight, 10);
        });

        // 点击事件 - 使用 pointerup 更可靠
        hitArea.on('pointerup', () => {
            console.log(`按钮点击: ${text}`);
            callback();
        });

        return { button, buttonText, hitArea };
    }

    startGame(isMultiplayer) {
        console.log(`开始${isMultiplayer ? '多人' : '单人'}游戏`);
        console.log('networkClient:', this.networkClient);

        // 停止当前场景并启动游戏场景
        this.scene.stop('MainMenuScene');
        this.scene.start('GameScene', {
            networkClient: this.networkClient,
            isMultiplayer: isMultiplayer
        });
    }
}
