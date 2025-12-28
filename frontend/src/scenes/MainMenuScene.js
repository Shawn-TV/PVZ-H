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
        howToPlay: '玩法介绍',
        howToPlayTitle: '游戏玩法介绍',
        howToPlayContent: [
            '【游戏目标】',
            '僵尸：穿越迷宫，找到出口逃离！',
            '戴夫：种植植物，阻止僵尸逃跑！',
            '',
            '【操作按键】',
            '单人模式：',
            '  WASD/方向键 - 控制僵尸移动',
            '  Ctrl - 使用撑杆跳跃障碍',
            '  Tab/Shift - 打开小地图',
            '',
            '多人模式：',
            '  WASD - 控制戴夫移动',
            '  方向键 - 控制僵尸移动',
            '  Q - 打开种植菜单',
            '  Ctrl - 撑杆跳跃',
            '',
            '【道具系统】',
            '🪣 铁桶 - 增加护甲值，抵御攻击',
            '🏃 撑杆跳 - 可跳过一格障碍物',
            '❤️ 生命药水 - 恢复40%生命值',
            '⚡ 速度药水 - 移动速度提升50%',
            '',
            '【植物介绍】',
            '🌱 豌豆射手 - 向僵尸发射豌豆',
            '🌱 双发射手 - 一次发射两颗豌豆',
            '🥜 坚果墙 - 阻挡僵尸前进',
            '🍒 樱桃炸弹 - 范围爆炸秒杀僵尸'
        ],
        language: '语言',
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
        howToPlay: 'How to Play',
        howToPlayTitle: 'How to Play',
        howToPlayContent: [
            '【OBJECTIVE】',
            'Zombie: Navigate the maze and escape!',
            'Dave: Plant defenses to stop the zombie!',
            '',
            '【CONTROLS】',
            'Single Player:',
            '  WASD/Arrows - Move zombie',
            '  Ctrl - Pole vault jump',
            '  Tab/Shift - Open minimap',
            '',
            'Multiplayer:',
            '  WASD - Move Dave',
            '  Arrows - Move zombie',
            '  Q - Open plant menu',
            '  Ctrl - Pole vault jump',
            '',
            '【ITEMS】',
            '🪣 Bucket - Adds armor protection',
            '🏃 Pole Vault - Jump over one obstacle',
            '❤️ Health Potion - Restore 40% HP',
            '⚡ Speed Potion - 50% speed boost',
            '',
            '【PLANTS】',
            '🌱 Peashooter - Shoots peas at zombies',
            '🌱 Repeater - Fires two peas at once',
            '🥜 Wall-nut - Blocks zombie movement',
            '🍒 Cherry Bomb - Instant kill explosion'
        ],
        language: 'Lang',
        languageTitle: 'Select Language',
        chinese: '中文',
        english: 'English',
        close: 'Close'
    }
};

export class MainMenuScene extends Phaser.Scene {
    constructor() {
        super({ key: 'MainMenuScene' });
        this.currentLang = localStorage.getItem('pvz_language') || 'zh';
    }

    init(data) {
        this.networkClient = data.networkClient;
    }

    preload() {
        // 加载背景图片
        this.load.image('menu_background', 'assets/images/ui/menu_background.png');

        // 处理加载错误
        this.load.on('loaderror', (file) => {
            console.error('加载失败:', file.key, file.url);
        });
    }

    create() {
        // 检查背景图片是否加载成功
        if (this.textures.exists('menu_background')) {
            console.log('背景图片加载成功');
        } else {
            console.warn('背景图片未加载，尝试重新加载...');
        }

        this.createMainMenu();
    }

    createMainMenu() {
        // 清除现有内容
        this.children.removeAll();

        const centerX = this.cameras.main.width / 2;
        const centerY = this.cameras.main.height / 2;
        const screenWidth = this.cameras.main.width;
        const screenHeight = this.cameras.main.height;
        const lang = LANGUAGES[this.currentLang];

        // 先设置纯色背景作为底层（作为图片加载失败时的备用）
        this.cameras.main.setBackgroundColor('#1a4d1a');

        // 设置背景图片（图层0）- 覆盖整个屏幕
        if (this.textures.exists('menu_background')) {
            const bg = this.add.image(0, 0, 'menu_background');
            bg.setOrigin(0, 0);  // 设置原点为左上角
            // 缩放背景以覆盖整个屏幕
            const scaleX = screenWidth / bg.width;
            const scaleY = screenHeight / bg.height;
            const scale = Math.max(scaleX, scaleY);
            bg.setScale(scale);
            // 居中调整
            bg.setPosition(
                (screenWidth - bg.width * scale) / 2,
                (screenHeight - bg.height * scale) / 2
            );
            bg.setDepth(-1);  // 使用负数确保在最底层
        } else {
            console.warn('背景图片纹理不存在，使用纯色背景');
        }

        // 标题（图层10）
        const title = this.add.text(centerX, centerY - 180, lang.title, {
            fontSize: '64px',
            color: '#ffffff',
            fontStyle: 'bold',
            stroke: '#000000',
            strokeThickness: 6
        });
        title.setOrigin(0.5);
        title.setDepth(10);

        // 副标题（图层10）
        const subtitle = this.add.text(centerX, centerY - 110, lang.subtitle, {
            fontSize: '24px',
            color: '#88ff88',
            fontStyle: 'bold',
            stroke: '#000000',
            strokeThickness: 3
        });
        subtitle.setOrigin(0.5);
        subtitle.setDepth(10);

        // 单人游戏按钮
        this.createMenuButton(centerX, centerY - 20, lang.singlePlayer, () => {
            this.startGame(false);
        });

        // 多人游戏按钮
        this.createMenuButton(centerX, centerY + 50, lang.multiPlayer, () => {
            this.startGame(true);
        });

        // 玩法介绍按钮
        this.createMenuButton(centerX, centerY + 120, lang.howToPlay, () => {
            this.showHowToPlayPopup();
        });

        // 左下角语言按钮
        this.createLanguageButton();
    }

    /**
     * 创建菜单按钮 - 与多人游戏按钮完全相同的实现方式
     */
    createMenuButton(x, y, text, callback) {
        const buttonWidth = 200;
        const buttonHeight = 50;

        // 按钮背景 - Rectangle（图层10）
        const bg = this.add.rectangle(x, y, buttonWidth, buttonHeight, 0x4a7c3f);
        bg.setStrokeStyle(3, 0x2d5a27);
        bg.setInteractive({ useHandCursor: true });
        bg.setDepth(10);

        // 按钮文字（图层11）
        const label = this.add.text(x, y, text, {
            fontSize: '24px',
            color: '#ffffff',
            fontStyle: 'bold'
        });
        label.setOrigin(0.5);
        label.setDepth(11);

        // 悬停效果
        bg.on('pointerover', () => {
            bg.setFillStyle(0x5a9c4f);
            bg.setStrokeStyle(3, 0x3d7a37);
        });

        bg.on('pointerout', () => {
            bg.setFillStyle(0x4a7c3f);
            bg.setStrokeStyle(3, 0x2d5a27);
        });

        // 点击事件 - 与多人游戏按钮完全相同
        bg.on('pointerdown', () => {
            callback();
        });
    }

    /**
     * 创建左下角语言按钮 - 使用与菜单按钮相同的模式
     */
    createLanguageButton() {
        const lang = LANGUAGES[this.currentLang];
        const buttonX = 60;
        const buttonY = this.cameras.main.height - 40;

        // 按钮背景 - 与其他按钮相同的创建方式（图层10）
        const bg = this.add.rectangle(buttonX, buttonY, 80, 40, 0x3a5c35);
        bg.setStrokeStyle(2, 0x2d5a27);
        bg.setInteractive({ useHandCursor: true });
        bg.setDepth(10);

        // 按钮文字（图层11）
        const label = this.add.text(buttonX, buttonY, '🌐 ' + lang.language, {
            fontSize: '14px',
            color: '#ffffff'
        });
        label.setOrigin(0.5);
        label.setDepth(11);

        // 悬停效果
        bg.on('pointerover', () => {
            bg.setFillStyle(0x5a9c4f);
        });

        bg.on('pointerout', () => {
            bg.setFillStyle(0x3a5c35);
        });

        // 点击事件 - 与多人游戏按钮完全相同的模式
        bg.on('pointerdown', () => {
            this.showLanguagePopup();
        });
    }

    startGame(isMultiplayer) {
        this.scene.start('GameScene', {
            networkClient: this.networkClient,
            isMultiplayer: isMultiplayer
        });
        this.scene.stop('MainMenuScene');
    }

    /**
     * 显示玩法介绍弹窗
     */
    showHowToPlayPopup() {
        const lang = LANGUAGES[this.currentLang];
        const centerX = this.cameras.main.width / 2;
        const centerY = this.cameras.main.height / 2;

        // 创建弹窗容器
        this.howToPlayPopup = this.add.container(centerX, centerY);
        this.howToPlayPopup.setDepth(1000);

        // 半透明黑色背景遮罩
        const overlay = this.add.rectangle(0, 0, this.cameras.main.width, this.cameras.main.height, 0x000000, 0.8);
        overlay.setInteractive();
        overlay.on('pointerdown', () => this.hideHowToPlayPopup());
        this.howToPlayPopup.add(overlay);

        // 弹窗背景
        const popupWidth = 500;
        const popupHeight = 550;
        const popupBg = this.add.rectangle(0, 0, popupWidth, popupHeight, 0x2d5a27);
        popupBg.setStrokeStyle(4, 0x4a7c3f);
        popupBg.setInteractive(); // 阻止点击穿透
        this.howToPlayPopup.add(popupBg);

        // 标题
        const title = this.add.text(0, -popupHeight / 2 + 40, lang.howToPlayTitle, {
            fontSize: '28px',
            color: '#ffffff',
            fontStyle: 'bold'
        });
        title.setOrigin(0.5);
        this.howToPlayPopup.add(title);

        // 内容
        const content = this.add.text(0, 20, lang.howToPlayContent.join('\n'), {
            fontSize: '14px',
            color: '#88ff88',
            align: 'left',
            lineSpacing: 6
        });
        content.setOrigin(0.5);
        this.howToPlayPopup.add(content);

        // 关闭按钮
        const closeBtn = this.add.rectangle(0, popupHeight / 2 - 40, 120, 40, 0x4a7c3f);
        closeBtn.setStrokeStyle(2, 0x2d5a27);
        closeBtn.setInteractive({ useHandCursor: true });
        closeBtn.on('pointerover', () => closeBtn.setFillStyle(0x5a9c4f));
        closeBtn.on('pointerout', () => closeBtn.setFillStyle(0x4a7c3f));
        closeBtn.on('pointerdown', () => this.hideHowToPlayPopup());
        this.howToPlayPopup.add(closeBtn);

        const closeBtnText = this.add.text(0, popupHeight / 2 - 40, lang.close, {
            fontSize: '18px',
            color: '#ffffff',
            fontStyle: 'bold'
        });
        closeBtnText.setOrigin(0.5);
        this.howToPlayPopup.add(closeBtnText);

        // 入场动画
        this.howToPlayPopup.setScale(0.8);
        this.howToPlayPopup.setAlpha(0);
        this.tweens.add({
            targets: this.howToPlayPopup,
            scale: 1,
            alpha: 1,
            duration: 200,
            ease: 'Back.easeOut'
        });
    }

    hideHowToPlayPopup() {
        if (this.howToPlayPopup) {
            this.tweens.add({
                targets: this.howToPlayPopup,
                scale: 0.8,
                alpha: 0,
                duration: 150,
                ease: 'Back.easeIn',
                onComplete: () => {
                    this.howToPlayPopup.destroy();
                    this.howToPlayPopup = null;
                }
            });
        }
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
        const overlay = this.add.rectangle(0, 0, this.cameras.main.width, this.cameras.main.height, 0x000000, 0.8);
        overlay.setInteractive();
        overlay.on('pointerdown', () => this.hideLanguagePopup());
        this.languagePopup.add(overlay);

        // 弹窗背景
        const popupWidth = 300;
        const popupHeight = 220;
        const popupBg = this.add.rectangle(0, 0, popupWidth, popupHeight, 0x2d5a27);
        popupBg.setStrokeStyle(4, 0x4a7c3f);
        popupBg.setInteractive(); // 阻止点击穿透
        this.languagePopup.add(popupBg);

        // 标题
        const title = this.add.text(0, -popupHeight / 2 + 35, lang.languageTitle, {
            fontSize: '24px',
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
        this.createPopupButton(0, 40, lang.english, this.currentLang === 'en', () => {
            this.setLanguage('en');
        });

        // 入场动画
        this.languagePopup.setScale(0.8);
        this.languagePopup.setAlpha(0);
        this.tweens.add({
            targets: this.languagePopup,
            scale: 1,
            alpha: 1,
            duration: 200,
            ease: 'Back.easeOut'
        });
    }

    createPopupButton(x, y, text, isSelected, callback) {
        const buttonWidth = 200;
        const buttonHeight = 40;
        const fillColor = isSelected ? 0x5a9c4f : 0x3a5c35;

        const bg = this.add.rectangle(x, y, buttonWidth, buttonHeight, fillColor);
        bg.setStrokeStyle(2, isSelected ? 0x7cba6f : 0x2d5a27);
        bg.setInteractive({ useHandCursor: true });

        const label = this.add.text(x, y, text + (isSelected ? ' ✓' : ''), {
            fontSize: '18px',
            color: '#ffffff',
            fontStyle: isSelected ? 'bold' : 'normal'
        });
        label.setOrigin(0.5);

        bg.on('pointerover', () => {
            if (!isSelected) bg.setFillStyle(0x4a7c3f);
        });

        bg.on('pointerout', () => {
            if (!isSelected) bg.setFillStyle(0x3a5c35);
        });

        bg.on('pointerdown', () => {
            callback();
        });

        this.languagePopup.add(bg);
        this.languagePopup.add(label);
    }

    hideLanguagePopup() {
        if (this.languagePopup) {
            this.tweens.add({
                targets: this.languagePopup,
                scale: 0.8,
                alpha: 0,
                duration: 150,
                ease: 'Back.easeIn',
                onComplete: () => {
                    this.languagePopup.destroy();
                    this.languagePopup = null;
                }
            });
        }
    }

    setLanguage(langCode) {
        this.currentLang = langCode;
        localStorage.setItem('pvz_language', langCode);
        this.hideLanguagePopup();
        // 延迟重建菜单，等待弹窗关闭动画完成
        this.time.delayedCall(200, () => {
            this.createMainMenu();
        });
    }
}
