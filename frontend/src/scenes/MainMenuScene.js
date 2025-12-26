/**
 * @file MainMenuScene.js
 * @brief 主菜单场景
 */

import Phaser from 'phaser';

// 多语言文本
const LANGUAGES = {
    zh: {
        title: 'PVZ 迷宫',
        subtitle: '植物大战僵尸 - 迷宫模式',
        singlePlayer: '单人游戏',
        multiPlayer: '多人游戏',
        instructions: [
            '操作说明:',
            '单人模式: WASD/方向键 移动僵尸, Tab 小地图',
            '多人模式: WASD 戴夫, 方向键/小键盘 僵尸'
        ],
        world: '🌍 语言',
        languageTitle: '选择语言',
        chinese: '中文',
        english: 'English',
        close: '关闭'
    },
    en: {
        title: 'PVZ Maze',
        subtitle: 'Plants vs Zombies - Maze Mode',
        singlePlayer: 'Single Player',
        multiPlayer: 'Multiplayer',
        instructions: [
            'Controls:',
            'Single: WASD/Arrows move zombie, Tab minimap',
            'Multi: WASD Dave, Arrows/Numpad Zombie'
        ],
        world: '🌍 Lang',
        languageTitle: 'Select Language',
        chinese: '中文',
        english: 'English',
        close: 'Close'
    }
};

export class MainMenuScene extends Phaser.Scene {
    constructor() {
        super({ key: 'MainMenuScene' });
        // 默认语言（从localStorage读取或默认中文）
        this.currentLang = localStorage.getItem('pvz_language') || 'zh';
    }

    init(data) {
        this.networkClient = data.networkClient;
    }

    preload() {
        // 加载背景图片（如果有的话）
        // this.load.image('menu_bg', 'assets/images/ui/menu_background.png');
    }

    create() {
        // 调试：检查输入系统是否正常工作
        this.input.on('pointerdown', (pointer) => {
            console.log(`全局点击: x=${pointer.x}, y=${pointer.y}`);
        });

        this.createMainMenu();
    }

    createMainMenu() {
        // 清除现有内容
        this.children.removeAll();

        const centerX = this.cameras.main.width / 2;
        const centerY = this.cameras.main.height / 2;
        const lang = LANGUAGES[this.currentLang];

        // 设置背景
        this.cameras.main.setBackgroundColor('#1a4d1a');

        // 标题
        const title = this.add.text(centerX, centerY - 150, lang.title, {
            fontSize: '64px',
            color: '#ffffff',
            fontStyle: 'bold',
            stroke: '#000000',
            strokeThickness: 6
        });
        title.setOrigin(0.5);

        // 副标题
        const subtitle = this.add.text(centerX, centerY - 80, lang.subtitle, {
            fontSize: '24px',
            color: '#88ff88',
            fontStyle: 'bold'
        });
        subtitle.setOrigin(0.5);

        // 单人游戏按钮
        console.log('创建单人游戏按钮, 位置:', centerX, centerY + 20);
        const singleBtn = this.createButton(centerX, centerY + 20, lang.singlePlayer, () => {
            console.log('单人游戏回调被执行');
            this.startGame(false);
        });
        console.log('单人按钮对象:', singleBtn.hitArea);

        // 多人游戏按钮
        console.log('创建多人游戏按钮, 位置:', centerX, centerY + 90);
        const multiBtn = this.createButton(centerX, centerY + 90, lang.multiPlayer, () => {
            console.log('多人游戏回调被执行');
            this.startGame(true);
        });
        console.log('多人按钮对象:', multiBtn.hitArea);

        // 操作说明
        const instructions = this.add.text(centerX, centerY + 180, lang.instructions, {
            fontSize: '16px',
            color: '#cccccc',
            align: 'center',
            lineSpacing: 8
        });
        instructions.setOrigin(0.5);

        // 左下角语言按钮
        this.createWorldButton();
    }

    createButton(x, y, text, callback) {
        const buttonWidth = 200;
        const buttonHeight = 50;

        console.log(`createButton: 创建按钮 "${text}" 在 (${x}, ${y})`);

        // 使用 Rectangle 作为可交互的按钮背景（最可靠的方式）
        const hitArea = this.add.rectangle(x, y, buttonWidth, buttonHeight, 0x4a7c3f);
        hitArea.setStrokeStyle(3, 0x2d5a27);
        hitArea.setInteractive({ useHandCursor: true });

        console.log(`createButton: "${text}" input enabled:`, hitArea.input ? hitArea.input.enabled : 'no input');

        // 按钮文字
        const buttonText = this.add.text(x, y, text, {
            fontSize: '24px',
            color: '#ffffff',
            fontStyle: 'bold'
        });
        buttonText.setOrigin(0.5);

        // 鼠标悬停效果
        hitArea.on('pointerover', () => {
            console.log(`悬停: ${text}`);
            hitArea.setFillStyle(0x5a9c4f);
            hitArea.setStrokeStyle(3, 0x3d7a37);
        });

        hitArea.on('pointerout', () => {
            hitArea.setFillStyle(0x4a7c3f);
            hitArea.setStrokeStyle(3, 0x2d5a27);
        });

        // 点击事件
        hitArea.on('pointerdown', () => {
            console.log(`按钮点击: ${text}`);
            callback();
        });

        return { hitArea, buttonText };
    }

    startGame(isMultiplayer) {
        console.log(`===== 启动游戏 =====`);
        console.log(`模式: ${isMultiplayer ? '多人' : '单人'}`);
        console.log('networkClient:', this.networkClient);
        console.log('networkClient.connected:', this.networkClient ? this.networkClient.connected : 'N/A');

        // 使用switch方法切换场景（更可靠的方式）
        this.scene.start('GameScene', {
            networkClient: this.networkClient,
            isMultiplayer: isMultiplayer
        });
        this.scene.stop('MainMenuScene');
    }

    /**
     * 创建左下角的语言/世界按钮（小方形图标按钮，与登录界面风格一致）
     */
    createWorldButton() {
        const buttonSize = 50;
        const buttonX = 40;
        const buttonY = this.cameras.main.height - 40;

        // 使用 Rectangle 作为可交互的按钮背景（深色方形，与主按钮风格一致）
        const hitArea = this.add.rectangle(buttonX, buttonY, buttonSize, buttonSize, 0x3a5c35);
        hitArea.setStrokeStyle(3, 0x2d5a27);
        hitArea.setInteractive({ useHandCursor: true });

        // 地球图标（使用文字表情，居中显示）
        const iconText = this.add.text(buttonX, buttonY, '🌐', {
            fontSize: '28px'
        });
        iconText.setOrigin(0.5);

        // 悬停效果（与主按钮悬停效果一致）
        hitArea.on('pointerover', () => {
            hitArea.setFillStyle(0x5a9c4f);
            hitArea.setStrokeStyle(3, 0x3d7a37);
        });

        hitArea.on('pointerout', () => {
            hitArea.setFillStyle(0x3a5c35);
            hitArea.setStrokeStyle(3, 0x2d5a27);
        });

        // 点击打开语言选择
        hitArea.on('pointerdown', () => {
            console.log('语言按钮点击');
            this.showLanguagePopup();
        });
    }

    /**
     * 显示语言选择弹窗
     */
    showLanguagePopup() {
        const lang = LANGUAGES[this.currentLang];
        const centerX = this.cameras.main.width / 2;
        const centerY = this.cameras.main.height / 2;

        // 创建弹窗容器
        this.languagePopup = this.add.container(centerX, centerY);
        this.languagePopup.setDepth(1000);

        // 半透明黑色背景遮罩
        const overlay = this.add.graphics();
        overlay.fillStyle(0x000000, 0.7);
        overlay.fillRect(-centerX, -centerY, this.cameras.main.width, this.cameras.main.height);
        this.languagePopup.add(overlay);

        // 弹窗背景
        const popupWidth = 300;
        const popupHeight = 250;
        const popupBg = this.add.graphics();
        popupBg.fillStyle(0x2d5a27, 1);
        popupBg.fillRoundedRect(-popupWidth / 2, -popupHeight / 2, popupWidth, popupHeight, 15);
        popupBg.lineStyle(4, 0x4a7c3f);
        popupBg.strokeRoundedRect(-popupWidth / 2, -popupHeight / 2, popupWidth, popupHeight, 15);
        this.languagePopup.add(popupBg);

        // 标题
        const title = this.add.text(0, -popupHeight / 2 + 35, lang.languageTitle, {
            fontSize: '28px',
            color: '#ffffff',
            fontStyle: 'bold'
        });
        title.setOrigin(0.5);
        this.languagePopup.add(title);

        // 中文按钮
        this.createPopupButton(0, -20, lang.chinese, this.currentLang === 'zh', () => {
            this.setLanguage('zh');
        });

        // English按钮
        this.createPopupButton(0, 50, lang.english, this.currentLang === 'en', () => {
            this.setLanguage('en');
        });

        // 关闭按钮
        const closeBtn = this.add.text(popupWidth / 2 - 30, -popupHeight / 2 + 15, '✕', {
            fontSize: '24px',
            color: '#ff6666',
            fontStyle: 'bold'
        });
        closeBtn.setOrigin(0.5);
        closeBtn.setInteractive({ useHandCursor: true });
        closeBtn.on('pointerover', () => closeBtn.setColor('#ff9999'));
        closeBtn.on('pointerout', () => closeBtn.setColor('#ff6666'));
        closeBtn.on('pointerdown', () => this.hideLanguagePopup());
        this.languagePopup.add(closeBtn);

        // 点击遮罩背景关闭（使用overlay graphics对象）
        overlay.setInteractive(new Phaser.Geom.Rectangle(-centerX, -centerY, this.cameras.main.width, this.cameras.main.height), Phaser.Geom.Rectangle.Contains);
        overlay.on('pointerdown', (pointer) => {
            // 检查是否点击在弹窗外
            const localPoint = this.languagePopup.getLocalPoint(pointer.x, pointer.y);
            if (Math.abs(localPoint.x) > popupWidth / 2 || Math.abs(localPoint.y) > popupHeight / 2) {
                this.hideLanguagePopup();
            }
        });

        // 入场动画
        this.languagePopup.setScale(0.5);
        this.languagePopup.setAlpha(0);
        this.tweens.add({
            targets: this.languagePopup,
            scale: 1,
            alpha: 1,
            duration: 200,
            ease: 'Back.easeOut'
        });
    }

    /**
     * 创建弹窗内的语言选择按钮
     */
    createPopupButton(x, y, text, isSelected, callback) {
        const buttonWidth = 200;
        const buttonHeight = 45;

        // 按钮背景
        const button = this.add.graphics();
        const bgColor = isSelected ? 0x5a9c4f : 0x4a7c3f;
        button.fillStyle(bgColor, 1);
        button.fillRoundedRect(x - buttonWidth / 2, y - buttonHeight / 2, buttonWidth, buttonHeight, 10);
        button.lineStyle(3, isSelected ? 0x7ac76f : 0x2d5a27);
        button.strokeRoundedRect(x - buttonWidth / 2, y - buttonHeight / 2, buttonWidth, buttonHeight, 10);
        this.languagePopup.add(button);

        // 选中标记
        if (isSelected) {
            const checkMark = this.add.text(x - buttonWidth / 2 + 25, y, '✓', {
                fontSize: '20px',
                color: '#ffffff',
                fontStyle: 'bold'
            });
            checkMark.setOrigin(0.5);
            this.languagePopup.add(checkMark);
        }

        // 按钮文字
        const buttonText = this.add.text(x, y, text, {
            fontSize: '22px',
            color: '#ffffff',
            fontStyle: 'bold'
        });
        buttonText.setOrigin(0.5);
        this.languagePopup.add(buttonText);

        // 使用Zone作为点击区域
        const hitZone = this.add.zone(x, y, buttonWidth, buttonHeight);
        hitZone.setInteractive({ useHandCursor: true });
        hitZone.setDepth(1001);  // 确保在弹窗之上
        this.languagePopup.add(hitZone);

        // 悬停效果
        hitZone.on('pointerover', () => {
            button.clear();
            button.fillStyle(0x6aac5f, 1);
            button.fillRoundedRect(x - buttonWidth / 2, y - buttonHeight / 2, buttonWidth, buttonHeight, 10);
            button.lineStyle(3, 0x7ac76f);
            button.strokeRoundedRect(x - buttonWidth / 2, y - buttonHeight / 2, buttonWidth, buttonHeight, 10);
        });

        hitZone.on('pointerout', () => {
            button.clear();
            const bgColor = isSelected ? 0x5a9c4f : 0x4a7c3f;
            button.fillStyle(bgColor, 1);
            button.fillRoundedRect(x - buttonWidth / 2, y - buttonHeight / 2, buttonWidth, buttonHeight, 10);
            button.lineStyle(3, isSelected ? 0x7ac76f : 0x2d5a27);
            button.strokeRoundedRect(x - buttonWidth / 2, y - buttonHeight / 2, buttonWidth, buttonHeight, 10);
        });

        // 点击事件
        hitZone.on('pointerdown', () => {
            console.log(`语言选择: ${text}`);
            callback();
        });
    }

    /**
     * 隐藏语言选择弹窗
     */
    hideLanguagePopup() {
        if (this.languagePopup) {
            // 退出动画
            this.tweens.add({
                targets: this.languagePopup,
                scale: 0.5,
                alpha: 0,
                duration: 150,
                ease: 'Back.easeIn',
                onComplete: () => {
                    if (this.languagePopup) {
                        this.languagePopup.destroy();
                        this.languagePopup = null;
                    }
                }
            });
        }
    }

    /**
     * 设置语言
     */
    setLanguage(langCode) {
        if (this.currentLang !== langCode) {
            this.currentLang = langCode;
            // 保存到localStorage
            localStorage.setItem('pvz_language', langCode);
            // 关闭弹窗并重新创建菜单
            this.hideLanguagePopup();
            // 延迟重建菜单，等待弹窗关闭动画完成
            this.time.delayedCall(200, () => {
                this.createMainMenu();
            });
        } else {
            this.hideLanguagePopup();
        }
    }
}
