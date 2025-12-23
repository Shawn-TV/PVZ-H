/**
 * @file GameScene.js
 * @brief Phaser游戏主场景
 *
 * 功能：
 * - 渲染迷宫
 * - 渲染所有实体
 * - 摄像机跟随僵尸
 * - 处理输入
 * - 60FPS平滑插值动画
 */

import { InterpolationManager } from '../utils/InterpolationManager.js';

export class GameScene extends Phaser.Scene {
    constructor() {
        super({ key: 'GameScene' });

        this.networkClient = null;
        this.maze = null;
        this.entities = new Map(); // entityId -> sprite
        this.zombie = null; // 玩家僵尸
        this.mazeGraphics = null;

        // 帧插值管理器（用于60FPS平滑动画）
        this.interpolationManager = new InterpolationManager();

        // 输入状态
        this.keys = {};
        this.isAttacking = false;

        // FPS统计
        this.fpsText = null;
        this.lastFpsUpdate = 0;
        this.frameCount = 0;

        // 动画配置
        // 注意：撑杆跳僵尸的动画需要与实际移动速度匹配
        // 正常僵尸速度为基准，撑杆跳僵尸速度是1.5倍
        // 降低帧率使动画与实际移动距离匹配
        this.animationConfigs = {
            zombie_walk: { frameRate: 10, repeat: -1 },
            dave_walk: { frameRate: 10, repeat: -1 },
            pole_vaulter_walk: { frameRate: 8, repeat: -1 },   // 降低到8fps，匹配正常速度
            pole_vaulter_jump: { frameRate: 4, repeat: 0 },    // 大幅降低到4fps，让跳跃动画更慢
            pole_vaulter_run: { frameRate: 10, repeat: -1 }    // 降低到10fps，与正常僵尸相似
        };
    }

    init(data) {
        this.networkClient = data.networkClient;
    }

    preload() {
        // 加载精灵表
        // Dave走路动画 (8列 x 6行, 每帧128x128)
        this.load.spritesheet('dave_walk', 'assets/images/dave/dave_walk_spritesheet.png', {
            frameWidth: 128,
            frameHeight: 128
        });

        // 普通僵尸走路动画 (8列 x 6行, 每帧约128x128)
        this.load.spritesheet('zombie_walk', 'assets/images/zombies/zombie_walk_spritesheet.png', {
            frameWidth: 128,
            frameHeight: 170
        });

        // 撑杆跳僵尸走路动画
        this.load.spritesheet('pole_vaulter_walk', 'assets/images/zombies/pole_vaulter_walk_spritesheet.png', {
            frameWidth: 128,
            frameHeight: 170
        });

        // 撑杆跳僵尸跳跃动画 (4列 x 10行, 每帧约256x256)
        this.load.spritesheet('pole_vaulter_jump', 'assets/images/zombies/pole_vaulter_jump_spritesheet.png', {
            frameWidth: 512,
            frameHeight: 205
        });

        // 撑杆跳僵尸跑步动画
        this.load.spritesheet('pole_vaulter_run', 'assets/images/zombies/pole_vaulter_run_spritesheet.png', {
            frameWidth: 128,
            frameHeight: 170
        });
    }

    create() {
        console.log('GameScene创建');

        // 创建动画
        this.createAnimations();

        // 创建迷宫图层
        this.mazeGraphics = this.add.graphics();

        // 设置摄像机范围（初始）
        this.cameras.main.setBounds(0, 0, 1680, 2480);
        this.cameras.main.setZoom(1.0);

        // 设置输入
        this.setupInput();

        // 注册网络消息处理器
        if (this.networkClient) {
            this.networkClient.on('MAZE_INIT', (data) => this.handleMazeInit(data));
            this.networkClient.on('ENTITIES_UPDATE', (data) => this.handleEntitiesUpdate(data));
            this.networkClient.on('GAME_STATE', (data) => this.handleGameState(data));
            this.networkClient.on('GAME_OVER', (data) => this.handleGameOver(data));
        }

        // 显示加载提示
        this.add.text(400, 300, '连接到服务器...', {
            fontSize: '32px',
            color: '#ffffff'
        }).setScrollFactor(0);

        // 添加FPS显示（右上角）
        this.fpsText = this.add.text(10, 10, 'FPS: 0', {
            fontSize: '16px',
            color: '#00ff00',
            backgroundColor: '#000000',
            padding: { x: 5, y: 5 }
        });
        this.fpsText.setScrollFactor(0);
        this.fpsText.setDepth(1000);
    }

    createAnimations() {
        // 检查精灵表是否已加载，如果加载失败则跳过动画创建
        const textureKeys = ['dave_walk', 'zombie_walk', 'pole_vaulter_walk', 'pole_vaulter_jump', 'pole_vaulter_run'];

        textureKeys.forEach(key => {
            if (!this.textures.exists(key)) {
                console.warn(`Texture ${key} not loaded, skipping animation creation`);
                return;
            }

            const config = this.animationConfigs[key];
            if (!config) return;

            // 获取精灵表的帧数
            const texture = this.textures.get(key);
            const frameCount = texture.frameTotal - 1; // 减去__BASE帧

            if (frameCount > 0) {
                this.anims.create({
                    key: key,
                    frames: this.anims.generateFrameNumbers(key, {
                        start: 0,
                        end: frameCount - 1
                    }),
                    frameRate: config.frameRate,
                    repeat: config.repeat
                });
                console.log(`Created animation: ${key} with ${frameCount} frames`);
            }
        });
    }

    setupInput() {
        // WASD键
        this.keys.W = this.input.keyboard.addKey(Phaser.Input.Keyboard.KeyCodes.W);
        this.keys.A = this.input.keyboard.addKey(Phaser.Input.Keyboard.KeyCodes.A);
        this.keys.S = this.input.keyboard.addKey(Phaser.Input.Keyboard.KeyCodes.S);
        this.keys.D = this.input.keyboard.addKey(Phaser.Input.Keyboard.KeyCodes.D);

        // 方向键
        this.cursors = this.input.keyboard.createCursorKeys();

        // Ctrl键（撑杆跳）
        this.keys.CTRL = this.input.keyboard.addKey(Phaser.Input.Keyboard.KeyCodes.CTRL);

        // 鼠标攻击
        this.input.on('pointerdown', () => {
            this.isAttacking = true;
            this.networkClient?.send('ATTACK', {});
        });

        this.input.on('pointerup', () => {
            this.isAttacking = false;
        });

        // 空格键攻击
        this.keys.SPACE = this.input.keyboard.addKey(Phaser.Input.Keyboard.KeyCodes.SPACE);
    }

    handleMazeInit(maze) {
        console.log('收到迷宫数据:', maze);
        this.maze = maze;

        // 绘制迷宫
        this.renderMaze();

        // 更新摄像机边界
        this.cameras.main.setBounds(0, 0, maze.pixelWidth, maze.pixelHeight);
    }

    renderMaze() {
        if (!this.maze) return;

        this.mazeGraphics.clear();

        const cellSize = this.maze.cellSize;

        // 绘制地板（草地）- 使用PVZ风格的绿色格子
        for (let y = 0; y < this.maze.gridHeight; y++) {
            for (let x = 0; x < this.maze.gridWidth; x++) {
                const px = x * cellSize;
                const py = y * cellSize;

                // 棋盘格效果
                const isEven = (x + y) % 2 === 0;
                this.mazeGraphics.fillStyle(isEven ? 0x4a7c2f : 0x568c3a, 1.0);
                this.mazeGraphics.fillRect(px, py, cellSize, cellSize);
            }
        }

        // 绘制墙壁
        this.mazeGraphics.lineStyle(2, 0x2d4a1f, 1.0);
        this.mazeGraphics.fillStyle(0x3d5a2f, 1.0);

        this.maze.walls.forEach(wall => {
            const px = wall.x * cellSize;
            const py = wall.y * cellSize;

            // 填充墙壁
            this.mazeGraphics.fillRect(px, py, cellSize, cellSize);

            // 绘制边框
            this.mazeGraphics.strokeRect(px, py, cellSize, cellSize);
        });

        // 绘制入口（绿色）
        this.mazeGraphics.fillStyle(0x00ff00, 0.5);
        this.mazeGraphics.fillCircle(
            this.maze.entrance.x,
            this.maze.entrance.y,
            cellSize / 2
        );

        // 绘制出口（红色）
        this.mazeGraphics.fillStyle(0xff0000, 0.5);
        this.mazeGraphics.fillCircle(
            this.maze.exit.x,
            this.maze.exit.y,
            cellSize / 2
        );
    }

    handleEntitiesUpdate(entities) {
        // 更新所有实体
        const currentEntityIds = new Set();

        entities.forEach(entityData => {
            currentEntityIds.add(entityData.id);

            // 更新插值管理器的数据（用于60FPS平滑插值）
            this.interpolationManager.updateEntity(entityData.id, entityData);

            let sprite = this.entities.get(entityData.id);

            if (!sprite) {
                // 创建新的精灵
                sprite = this.createEntitySprite(entityData);
                this.entities.set(entityData.id, sprite);

                // 如果是僵尸，设置摄像机跟随
                if (entityData.type === 'zombie') {
                    this.zombie = sprite;
                    this.cameras.main.startFollow(sprite, true, 0.1, 0.1);
                }
            }

            // 注意：不在这里直接设置位置，而是在update循环中使用插值位置
            // 保存实体数据用于其他用途（如生命值显示）
            sprite.setData('entityData', entityData);

            // 更新动画状态
            this.updateEntityAnimation(sprite, entityData);

            // 更新生命值显示
            this.updateHealthBar(sprite, entityData);
        });

        // 移除不再存在的实体
        this.entities.forEach((sprite, id) => {
            if (!currentEntityIds.has(id)) {
                sprite.destroy();
                if (sprite.healthBar) {
                    sprite.healthBar.destroy();
                    sprite.healthBarBg.destroy();
                }
                this.entities.delete(id);
                // 从插值管理器中移除
                this.interpolationManager.removeEntity(id);
            }
        });
    }

    createEntitySprite(entityData) {
        let sprite;

        switch (entityData.type) {
            case 'zombie':
                // 检查是否是撑杆跳僵尸
                const isPoleVaulter = entityData.zombieForm === 'pole_vaulter' || entityData.hasPoleVault;

                if (isPoleVaulter && this.textures.exists('pole_vaulter_walk')) {
                    sprite = this.add.sprite(entityData.x, entityData.y, 'pole_vaulter_walk');
                    sprite.setScale(0.5);
                    sprite.play('pole_vaulter_walk');
                    sprite.setData('isPoleVaulter', true);
                } else if (this.textures.exists('zombie_walk')) {
                    sprite = this.add.sprite(entityData.x, entityData.y, 'zombie_walk');
                    sprite.setScale(0.5);
                    sprite.play('zombie_walk');
                } else {
                    // 后备：使用简单图形
                    const graphics = this.add.graphics();
                    graphics.fillStyle(0x00ff00, 1.0);
                    graphics.fillRect(-16, -16, 32, 32);
                    sprite = this.add.sprite(entityData.x, entityData.y, graphics.generateTexture('zombie_fallback'));
                    graphics.destroy();
                }
                break;

            case 'dave':
                if (this.textures.exists('dave_walk')) {
                    sprite = this.add.sprite(entityData.x, entityData.y, 'dave_walk');
                    sprite.setScale(0.6);
                    sprite.play('dave_walk');
                } else {
                    // 后备：使用简单图形
                    const graphics = this.add.graphics();
                    graphics.fillStyle(0x0000ff, 1.0);
                    graphics.fillCircle(0, 0, 20);
                    sprite = this.add.sprite(entityData.x, entityData.y, graphics.generateTexture('dave_fallback'));
                    graphics.destroy();
                }
                break;

            case 'plant':
                // 植物 - 使用简单图形（后续可以添加植物精灵表）
                const plantColor = this.getPlantColor(entityData.plantType);
                const plantGraphics = this.add.graphics();
                plantGraphics.fillStyle(plantColor, 1.0);
                plantGraphics.fillCircle(0, 0, 15);
                sprite = this.add.sprite(entityData.x, entityData.y, plantGraphics.generateTexture(`plant_${entityData.id}`));
                plantGraphics.destroy();
                break;

            case 'item':
                // 道具
                const itemGraphics = this.add.graphics();
                itemGraphics.fillStyle(0xffff00, 1.0);
                itemGraphics.fillCircle(0, 0, 12);
                sprite = this.add.sprite(entityData.x, entityData.y, itemGraphics.generateTexture(`item_${entityData.id}`));
                itemGraphics.destroy();
                break;

            case 'projectile':
                // 豌豆
                const projGraphics = this.add.graphics();
                projGraphics.fillStyle(0x90ee90, 1.0);
                projGraphics.fillCircle(0, 0, 5);
                sprite = this.add.sprite(entityData.x, entityData.y, projGraphics.generateTexture(`projectile_${entityData.id}`));
                projGraphics.destroy();
                break;

            default:
                // 默认
                const defaultGraphics = this.add.graphics();
                defaultGraphics.fillStyle(0xffffff, 1.0);
                defaultGraphics.fillCircle(0, 0, 10);
                sprite = this.add.sprite(entityData.x, entityData.y, defaultGraphics.generateTexture(`entity_${entityData.id}`));
                defaultGraphics.destroy();
                break;
        }

        // 创建生命值条
        sprite.healthBarBg = this.add.graphics();
        sprite.healthBar = this.add.graphics();

        return sprite;
    }

    // 更新实体动画状态
    updateEntityAnimation(sprite, entityData) {
        if (!sprite || !entityData) return;

        const isPoleVaulter = sprite.getData('isPoleVaulter');

        // 处理撑杆跳僵尸的动画切换
        if (isPoleVaulter && entityData.type === 'zombie') {
            // 检查是否正在跳跃（根据后端状态）
            const isJumping = entityData.isJumping || entityData.state === 'jumping';
            // 检查是否正在跑动（后端状态为RUNNING）
            const isRunning = entityData.state === 'running' || entityData.state === 'RUNNING';
            // 检查是否还有撑杆跳能力
            const hasPoleVault = entityData.hasPoleVault !== false && entityData.form === 'pole_vaulter';

            if (isJumping) {
                // 正在跳跃 - 播放跳跃动画（速度已降低到4fps）
                if (sprite.anims.currentAnim?.key !== 'pole_vaulter_jump') {
                    if (this.anims.exists('pole_vaulter_jump')) {
                        sprite.play('pole_vaulter_jump');
                        sprite.setScale(0.35); // 跳跃帧更大，需要更多缩小
                    }
                }
            } else if (!hasPoleVault) {
                // 已经失去撑杆跳能力，使用普通僵尸动画
                if (sprite.anims.currentAnim?.key !== 'zombie_walk') {
                    if (this.anims.exists('zombie_walk')) {
                        sprite.play('zombie_walk');
                        sprite.setScale(0.5);
                        sprite.setData('isPoleVaulter', false);
                    }
                }
            } else if (isRunning) {
                // 正在跑动 - 播放跑动动画（与正常僵尸速度相似的10fps）
                if (sprite.anims.currentAnim?.key !== 'pole_vaulter_run') {
                    if (this.anims.exists('pole_vaulter_run')) {
                        sprite.play('pole_vaulter_run');
                        sprite.setScale(0.5);
                    }
                }
            } else {
                // 普通行走（8fps，与正常僵尸速度匹配）
                if (sprite.anims.currentAnim?.key !== 'pole_vaulter_walk') {
                    if (this.anims.exists('pole_vaulter_walk')) {
                        sprite.play('pole_vaulter_walk');
                        sprite.setScale(0.5);
                    }
                }
            }
        }

        // 翻转精灵以面向移动方向
        if (entityData.direction) {
            if (entityData.direction.x < 0) {
                sprite.setFlipX(true);
            } else if (entityData.direction.x > 0) {
                sprite.setFlipX(false);
            }
        }
    }

    getPlantColor(plantType) {
        switch (plantType) {
            case 'pea_shooter': return 0x00ff00;
            case 'double_pea_shooter': return 0x00aa00;
            case 'cherry_bomb': return 0xff0000;
            case 'wall_nut': return 0x8b4513;
            default: return 0x00ff00;
        }
    }

    updateHealthBar(sprite, entityData) {
        if (!sprite.healthBar) return;

        const { health, maxHealth } = entityData;
        const barWidth = 40;
        const barHeight = 4;
        const x = entityData.x - barWidth / 2;
        const y = entityData.y - 25;

        // 背景
        sprite.healthBarBg.clear();
        sprite.healthBarBg.fillStyle(0x000000, 0.5);
        sprite.healthBarBg.fillRect(x, y, barWidth, barHeight);

        // 生命值
        sprite.healthBar.clear();
        const healthPercent = Math.max(0, health / maxHealth);
        const color = healthPercent > 0.5 ? 0x00ff00 : healthPercent > 0.25 ? 0xffff00 : 0xff0000;
        sprite.healthBar.fillStyle(color, 1.0);
        sprite.healthBar.fillRect(x, y, barWidth * healthPercent, barHeight);
    }

    handleGameState(data) {
        console.log('游戏状态:', data);

        // 根据游戏状态显示UI
        if (data.status === 'win') {
            this.showGameOver('胜利！', 0x00ff00);
        } else if (data.status === 'lose') {
            this.showGameOver('失败！', 0xff0000);
        }
    }

    handleGameOver(data) {
        console.log('游戏结束:', data);
    }

    showGameOver(text, color) {
        const bg = this.add.graphics();
        bg.fillStyle(0x000000, 0.7);
        bg.fillRect(0, 0, 800, 600);
        bg.setScrollFactor(0);

        const gameOverText = this.add.text(400, 300, text, {
            fontSize: '64px',
            color: `#${color.toString(16)}`
        });
        gameOverText.setOrigin(0.5);
        gameOverText.setScrollFactor(0);
    }

    update(time, delta) {
        // 应用插值位置到所有实体（实现60FPS平滑动画）
        this.entities.forEach((sprite, id) => {
            const interpolatedPos = this.interpolationManager.getInterpolatedPosition(id);
            if (interpolatedPos) {
                sprite.setPosition(interpolatedPos.x, interpolatedPos.y);
            }
        });

        // 更新FPS显示
        this.frameCount++;
        if (time - this.lastFpsUpdate >= 1000) {
            const fps = Math.round(this.frameCount * 1000 / (time - this.lastFpsUpdate));
            const stats = this.interpolationManager.getStats();
            this.fpsText.setText(
                `FPS: ${fps} | Entities: ${stats.entityCount} | Latency: ${Math.round(stats.averageLatency)}ms`
            );
            this.frameCount = 0;
            this.lastFpsUpdate = time;
        }

        // 处理输入
        this.handleInput();
    }

    handleInput() {
        if (!this.networkClient) return;

        // 检测移动输入
        let moving = false;

        if (this.keys.W.isDown || this.cursors.up.isDown) {
            this.networkClient.send('MOVE_UP', {});
            moving = true;
        } else if (this.keys.S.isDown || this.cursors.down.isDown) {
            this.networkClient.send('MOVE_DOWN', {});
            moving = true;
        }

        if (this.keys.A.isDown || this.cursors.left.isDown) {
            this.networkClient.send('MOVE_LEFT', {});
            moving = true;
        } else if (this.keys.D.isDown || this.cursors.right.isDown) {
            this.networkClient.send('MOVE_RIGHT', {});
            moving = true;
        }

        if (!moving) {
            this.networkClient.send('STOP_MOVE', {});
        }

        // 撑杆跳
        if (Phaser.Input.Keyboard.JustDown(this.keys.CTRL)) {
            this.networkClient.send('POLE_VAULT', {});
        }

        // 空格攻击
        if (this.keys.SPACE.isDown && !this.isAttacking) {
            this.isAttacking = true;
            this.networkClient.send('ATTACK', {});
        } else if (!this.keys.SPACE.isDown && this.isAttacking) {
            this.isAttacking = false;
        }
    }
}
