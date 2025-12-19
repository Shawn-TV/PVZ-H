/**
 * @file GameScene.js
 * @brief Phaser游戏主场景
 *
 * 功能：
 * - 渲染迷宫
 * - 渲染所有实体
 * - 摄像机跟随僵尸
 * - 处理输入
 */

export class GameScene extends Phaser.Scene {
    constructor() {
        super({ key: 'GameScene' });

        this.networkClient = null;
        this.maze = null;
        this.entities = new Map(); // entityId -> sprite
        this.zombie = null; // 玩家僵尸
        this.mazeGraphics = null;

        // 输入状态
        this.keys = {};
        this.isAttacking = false;
    }

    init(data) {
        this.networkClient = data.networkClient;
    }

    create() {
        console.log('GameScene创建');

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

            // 更新精灵位置和状态
            sprite.setPosition(entityData.x, entityData.y);
            sprite.setData('entityData', entityData);

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
            }
        });
    }

    createEntitySprite(entityData) {
        let sprite;

        // 根据类型创建不同的精灵
        const graphics = this.add.graphics();

        switch (entityData.type) {
            case 'zombie':
                // 僵尸 - 绿色矩形
                graphics.fillStyle(0x00ff00, 1.0);
                graphics.fillRect(-16, -16, 32, 32);
                sprite = this.add.sprite(entityData.x, entityData.y, graphics.generateTexture('zombie'));
                graphics.destroy();
                break;

            case 'dave':
                // 戴夫 - 蓝色圆形
                graphics.fillStyle(0x0000ff, 1.0);
                graphics.fillCircle(0, 0, 20);
                sprite = this.add.sprite(entityData.x, entityData.y, graphics.generateTexture('dave'));
                graphics.destroy();
                break;

            case 'plant':
                // 植物 - 根据类型选择颜色
                const plantColor = this.getPlantColor(entityData.plantType);
                graphics.fillStyle(plantColor, 1.0);
                graphics.fillCircle(0, 0, 15);
                sprite = this.add.sprite(entityData.x, entityData.y, graphics.generateTexture(`plant_${entityData.id}`));
                graphics.destroy();
                break;

            case 'item':
                // 道具 - 黄色星形
                graphics.fillStyle(0xffff00, 1.0);
                graphics.fillCircle(0, 0, 12);
                sprite = this.add.sprite(entityData.x, entityData.y, graphics.generateTexture(`item_${entityData.id}`));
                graphics.destroy();
                break;

            case 'projectile':
                // 豌豆 - 绿色小圆
                graphics.fillStyle(0x90ee90, 1.0);
                graphics.fillCircle(0, 0, 5);
                sprite = this.add.sprite(entityData.x, entityData.y, graphics.generateTexture(`projectile_${entityData.id}`));
                graphics.destroy();
                break;

            default:
                // 默认 - 白色圆形
                graphics.fillStyle(0xffffff, 1.0);
                graphics.fillCircle(0, 0, 10);
                sprite = this.add.sprite(entityData.x, entityData.y, graphics.generateTexture(`entity_${entityData.id}`));
                graphics.destroy();
                break;
        }

        // 创建生命值条
        sprite.healthBarBg = this.add.graphics();
        sprite.healthBar = this.add.graphics();

        return sprite;
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
