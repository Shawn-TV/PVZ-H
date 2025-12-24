/**
 * @file GameScene.js
 * @brief Phaser游戏主场景
 *
 * 功能：
 * - 渲染迷宫（使用草地瓦片）
 * - 渲染所有实体（使用实际精灵）
 * - 摄像机跟随僵尸
 * - 处理输入（WASD和方向键）
 * - 60FPS平滑插值动画
 */

import { InterpolationManager } from '../utils/InterpolationManager.js';

export class GameScene extends Phaser.Scene {
    constructor() {
        super({ key: 'GameScene' });

        this.networkClient = null;
        this.maze = null;
        this.entities = new Map(); // entityId -> sprite
        this.zombieSprite = null; // 唯一的僵尸精灵引用
        this.zombieId = null; // 僵尸的实体ID
        this.daveSprite = null; // 唯一的戴夫精灵引用
        this.daveId = null; // 戴夫的实体ID
        this.mazeGraphics = null;
        this.mazeTiles = []; // 存储迷宫瓦片精灵

        // 帧插值管理器（用于60FPS平滑动画）
        this.interpolationManager = new InterpolationManager();

        // 输入状态
        this.keys = {};
        this.isAttacking = false;
        this.lastMoveDirection = null;

        // 资源加载状态
        this.assetsLoaded = false;

        // 游戏结束状态
        this.gameOverShown = false;
        this.lastGameStatus = 'playing';

        // 撑杆跳冷却
        this.poleVaultCooldown = 0;
    }

    init(data) {
        console.log('===== GameScene.init() 被调用 =====');
        console.log('接收到的数据:', data);
        this.networkClient = data.networkClient;
        // 从主菜单接收游戏模式（默认单人模式）
        this.startAsMultiplayer = data.isMultiplayer || false;
        console.log('startAsMultiplayer 设置为:', this.startAsMultiplayer);

        // 重置所有游戏状态，确保单人/多人模式完全独立
        this.maze = null;
        this.entities = new Map();
        this.zombieSprite = null;
        this.zombieId = null;
        this.daveSprite = null;
        this.daveId = null;
        this.mazeTiles = [];
        this.mazeGraphics = null;
        this.gameOverShown = false;
        this.lastGameStatus = 'playing';
        this.isMultiplayerMode = false;
        this.splitScreenEnabled = false;
        this.daveCamera = null;
        this.zombieCamera = null;
        this.minimapVisible = false;
        this.currentMinimap = null;
        this.seedPacketVisible = false;
        this.selectedPlantIndex = -1;
        this.currentDaveData = null;
        // 重置输入状态
        this.lastMoveDirection = null;
        this.lastDaveMoveDirection = null;
        this.keys = {};
        console.log('游戏状态已重置');
    }

    preload() {
        console.log('开始加载游戏资源...');

        // 加载背景
        this.load.image('lawn', 'assets/images/backgrounds/lawn.png');

        // 加载迷宫瓦片
        this.load.image('grass_dark', 'assets/images/tiles/grass_dark.png');
        this.load.image('grass_light', 'assets/images/tiles/grass_light.png');
        this.load.image('wall', 'assets/images/tiles/wall.png');
        this.load.image('entrance', 'assets/images/tiles/entrance.png');
        this.load.image('exit', 'assets/images/tiles/exit.png');
        this.load.image('item_spawn', 'assets/images/tiles/item_spawn.png');

        // ========== 僵尸精灵表 (原始素材 1024x1024) ==========
        // 普通僵尸行走: 100x139 每帧, 10列, 46帧
        this.load.spritesheet('zombie_walk', 'assets/images/zombies/zombie_walk_spritesheet.png', {
            frameWidth: 100,
            frameHeight: 139,
            endFrame: 45
        });

        // 普通僵尸吃: 100x139 每帧, 10列, 39帧
        this.load.spritesheet('zombie_eat', 'assets/images/zombies/zombie_eat_spritesheet.png', {
            frameWidth: 100,
            frameHeight: 139,
            endFrame: 38
        });

        // 铁桶僵尸行走
        this.load.spritesheet('bucket_walk', 'assets/images/zombies/bucket_head_walk_spritesheet.png', {
            frameWidth: 100,
            frameHeight: 139,
            endFrame: 45
        });

        // 铁桶僵尸吃
        this.load.spritesheet('bucket_eat', 'assets/images/zombies/bucket_head_eat_spritesheet.png', {
            frameWidth: 100,
            frameHeight: 139,
            endFrame: 38
        });

        // 撑杆跳僵尸 - 参考PVZ原版精灵表尺寸
        // walk: 1024x1024 sheet, 100x180 per sprite, 10 cols, 44 frames
        this.load.spritesheet('pole_walk', 'assets/images/zombies/pole_vaulter_walk_spritesheet.png', {
            frameWidth: 100,
            frameHeight: 180,
            endFrame: 43
        });

        // run: 2048x2048 sheet, 300x180 per sprite, 6 cols, 36 frames
        this.load.spritesheet('pole_run', 'assets/images/zombies/pole_vaulter_run_spritesheet.png', {
            frameWidth: 300,
            frameHeight: 180,
            endFrame: 35
        });

        // jump: 2048x2048 sheet, 500x180 per sprite, 4 cols, 42 frames
        this.load.spritesheet('pole_jump', 'assets/images/zombies/pole_vaulter_jump_spritesheet.png', {
            frameWidth: 500,
            frameHeight: 180,
            endFrame: 41
        });

        // eat: 1024x1024 sheet, 100x180 per sprite, 10 cols, 27 frames
        this.load.spritesheet('pole_eat', 'assets/images/zombies/pole_vaulter_eat_spritesheet.png', {
            frameWidth: 100,
            frameHeight: 180,
            endFrame: 26
        });

        // ========== 植物精灵表 (512x512) ==========
        // 豌豆射手: 6列 x 4行, 80x80每帧, 24帧
        this.load.spritesheet('peashooter', 'assets/images/plants/peashooter_spritesheet.png', {
            frameWidth: 80,
            frameHeight: 80,
            endFrame: 23
        });

        // 双发射手
        this.load.spritesheet('repeater', 'assets/images/plants/repeater_spritesheet.png', {
            frameWidth: 80,
            frameHeight: 80,
            endFrame: 23
        });

        // 樱桃炸弹
        this.load.spritesheet('cherry_bomb', 'assets/images/plants/cherry_bomb_spritesheet.png', {
            frameWidth: 73,
            frameHeight: 85,
            endFrame: 13
        });

        // 坚果墙
        this.load.spritesheet('wallnut', 'assets/images/plants/wallnut_spritesheet.png', {
            frameWidth: 73,
            frameHeight: 85,
            endFrame: 37
        });

        // 坚果墙(破损)
        this.load.spritesheet('wallnut_cracked', 'assets/images/plants/wallnut_cracked_spritesheet.png', {
            frameWidth: 73,
            frameHeight: 85,
            endFrame: 37
        });

        // ========== 其他资源 ==========
        // 加载投射物
        this.load.image('pea', 'assets/images/projectiles/pea.png');

        // 加载爆炸效果
        this.load.image('explosion', 'assets/images/effects/explosion.png');

        // 加载道具图片
        this.load.image('item_bucket', 'assets/images/items/bucket.png');
        this.load.image('item_pole', 'assets/images/items/pole.png');
        this.load.image('item_health_potion', 'assets/images/items/health_potion.png');
        this.load.image('item_speed_potion', 'assets/images/items/speed_potion.png');

        // 加载Dave行走精灵表 (16帧, 每帧 180x300, 4x4网格)
        this.load.spritesheet('dave_walk', 'assets/sprites/dave_walk_spritesheet.png', {
            frameWidth: 180,
            frameHeight: 300,
            endFrame: 15
        });
        // 后备：静态Dave图片
        this.load.image('dave', 'assets/images/dave/dave.png');

        // 加载UI
        this.load.image('zombies_won', 'assets/images/ui/ZombiesWon.jpg');
        this.load.image('victory_image', 'assets/images/ui/Victory_image.png');
        this.load.image('defeat_image', 'assets/images/ui/Defeat_image.png');

        // 加载新的胜利/失败图片
        this.load.image('victory_zombie', 'assets/images/ui/Victory_Zombie.png');
        this.load.image('victory_dave', 'assets/images/ui/Victory_Dave.png');
        this.load.image('defeat_zombie', 'assets/images/ui/Defeat_Zombie.png');

        // 加载种子包UI
        this.load.image('seedpacket_peashooter', 'assets/images/ui/seedpackets/seedpacket_peashooter.png');
        this.load.image('seedpacket_repeater', 'assets/images/ui/seedpackets/seedpacket_repeater.png');
        this.load.image('seedpacket_cherry_bomb', 'assets/images/ui/seedpackets/seedpacket_cherry_bomb.png');
        this.load.image('seedpacket_wallnut', 'assets/images/ui/seedpackets/seedpacket_wallnut.png');
        this.load.image('seedpacket_cooldown', 'assets/images/ui/seedpackets/seedpacket_cooldown.png');

        // 加载完成回调
        this.load.on('complete', () => {
            console.log('所有资源加载完成');
            this.assetsLoaded = true;
        });

        this.load.on('loaderror', (file) => {
            console.warn('资源加载失败:', file.key);
        });
    }

    create() {
        console.log('GameScene创建');

        // 设置世界背景色
        this.cameras.main.setBackgroundColor('#1a4d1a');

        // 创建迷宫容器
        this.mazeContainer = this.add.container(0, 0);

        // 创建迷宫图层（用于后备渲染）
        this.mazeGraphics = this.add.graphics();

        // 设置摄像机范围（初始）
        this.cameras.main.setBounds(0, 0, 2250, 3150);
        this.cameras.main.setZoom(1.5);  // 放大摄像机，让角色显示更大

        // 分屏相关
        this.splitScreenEnabled = false;
        this.daveCamera = null;  // 戴夫视角（左半屏）
        this.zombieCamera = null;  // 僵尸视角（右半屏）

        // 设置输入
        this.setupInput();

        // 确保键盘输入获得焦点
        this.input.keyboard.enabled = true;
        this.game.canvas.focus();

        // 注册网络消息处理器
        // 注意：bridge.js 会转换消息类型：MAZE_DATA -> MAZE_INIT, ENTITIES -> ENTITIES_UPDATE
        if (this.networkClient) {
            this.networkClient.on('MAZE_INIT', (data) => this.handleMazeInit(data));
            this.networkClient.on('ENTITIES_UPDATE', (data) => this.handleEntitiesUpdate(data));
            this.networkClient.on('GAME_STATE', (data) => this.handleGameState(data));
            this.networkClient.on('GAME_OVER', (data) => this.handleGameOver(data));
        }

        // 创建精灵表动画
        this.createSpritesheetAnimations();

        // 创建种子包UI（多人模式戴夫玩家使用）
        this.createSeedPacketUI();

        // 根据主菜单选择决定游戏模式
        console.log('===== 检查游戏模式 =====');
        console.log('this.startAsMultiplayer:', this.startAsMultiplayer);
        if (this.startAsMultiplayer) {
            console.log('多人模式启动');
            // 立即发送 ENABLE_DAVE_PLAYER 消息，防止AI在延迟期间种植植物
            if (this.networkClient && this.networkClient.connected) {
                this.networkClient.send('ENABLE_DAVE_PLAYER', {});
                console.log('立即发送 ENABLE_DAVE_PLAYER 消息（防止AI种植）');
            }
            // 延迟启用分屏等视觉效果，等待网络连接稳定
            this.time.delayedCall(500, () => {
                console.log('延迟结束，现在启用多人模式UI');
                this.enableMultiplayerMode();
            });
        } else {
            console.log('以单人模式运行');
        }

        // 延迟聚焦，确保场景完全初始化后键盘可以正常工作
        this.time.delayedCall(100, () => {
            this.input.keyboard.enabled = true;
            this.game.canvas.focus();
            console.log('键盘输入已重新启用并聚焦');
        });

        // 每当场景从暂停/睡眠状态恢复时也重新聚焦
        this.events.on('resume', () => {
            this.input.keyboard.enabled = true;
            this.game.canvas.focus();
            console.log('场景恢复，键盘重新聚焦');
        });

        this.events.on('wake', () => {
            this.input.keyboard.enabled = true;
            this.game.canvas.focus();
            console.log('场景唤醒，键盘重新聚焦');
        });

        // 清理事件监听器，防止内存泄漏
        this.events.on('shutdown', () => {
            if (this.rightShiftHandler) {
                window.removeEventListener('keydown', this.rightShiftHandler);
                this.rightShiftHandler = null;
            }
            console.log('场景关闭，清理事件监听器');
        });

        this.events.on('destroy', () => {
            if (this.rightShiftHandler) {
                window.removeEventListener('keydown', this.rightShiftHandler);
                this.rightShiftHandler = null;
            }
            console.log('场景销毁，清理事件监听器');
        });
    }

    /**
     * 从精灵表创建动画 - 使用精灵表模式
     * 参考原版PVZ动画速度，大幅提高帧率
     */
    createSpritesheetAnimations() {
        // ========== 普通僵尸动画 ==========
        // 原版PVZ僵尸动画约12fps，但我们用更快的速度让动画更流畅
        if (this.textures.exists('zombie_walk')) {
            this.anims.create({
                key: 'zombie_walk_anim',
                frames: this.anims.generateFrameNumbers('zombie_walk', { start: 0, end: 45 }),
                frameRate: 24,  // 加快：18 -> 24
                repeat: -1
            });
            console.log('创建僵尸行走动画: 46帧');
        }

        if (this.textures.exists('zombie_eat')) {
            this.anims.create({
                key: 'zombie_eat_anim',
                frames: this.anims.generateFrameNumbers('zombie_eat', { start: 0, end: 38 }),
                frameRate: 24,  // 加快：18 -> 24
                repeat: -1
            });
            console.log('创建僵尸吃动画: 39帧');
        }

        // ========== 铁桶僵尸动画 ==========
        if (this.textures.exists('bucket_walk')) {
            this.anims.create({
                key: 'bucket_walk_anim',
                frames: this.anims.generateFrameNumbers('bucket_walk', { start: 0, end: 45 }),
                frameRate: 24,  // 加快：18 -> 24
                repeat: -1
            });
        }

        if (this.textures.exists('bucket_eat')) {
            this.anims.create({
                key: 'bucket_eat_anim',
                frames: this.anims.generateFrameNumbers('bucket_eat', { start: 0, end: 38 }),
                frameRate: 24,  // 加快：18 -> 24
                repeat: -1
            });
        }

        // ========== 撑杆跳僵尸动画 ==========
        // walk: 44 frames
        if (this.textures.exists('pole_walk')) {
            this.anims.create({
                key: 'pole_walk_anim',
                frames: this.anims.generateFrameNumbers('pole_walk', { start: 0, end: 43 }),
                frameRate: 24,  // 加快：18 -> 24
                repeat: -1
            });
        }

        // run: 36 frames, faster animation for running with pole
        if (this.textures.exists('pole_run')) {
            this.anims.create({
                key: 'pole_run_anim',
                frames: this.anims.generateFrameNumbers('pole_run', { start: 0, end: 35 }),
                frameRate: 30,  // 加快：24 -> 30
                repeat: -1
            });
        }

        // jump: 42 frames, 后端跳跃持续1.75秒
        // 42帧 / 1.75秒 = 24fps（与普通僵尸走路帧率一致）
        if (this.textures.exists('pole_jump')) {
            this.anims.create({
                key: 'pole_jump_anim',
                frames: this.anims.generateFrameNumbers('pole_jump', { start: 0, end: 41 }),
                frameRate: 24,  // 24fps，动画1.75秒播完（与后端同步）
                repeat: 0
            });
        }

        // eat: 27 frames
        if (this.textures.exists('pole_eat')) {
            this.anims.create({
                key: 'pole_eat_anim',
                frames: this.anims.generateFrameNumbers('pole_eat', { start: 0, end: 26 }),
                frameRate: 24,  // 加快：18 -> 24
                repeat: -1
            });
        }

        // ========== 植物动画 ==========
        // 豌豆射手: 6列 x 4行 = 24帧
        // 原版PVZ豌豆射手动画很快，约20-24fps
        if (this.textures.exists('peashooter')) {
            this.anims.create({
                key: 'peashooter_anim',
                frames: this.anims.generateFrameNumbers('peashooter', { start: 0, end: 23 }),
                frameRate: 24,  // 加快：18 -> 24
                repeat: -1
            });
        }

        if (this.textures.exists('repeater')) {
            this.anims.create({
                key: 'repeater_anim',
                frames: this.anims.generateFrameNumbers('repeater', { start: 0, end: 23 }),
                frameRate: 24,  // 加快：18 -> 24
                repeat: -1
            });
        }

        if (this.textures.exists('cherry_bomb')) {
            this.anims.create({
                key: 'cherry_bomb_anim',
                frames: this.anims.generateFrameNumbers('cherry_bomb', { start: 0, end: 13 }),
                frameRate: 15,  // 加快：10 -> 15
                repeat: 0
            });
        }

        if (this.textures.exists('wallnut')) {
            this.anims.create({
                key: 'wallnut_anim',
                frames: this.anims.generateFrameNumbers('wallnut', { start: 0, end: 37 }),
                frameRate: 12,  // 加快：8 -> 12
                repeat: -1
            });
        }

        if (this.textures.exists('wallnut_cracked')) {
            this.anims.create({
                key: 'wallnut_cracked_anim',
                frames: this.anims.generateFrameNumbers('wallnut_cracked', { start: 0, end: 37 }),
                frameRate: 12,  // 加快：8 -> 12
                repeat: -1
            });
        }

        // 创建Dave行走动画 (16帧)
        if (this.textures.exists('dave_walk')) {
            this.anims.create({
                key: 'dave_walk_anim',
                frames: this.anims.generateFrameNumbers('dave_walk', { start: 0, end: 15 }),
                frameRate: 10,
                repeat: -1
            });
            console.log('Dave行走动画创建完成: 16帧');
        }

        console.log('所有精灵表动画创建完成');
    }

    setupInput() {
        // ==================== 多人模式键位设置 ====================
        // 僵尸玩家：小键盘方向键 + 右Ctrl撑杆跳
        // 戴夫玩家：WASD移动 + Q种植

        // 戴夫控制 - WASD键
        this.keys.W = this.input.keyboard.addKey(Phaser.Input.Keyboard.KeyCodes.W);
        this.keys.A = this.input.keyboard.addKey(Phaser.Input.Keyboard.KeyCodes.A);
        this.keys.S = this.input.keyboard.addKey(Phaser.Input.Keyboard.KeyCodes.S);
        this.keys.D = this.input.keyboard.addKey(Phaser.Input.Keyboard.KeyCodes.D);

        // 僵尸控制 - 方向键（单人模式也可用）
        this.cursors = this.input.keyboard.createCursorKeys();

        // 僵尸控制 - 小键盘方向键（多人模式）
        this.keys.NUMPAD_UP = this.input.keyboard.addKey(Phaser.Input.Keyboard.KeyCodes.NUMPAD_EIGHT);
        this.keys.NUMPAD_DOWN = this.input.keyboard.addKey(Phaser.Input.Keyboard.KeyCodes.NUMPAD_TWO);
        this.keys.NUMPAD_LEFT = this.input.keyboard.addKey(Phaser.Input.Keyboard.KeyCodes.NUMPAD_FOUR);
        this.keys.NUMPAD_RIGHT = this.input.keyboard.addKey(Phaser.Input.Keyboard.KeyCodes.NUMPAD_SIX);

        // Ctrl键（撑杆跳）- 左右Ctrl都可用
        this.keys.CTRL = this.input.keyboard.addKey(Phaser.Input.Keyboard.KeyCodes.CTRL);

        // Tab键（小地图 - 戴夫用）- 阻止浏览器默认行为
        this.keys.TAB = this.input.keyboard.addKey(Phaser.Input.Keyboard.KeyCodes.TAB);
        this.input.keyboard.addCapture(Phaser.Input.Keyboard.KeyCodes.TAB);

        // 直接监听Tab键事件（戴夫小地图）
        this.keys.TAB.on('down', () => {
            console.log('Tab键事件触发');
            if (this.isMultiplayerMode) {
                // 多人模式：Tab键是戴夫的小地图
                this.toggleMinimap('dave');
            } else {
                // 单人模式：Tab键显示僵尸视角小地图
                this.toggleMinimap('zombie');
            }
        });

        // 使用原生键盘事件来检测右Shift键
        // Phaser的event有location属性
        this.input.keyboard.on('keydown', (event) => {
            // 右Shift键的location是2, keyCode是16
            if (event.keyCode === 16 && event.location === 2 && this.isMultiplayerMode) {
                console.log('右Shift键事件触发（僵尸小地图）- Phaser事件');
                this.toggleMinimap('zombie');
            }
        });

        // 作为备用方案，直接监听window的keydown事件
        // 某些情况下Phaser的事件可能无法捕获
        this.rightShiftHandler = (event) => {
            // 使用code属性来判断是否是右Shift键（更可靠）
            if ((event.code === 'ShiftRight' || (event.keyCode === 16 && event.location === 2)) && this.isMultiplayerMode) {
                console.log('右Shift键事件触发（僵尸小地图）- window事件');
                this.toggleMinimap('zombie');
                event.preventDefault();
            }
        };
        window.addEventListener('keydown', this.rightShiftHandler);

        // Q键（打开/关闭种植菜单 - 戴夫用）
        this.keys.Q = this.input.keyboard.addKey(Phaser.Input.Keyboard.KeyCodes.Q);
        this.keys.Q.on('down', () => {
            if (this.isMultiplayerMode) {
                console.log('Q键事件触发（切换种植菜单）');
                this.toggleSeedPacketUI();
            }
        });

        // 空格键攻击（保留用于其他功能）
        this.keys.SPACE = this.input.keyboard.addKey(Phaser.Input.Keyboard.KeyCodes.SPACE);

        // 多人模式标志
        this.isMultiplayerMode = false;  // 默认单人模式

        // 小地图状态
        this.minimapVisible = false;

        // 种植菜单状态
        this.seedPacketVisible = false;

        // 戴夫移动状态追踪
        this.lastDaveMoveDirection = null;

        console.log('输入控制已设置:');
        console.log('  僵尸: 方向键/小键盘移动, Ctrl撑杆跳');
        console.log('  戴夫: WASD移动, Q种植');
        console.log('  小地图: Tab(戴夫), RShift(僵尸)');
    }

    handleMazeInit(maze) {
        console.log('=== 收到迷宫数据 ===');
        console.log('迷宫尺寸:', maze.gridWidth, 'x', maze.gridHeight);
        console.log('cellSize:', maze.cellSize);
        console.log('pixelWidth:', maze.pixelWidth, 'pixelHeight:', maze.pixelHeight);
        console.log('入口(格子坐标):', maze.entrance);
        console.log('出口(格子坐标):', maze.exit);
        console.log('grid存在:', !!maze.grid);
        console.log('grid长度:', maze.grid ? maze.grid.length : 0);
        if (maze.grid && maze.grid.length > 0) {
            console.log('grid[0]长度:', maze.grid[0].length);
            console.log('grid[0][0..4]:', maze.grid[0].slice(0, 5));
            console.log('grid[1][0..4]:', maze.grid[1] ? maze.grid[1].slice(0, 5) : 'N/A');
            // 统计墙壁和通道数量
            let wallCount = 0, pathCount = 0;
            for (let y = 0; y < maze.grid.length; y++) {
                for (let x = 0; x < maze.grid[y].length; x++) {
                    if (maze.grid[y][x] === 0) wallCount++;
                    else pathCount++;
                }
            }
            console.log('墙壁格子数:', wallCount, '通道格子数:', pathCount);
        }
        this.maze = maze;

        // 隐藏加载提示
        if (this.loadingText) {
            this.loadingText.destroy();
            this.loadingText = null;
            console.log('加载提示已隐藏');
        }

        // 绘制迷宫
        this.renderMaze();

        // 更新摄像机边界
        this.cameras.main.setBounds(0, 0, maze.pixelWidth, maze.pixelHeight);
        console.log('迷宫初始化完成');
    }

    renderMaze() {
        if (!this.maze) return;

        console.log('===== renderMaze 开始 =====');
        console.log('maze.grid 存在:', !!this.maze.grid);
        console.log('maze.grid 类型:', typeof this.maze.grid);
        console.log('maze.grid 是数组:', Array.isArray(this.maze.grid));
        if (this.maze.grid && this.maze.grid[0]) {
            console.log('maze.grid[0] 类型:', typeof this.maze.grid[0]);
            console.log('maze.grid[0] 是数组:', Array.isArray(this.maze.grid[0]));
            console.log('maze.grid[0][0] 类型:', typeof this.maze.grid[0][0]);
            console.log('maze.grid[0][0] 值:', this.maze.grid[0][0]);
            console.log('maze.grid[0][0] === 0:', this.maze.grid[0][0] === 0);
            console.log('maze.grid[0][0] == 0:', this.maze.grid[0][0] == 0);
        }

        // 清除旧的瓦片和图形
        this.mazeTiles.forEach(tile => tile.destroy());
        this.mazeTiles = [];
        this.mazeGraphics.clear();

        const cellSize = this.maze.cellSize;
        const gridWidth = this.maze.gridWidth;
        const gridHeight = this.maze.gridHeight;
        const pixelWidth = this.maze.pixelWidth;
        const pixelHeight = this.maze.pixelHeight;

        console.log('渲染参数: cellSize=', cellSize, 'gridWidth=', gridWidth, 'gridHeight=', gridHeight);

        // 从grid数据判断墙壁位置（grid[y][x] === 0 表示墙壁）
        // grid值: 0=WALL, 1=PATH, 2=ENTRANCE, 3=EXIT, 4=ITEM_SPAWN
        const isWallAt = (gridX, gridY) => {
            if (!this.maze.grid || !this.maze.grid[gridY]) {
                console.warn('isWallAt: grid不存在, gridY=', gridY);
                return true;
            }
            const cellValue = this.maze.grid[gridY][gridX];
            return cellValue === 0;
        };

        // 测试几个点的isWallAt结果
        if (this.maze.grid) {
            console.log('测试isWallAt:');
            console.log('  (0,0):', isWallAt(0, 0), '值:', this.maze.grid[0][0]);
            console.log('  (1,1):', isWallAt(1, 1), '值:', this.maze.grid[1]?.[1]);
            console.log('  (5,5):', isWallAt(5, 5), '值:', this.maze.grid[5]?.[5]);
        }

        // ========== DungeonCanvas 风格的迷宫渲染 ==========

        // 噪声函数
        const perlinNoise = (x, y, seed = 0) => {
            const n = Math.sin(x * 12.9898 + y * 78.233 + seed * 43758.5453) * 43758.5453;
            return n - Math.floor(n);
        };

        // 湍流噪声
        const turbulence = (x, y) => {
            let result = 0;
            let amplitude = 1;
            let frequency = 1;
            let maxValue = 0;

            for (let i = 0; i < 4; i++) {
                result += amplitude * perlinNoise(x * frequency, y * frequency, i);
                maxValue += amplitude;
                amplitude *= 0.5;
                frequency *= 2;
            }

            return result / maxValue;
        };

        // 创建离屏canvas来绘制迷宫纹理
        const textureKey = 'maze_texture_' + Date.now();
        const canvas = document.createElement('canvas');
        canvas.width = pixelWidth;
        canvas.height = pixelHeight;
        const ctx = canvas.getContext('2d');

        // 检查某个像素位置是否在墙壁内
        const isWallPixel = (px, py) => {
            const gridX = Math.floor(px / cellSize);
            const gridY = Math.floor(py / cellSize);
            return isWallAt(gridX, gridY);
        };

        // 绘制噪声纹理
        const drawNoise = (x, y, size, isWall) => {
            for (let i = 0; i < size; i += 3) {
                for (let j = 0; j < size; j += 3) {
                    const noise = Math.random() * 25 - 12.5;
                    const alpha = 0.25 + Math.random() * 0.25;
                    // 墙壁用深色，通道用浅色
                    const baseGreen = isWall ? 50 : 90;
                    ctx.fillStyle = `rgba(0, ${baseGreen + noise}, 0, ${alpha})`;
                    ctx.fillRect(x + i, y + j, 3, 3);
                }
            }
        };

        // 绘制藤蔓
        const drawVines = (x, y, isWall) => {
            if (Math.random() > 0.82) {
                const vineLength = 4 + Math.floor(Math.random() * 6);
                ctx.strokeStyle = isWall
                    ? `rgba(35, 70, 25, ${0.5 + Math.random() * 0.4})`
                    : `rgba(70, 110, 50, ${0.4 + Math.random() * 0.3})`;
                ctx.lineWidth = 1 + Math.random() * 0.5;
                ctx.beginPath();
                let px = x + Math.random() * cellSize;
                let py = y + Math.random() * cellSize;
                ctx.moveTo(px, py);

                for (let i = 0; i < vineLength; i++) {
                    const angle = Math.random() * Math.PI * 2;
                    const length = 8 + Math.random() * 6;
                    px += Math.cos(angle) * length;
                    py += Math.sin(angle) * length;
                    ctx.lineTo(px, py);
                }
                ctx.stroke();
            }
        };

        // 绘制草
        const drawGrass = (x, y, isWall) => {
            if (Math.random() > 0.65) {
                const grassCount = 3 + Math.floor(Math.random() * 4);
                for (let i = 0; i < grassCount; i++) {
                    const gx = x + Math.random() * cellSize;
                    const gy = y + Math.random() * cellSize;
                    const height = 4 + Math.random() * 6;
                    const bend = (Math.random() - 0.5) * 3;

                    ctx.strokeStyle = isWall
                        ? `rgba(45, 85, 35, ${0.55 + Math.random() * 0.3})`
                        : `rgba(75, 120, 55, ${0.45 + Math.random() * 0.3})`;
                    ctx.lineWidth = 0.8 + Math.random() * 0.4;
                    ctx.beginPath();
                    ctx.moveTo(gx, gy);
                    ctx.quadraticCurveTo(gx + bend, gy - height * 0.5, gx + bend * 0.5, gy - height);
                    ctx.stroke();
                }
            }
        };

        // 绘制石头
        const drawStone = (x, y, isWall) => {
            if (Math.random() > 0.88) {
                const size = 4 + Math.random() * 6;
                ctx.fillStyle = isWall
                    ? `rgba(60, 75, 50, ${0.5 + Math.random() * 0.3})`
                    : `rgba(100, 120, 85, ${0.4 + Math.random() * 0.3})`;
                ctx.beginPath();
                ctx.ellipse(
                    x + Math.random() * cellSize,
                    y + Math.random() * cellSize,
                    size,
                    size * 0.65,
                    Math.random() * Math.PI,
                    0,
                    Math.PI * 2
                );
                ctx.fill();
            }
        };

        // 绘制小点
        const drawDots = (x, y, isWall) => {
            if (Math.random() > 0.78) {
                const dotCount = 2 + Math.floor(Math.random() * 3);
                for (let i = 0; i < dotCount; i++) {
                    ctx.fillStyle = isWall
                        ? `rgba(120, 180, 100, ${0.35 + Math.random() * 0.25})`
                        : `rgba(160, 210, 140, ${0.3 + Math.random() * 0.25})`;
                    ctx.beginPath();
                    ctx.arc(
                        x + Math.random() * cellSize,
                        y + Math.random() * cellSize,
                        1.2 + Math.random() * 0.8,
                        0,
                        Math.PI * 2
                    );
                    ctx.fill();
                }
            }
        };

        // 绘制迷宫基础和装饰
        for (let gridY = 0; gridY < gridHeight; gridY++) {
            for (let gridX = 0; gridX < gridWidth; gridX++) {
                const x = gridX * cellSize;
                const y = gridY * cellSize;
                const isWall = isWallAt(gridX, gridY);

                // 基础颜色
                if (isWall) {
                    // 墙壁 - 深绿色调（类似DungeonCanvas的channel）
                    const baseGreen = 45 + Math.random() * 20;
                    ctx.fillStyle = `rgb(${Math.floor(baseGreen * 0.5)}, ${Math.floor(baseGreen)}, ${Math.floor(baseGreen * 0.6)})`;
                } else {
                    // 通道 - 浅绿色调
                    const baseGreen = 85 + Math.random() * 35;
                    ctx.fillStyle = `rgb(${Math.floor(baseGreen * 0.6)}, ${Math.floor(baseGreen)}, ${Math.floor(baseGreen * 0.7)})`;
                }
                ctx.fillRect(x, y, cellSize, cellSize);

                // 添加装饰效果
                drawNoise(x, y, cellSize, isWall);
                drawGrass(x, y, isWall);
                drawVines(x, y, isWall);
                drawStone(x, y, isWall);
                drawDots(x, y, isWall);
            }
        }

        // 绘制墙壁边缘（让墙壁更明显）
        ctx.strokeStyle = 'rgba(20, 40, 15, 0.6)';
        ctx.lineWidth = 2;
        for (let gridY = 0; gridY < gridHeight; gridY++) {
            for (let gridX = 0; gridX < gridWidth; gridX++) {
                const isWall = isWallAt(gridX, gridY);
                if (isWall) {
                    const x = gridX * cellSize;
                    const y = gridY * cellSize;

                    // 检查相邻格子，只在墙壁和通道交界处画边
                    const leftIsPath = gridX > 0 && !isWallAt(gridX - 1, gridY);
                    const rightIsPath = gridX < gridWidth - 1 && !isWallAt(gridX + 1, gridY);
                    const topIsPath = gridY > 0 && !isWallAt(gridX, gridY - 1);
                    const bottomIsPath = gridY < gridHeight - 1 && !isWallAt(gridX, gridY + 1);

                    ctx.beginPath();
                    if (leftIsPath) {
                        ctx.moveTo(x, y);
                        ctx.lineTo(x, y + cellSize);
                    }
                    if (rightIsPath) {
                        ctx.moveTo(x + cellSize, y);
                        ctx.lineTo(x + cellSize, y + cellSize);
                    }
                    if (topIsPath) {
                        ctx.moveTo(x, y);
                        ctx.lineTo(x + cellSize, y);
                    }
                    if (bottomIsPath) {
                        ctx.moveTo(x, y + cellSize);
                        ctx.lineTo(x + cellSize, y + cellSize);
                    }
                    ctx.stroke();
                }
            }
        }

        // 将canvas转换为Phaser纹理并显示
        this.textures.addCanvas(textureKey, canvas);
        const mazeImage = this.add.image(pixelWidth / 2, pixelHeight / 2, textureKey);
        mazeImage.setDepth(0);
        this.mazeTiles.push(mazeImage);

        // 绘制入口（注意：entrance.x/y 是格子坐标，需要转换为像素坐标）
        if (this.maze.entrance) {
            // 格子坐标转像素坐标（格子中心）
            const entranceX = this.maze.entrance.x * cellSize + cellSize / 2;
            const entranceY = this.maze.entrance.y * cellSize + cellSize / 2;
            console.log('入口像素坐标:', entranceX, entranceY);

            // 入口光圈效果
            const entranceGlow = this.add.graphics();
            entranceGlow.fillStyle(0x00ff00, 0.3);
            entranceGlow.fillCircle(entranceX, entranceY, cellSize * 0.6);
            entranceGlow.fillStyle(0x00ff00, 0.5);
            entranceGlow.fillCircle(entranceX, entranceY, cellSize * 0.4);
            entranceGlow.setDepth(2);
            this.mazeTiles.push(entranceGlow);

            // 入口标签
            const entranceLabel = this.add.text(entranceX, entranceY - cellSize / 2 - 15, '入口', {
                fontSize: '16px',
                fontStyle: 'bold',
                color: '#00ff00',
                backgroundColor: '#000000aa',
                padding: { x: 6, y: 3 }
            }).setOrigin(0.5).setDepth(100);
            this.mazeTiles.push(entranceLabel);
        }

        // 绘制出口（注意：exit.x/y 是格子坐标，需要转换为像素坐标）
        if (this.maze.exit) {
            // 格子坐标转像素坐标（格子中心）
            const exitX = this.maze.exit.x * cellSize + cellSize / 2;
            const exitY = this.maze.exit.y * cellSize + cellSize / 2;
            console.log('出口像素坐标:', exitX, exitY);

            // 出口光圈效果
            const exitGlow = this.add.graphics();
            exitGlow.fillStyle(0xff0000, 0.3);
            exitGlow.fillCircle(exitX, exitY, cellSize * 0.6);
            exitGlow.fillStyle(0xff0000, 0.5);
            exitGlow.fillCircle(exitX, exitY, cellSize * 0.4);
            exitGlow.setDepth(2);
            this.mazeTiles.push(exitGlow);

            // 出口标签
            const exitLabel = this.add.text(exitX, exitY - cellSize / 2 - 15, '出口', {
                fontSize: '16px',
                fontStyle: 'bold',
                color: '#ff0000',
                backgroundColor: '#000000aa',
                padding: { x: 6, y: 3 }
            }).setOrigin(0.5).setDepth(100);
            this.mazeTiles.push(exitLabel);
        }

        console.log('迷宫渲染完成 (DungeonCanvas风格)');
    }

    handleEntitiesUpdate(entities) {
        // 隐藏加载提示（如果还存在）
        if (this.loadingText) {
            this.loadingText.destroy();
            this.loadingText = null;
        }

        // 更新所有实体
        const currentEntityIds = new Set();

        // 找到唯一的僵尸实体和戴夫实体（只取第一个）
        let zombieEntity = null;
        let daveEntity = null;
        for (const e of entities) {
            if (e.type === 'zombie' && !zombieEntity) {
                zombieEntity = e;
            }
            if (e.type === 'dave' && !daveEntity) {
                daveEntity = e;
            }
        }

        // 处理非僵尸和非戴夫实体
        entities.forEach(entityData => {
            if (entityData.type === 'zombie') return;  // 僵尸单独处理
            if (entityData.type === 'dave') return;    // 戴夫单独处理

            const entityId = Number(entityData.id);
            currentEntityIds.add(entityId);

            this.interpolationManager.updateEntity(entityId, entityData);

            let sprite = this.entities.get(entityId);

            if (!sprite) {
                sprite = this.createEntitySprite(entityData);
                this.entities.set(entityId, sprite);
            }

            sprite.setData('entityData', entityData);
            sprite.setData('entityId', entityId);
            // 位置更新在update()中通过lerp平滑处理，这里只更新动画
            this.updateEntityAnimation(sprite, entityData);
        });

        // 单独处理僵尸（确保只有一个）
        if (zombieEntity) {
            const entityId = Number(zombieEntity.id);
            currentEntityIds.add(entityId);

            // 如果僵尸ID变化或者没有僵尸精灵，需要清理并重建
            const needsRecreate = this.zombieId !== entityId || !this.zombieSprite || !this.zombieSprite.active;

            if (needsRecreate) {
                // 清理所有现有的僵尸相关精灵
                const zombieIdsToRemove = [];
                this.entities.forEach((sprite, id) => {
                    const data = sprite.getData('entityData');
                    if (data && data.type === 'zombie') {
                        zombieIdsToRemove.push(id);
                    }
                });

                zombieIdsToRemove.forEach(id => {
                    const sprite = this.entities.get(id);
                    if (sprite) {
                        if (sprite.healthBar) sprite.healthBar.destroy();
                        if (sprite.healthBarBg) sprite.healthBarBg.destroy();
                        if (sprite.armorBar) sprite.armorBar.destroy();
                        if (sprite.armorBarBg) sprite.armorBarBg.destroy();
                        if (sprite.nameLabel) sprite.nameLabel.destroy();
                        sprite.destroy();
                        this.entities.delete(id);
                    }
                });

                // 也清理zombieSprite引用
                if (this.zombieSprite && this.zombieSprite.active) {
                    if (this.zombieSprite.healthBar) this.zombieSprite.healthBar.destroy();
                    if (this.zombieSprite.healthBarBg) this.zombieSprite.healthBarBg.destroy();
                    if (this.zombieSprite.armorBar) this.zombieSprite.armorBar.destroy();
                    if (this.zombieSprite.armorBarBg) this.zombieSprite.armorBarBg.destroy();
                    if (this.zombieSprite.nameLabel) this.zombieSprite.nameLabel.destroy();
                    this.zombieSprite.destroy();
                }
                this.zombieSprite = null;

                // 创建新的僵尸精灵
                this.zombieSprite = this.createEntitySprite(zombieEntity);
                this.zombieId = entityId;
                this.entities.set(entityId, this.zombieSprite);
                this.cameras.main.startFollow(this.zombieSprite, true, 0.1, 0.1);
            }

            // 更新僵尸精灵（只有当精灵有效时）
            if (this.zombieSprite && this.zombieSprite.active) {
                this.zombieSprite.setData('entityData', zombieEntity);
                this.zombieSprite.setData('entityId', entityId);
                // 位置更新在update()中通过lerp平滑处理，这里只更新动画
                this.updateZombieAnimation(this.zombieSprite, zombieEntity);
            }
        }

        // 单独处理戴夫（确保只有一个，用于分屏摄像机跟随）
        if (daveEntity) {
            const entityId = Number(daveEntity.id);
            currentEntityIds.add(entityId);

            // 如果戴夫ID变化或者没有戴夫精灵，需要清理并重建
            const needsRecreate = this.daveId !== entityId || !this.daveSprite || !this.daveSprite.active;

            if (needsRecreate) {
                // 清理所有现有的戴夫相关精灵
                const daveIdsToRemove = [];
                this.entities.forEach((sprite, id) => {
                    const data = sprite.getData('entityData');
                    if (data && data.type === 'dave') {
                        daveIdsToRemove.push(id);
                    }
                });

                daveIdsToRemove.forEach(id => {
                    const sprite = this.entities.get(id);
                    if (sprite) {
                        if (sprite.healthBar) sprite.healthBar.destroy();
                        if (sprite.healthBarBg) sprite.healthBarBg.destroy();
                        if (sprite.nameLabel) sprite.nameLabel.destroy();
                        sprite.destroy();
                        this.entities.delete(id);
                    }
                });

                // 也清理daveSprite引用
                if (this.daveSprite && this.daveSprite.active) {
                    if (this.daveSprite.healthBar) this.daveSprite.healthBar.destroy();
                    if (this.daveSprite.healthBarBg) this.daveSprite.healthBarBg.destroy();
                    if (this.daveSprite.nameLabel) this.daveSprite.nameLabel.destroy();
                    this.daveSprite.destroy();
                }
                this.daveSprite = null;

                // 创建新的戴夫精灵
                this.daveSprite = this.createEntitySprite(daveEntity);
                this.daveId = entityId;
                this.entities.set(entityId, this.daveSprite);

                // 如果分屏已启用，让戴夫摄像机跟随戴夫
                if (this.splitScreenEnabled && this.daveCamera) {
                    this.daveCamera.startFollow(this.daveSprite, true, 0.1, 0.1);
                }
            }

            // 更新戴夫精灵（只有当精灵有效时）
            if (this.daveSprite && this.daveSprite.active) {
                this.daveSprite.setData('entityData', daveEntity);
                this.daveSprite.setData('entityId', entityId);
                // 位置更新在update()中通过lerp平滑处理，这里只更新动画
                this.updateEntityAnimation(this.daveSprite, daveEntity);
                // 存储Dave数据用于种子包UI更新
                this.storeDaveData(daveEntity);
            }
        }

        // 移除不再存在的实体（不包括僵尸和戴夫，单独管理）
        const idsToRemove = [];
        this.entities.forEach((sprite, id) => {
            if (!currentEntityIds.has(id)) {
                // 如果是当前的僵尸或戴夫精灵，不要删除
                if (id === this.zombieId) return;
                if (id === this.daveId) return;
                idsToRemove.push(id);
            }
        });

        idsToRemove.forEach(id => {
            const sprite = this.entities.get(id);
            if (sprite) {
                if (sprite.healthBar) sprite.healthBar.destroy();
                if (sprite.healthBarBg) sprite.healthBarBg.destroy();
                if (sprite.armorBar) sprite.armorBar.destroy();
                if (sprite.armorBarBg) sprite.armorBarBg.destroy();
                if (sprite.nameLabel) sprite.nameLabel.destroy();
                sprite.destroy();
                this.entities.delete(id);
                this.interpolationManager.removeEntity(id);
            }
        });
    }

    /**
     * 更新僵尸动画 - 使用Phaser精灵表动画
     * 参考PVZ原版实现，根据装备和状态选择正确的动画
     *
     * 撑杆跳僵尸精灵尺寸:
     * - walk/eat: 100x180
     * - run: 300x180 (3倍宽，但僵尸主体在中间)
     * - jump: 500x180 (5倍宽)
     *
     * 缩放策略：撑杆跳僵尸精灵内的僵尸主体较小，需要更大缩放
     */
    updateZombieAnimation(sprite, entityData) {
        const equipment = entityData.equipment || 'normal';
        const poleVaultJumped = entityData.poleVaultJumped === true;
        const poleVaultJumping = entityData.poleVaultJumping === true;
        const isEating = entityData.isEating === true;

        // 确定目标纹理、动画和缩放
        let targetTexture, targetAnim, scale;

        if (equipment === 'bucket') {
            // 铁桶僵尸: 100x139
            if (isEating) {
                targetTexture = 'bucket_eat';
                targetAnim = 'bucket_eat_anim';
            } else {
                targetTexture = 'bucket_walk';
                targetAnim = 'bucket_walk_anim';
            }
            scale = 0.9;
        } else if (equipment === 'pole_vault') {
            // 撑杆跳僵尸
            if (isEating) {
                targetTexture = 'pole_eat';
                targetAnim = 'pole_eat_anim';
                scale = 0.9;
            } else if (poleVaultJumping) {
                // 正在跳跃中: 使用jump动画 (500x180)
                targetTexture = 'pole_jump';
                targetAnim = 'pole_jump_anim';
                scale = 0.9;

                // 根据跳跃方向旋转动画
                // Direction枚举: UP=0, DOWN=1, LEFT=2, RIGHT=3
                // 注意：原始跳跃动画是向左跳的（角色面向左移动）
                const jumpDirection = entityData.jumpDirection !== undefined ? entityData.jumpDirection : 2;
                switch (jumpDirection) {
                    case 0:  // UP - 顺时针旋转90度 + flipX
                        sprite.setRotation(Math.PI / 2);
                        sprite.setFlipX(true);
                        sprite.setFlipY(false);
                        break;
                    case 1:  // DOWN - 逆时针旋转90度 + flipX
                        sprite.setRotation(-Math.PI / 2);
                        sprite.setFlipX(true);
                        sprite.setFlipY(false);
                        break;
                    case 2:  // LEFT - 原始动画方向（向左跳）
                        sprite.setRotation(0);
                        sprite.setFlipX(false);
                        sprite.setFlipY(false);
                        break;
                    case 3:  // RIGHT - 水平翻转（变成向右跳）
                    default:
                        sprite.setRotation(0);
                        sprite.setFlipX(true);
                        sprite.setFlipY(false);
                        break;
                }
            } else if (poleVaultJumped) {
                // 跳跃后: 使用walk动画 (100x180)
                targetTexture = 'pole_walk';
                targetAnim = 'pole_walk_anim';
                scale = 0.9;
                // 跳跃完成后恢复正常方向
                sprite.setRotation(0);
                sprite.setFlipY(false);
            } else {
                // 持杆跑: 使用run动画 (300x180)
                targetTexture = 'pole_run';
                targetAnim = 'pole_run_anim';
                scale = 0.9;
                // 持杆跑时恢复正常方向
                sprite.setRotation(0);
                sprite.setFlipY(false);
            }
        } else {
            // 普通僵尸: 100x139
            if (isEating) {
                targetTexture = 'zombie_eat';
                targetAnim = 'zombie_eat_anim';
            } else {
                targetTexture = 'zombie_walk';
                targetAnim = 'zombie_walk_anim';
            }
            scale = 0.9;
        }

        // 使用动画key作为状态key - 只在动画真正需要切换时才调用play()
        const currentAnimKey = sprite.anims.currentAnim ? sprite.anims.currentAnim.key : null;

        // 只有当动画key不同时才切换动画 - 避免频繁调用play()导致卡顿
        if (currentAnimKey !== targetAnim) {
            // 检查纹理是否存在并切换
            if (this.textures.exists(targetTexture)) {
                sprite.setTexture(targetTexture, 0);
                sprite.setScale(scale);
                // 播放新动画
                if (this.anims.exists(targetAnim)) {
                    sprite.play(targetAnim);
                }
            }
        }

        // 根据移动方向翻转精灵（跳跃中不翻转，保持跳跃方向的旋转设置）
        if (!poleVaultJumping) {
            if (entityData.vx < 0) {
                sprite.setFlipX(false);
            } else if (entityData.vx > 0) {
                sprite.setFlipX(true);
            }
        }

        // 检测是否在移动
        const isMoving = entityData.vx !== 0 || entityData.vy !== 0;

        // 撑杆跳僵尸持杆跑动时始终播放动画，跳跃中也播放动画
        const shouldAlwaysAnimate = (equipment === 'pole_vault' && !poleVaultJumped && !isEating) || poleVaultJumping;

        // 动画播放逻辑
        if (isMoving || shouldAlwaysAnimate || isEating) {
            // 移动中或需要持续动画时，确保动画在播放
            if (targetAnim && this.anims.exists(targetAnim)) {
                const currentAnim = sprite.anims.currentAnim;
                if (!currentAnim || currentAnim.key !== targetAnim) {
                    // 切换到不同的动画
                    sprite.play(targetAnim);
                } else if (!sprite.anims.isPlaying) {
                    // 同一动画但已暂停，从当前帧继续播放
                    sprite.anims.resume();
                }
                // 如果已经在播放相同动画，不做任何操作
            }
        } else {
            // 停止移动且不攻击时，暂停动画并保持当前帧
            if (sprite.anims.isPlaying) {
                sprite.anims.pause();
                // 不重置帧，保持在当前帧
            }
        }
    }

    createEntitySprite(entityData) {
        let sprite;
        const x = entityData.x;
        const y = entityData.y;

        switch (entityData.type) {
            case 'zombie':
                sprite = this.createZombieSprite(x, y, entityData);
                break;

            case 'dave':
                sprite = this.createDaveSprite(x, y, entityData);
                break;

            case 'plant':
                sprite = this.createPlantSprite(x, y, entityData);
                break;

            case 'item':
                sprite = this.createItemSprite(x, y, entityData);
                break;

            case 'projectile':
                sprite = this.createProjectileSprite(x, y, entityData);
                break;

            default:
                sprite = this.createDefaultSprite(x, y, entityData);
                break;
        }

        sprite.setDepth(10);

        // 创建生命值条（如果还没有）
        if (!sprite.healthBarBg) {
            sprite.healthBarBg = this.add.graphics();
            sprite.healthBarBg.setDepth(100);
        }
        if (!sprite.healthBar) {
            sprite.healthBar = this.add.graphics();
            sprite.healthBar.setDepth(101);
        }

        // 创建护甲条（如果还没有 - 铁桶道具在createItemSprite中已创建）
        if (!sprite.armorBarBg) {
            sprite.armorBarBg = this.add.graphics();
            sprite.armorBarBg.setDepth(100);
        }
        if (!sprite.armorBar) {
            sprite.armorBar = this.add.graphics();
            sprite.armorBar.setDepth(101);
        }

        return sprite;
    }

    createZombieSprite(x, y, entityData) {
        let sprite;

        // 根据装备选择初始精灵表
        const equipment = entityData.equipment || 'normal';
        const poleVaultJumped = entityData.poleVaultJumped === true;
        const poleVaultJumping = entityData.poleVaultJumping === true;

        // 确定初始纹理和动画
        // 撑杆跳僵尸使用不同尺寸的精灵，需要调整缩放保持僵尸主体一致
        let textureKey, animKey, scale;

        if (equipment === 'bucket') {
            // 铁桶僵尸: 100x139
            textureKey = 'bucket_walk';
            animKey = 'bucket_walk_anim';
            scale = 0.9;
        } else if (equipment === 'pole_vault') {
            // 撑杆跳僵尸 - 使用0.9缩放使僵尸主体和普通僵尸一样大
            if (poleVaultJumping) {
                // 正在跳跃中: jump动画 (500x180)
                textureKey = 'pole_jump';
                animKey = 'pole_jump_anim';
                scale = 0.9;
            } else if (poleVaultJumped) {
                // 跳跃后: walk动画 (100x180)
                textureKey = 'pole_walk';
                animKey = 'pole_walk_anim';
                scale = 0.9;
            } else {
                // 持杆跑: run动画 (300x180)
                textureKey = 'pole_run';
                animKey = 'pole_run_anim';
                scale = 0.9;
            }
        } else {
            // 普通僵尸: 100x139
            textureKey = 'zombie_walk';
            animKey = 'zombie_walk_anim';
            scale = 0.9;
        }

        // 使用精灵表创建精灵
        if (this.textures.exists(textureKey)) {
            sprite = this.add.sprite(x, y, textureKey, 0);
            if (this.anims.exists(animKey)) {
                sprite.play(animKey);
            }
        } else if (this.textures.exists('zombie_walk')) {
            sprite = this.add.sprite(x, y, 'zombie_walk', 0);
            textureKey = 'zombie_walk';
            animKey = 'zombie_walk_anim';
            scale = 0.9;
            if (this.anims.exists('zombie_walk_anim')) {
                sprite.play('zombie_walk_anim');
            }
        } else {
            // 后备：创建程序化精灵
            if (!this.textures.exists('zombie_fallback')) {
                const graphics = this.add.graphics();
                graphics.fillStyle(0x7cba5f, 1.0);
                graphics.fillRoundedRect(-20, -30, 40, 60, 8);
                graphics.fillStyle(0x5a8a3f, 1.0);
                graphics.fillCircle(0, -20, 15);
                graphics.generateTexture('zombie_fallback', 40, 60);
                graphics.destroy();
            }
            sprite = this.add.sprite(x, y, 'zombie_fallback');
            textureKey = 'zombie_fallback';
            scale = 1.0;
        }

        // 设置原点为底部中心
        sprite.setOrigin(0.5, 1);
        sprite.setScale(scale);

        // 记录初始状态key（与updateZombieAnimation保持一致）
        const stateKey = `${equipment}_${poleVaultJumped}`;
        sprite.setData('zombieStateKey', stateKey);

        return sprite;
    }

    createDaveSprite(x, y, entityData) {
        let sprite;

        // 优先使用Dave行走精灵表
        if (this.textures.exists('dave_walk')) {
            sprite = this.add.sprite(x, y, 'dave_walk');
            // 设置合适的缩放 (原帧 180x300，缩放到约95像素高)
            sprite.setScale(0.32);
            // 设置原点在底部中心，方便定位
            sprite.setOrigin(0.5, 1);
            // 标记使用精灵表动画
            sprite.setData('useSpritesheet', true);
        } else if (this.textures.exists('dave')) {
            // 后备：静态图片
            sprite = this.add.sprite(x, y, 'dave');
            sprite.setScale(0.15);
            sprite.setOrigin(0.5, 1);
            sprite.setData('useSpritesheet', false);
        } else {
            // 最后备方案：程序化精灵
            if (!this.textures.exists('dave_sprite')) {
                const graphics = this.add.graphics();
                graphics.fillStyle(0x4169e1, 1.0);
                graphics.fillRoundedRect(-20, -15, 40, 50, 8);
                graphics.fillStyle(0xffd4b3, 1.0);
                graphics.fillCircle(0, -25, 18);
                graphics.fillStyle(0x808080, 1.0);
                graphics.fillRect(-15, -45, 30, 10);
                graphics.generateTexture('dave_sprite', 50, 70);
                graphics.destroy();
            }
            sprite = this.add.sprite(x, y, 'dave_sprite');
            sprite.setData('useSpritesheet', false);
        }

        // 存储动画状态
        sprite.setData('isMoving', false);
        sprite.setData('isAttacking', false);
        sprite.setData('lastX', x);
        sprite.setData('lastY', y);
        sprite.setData('walkPhase', 0);

        // 如果不使用精灵表，创建程序化行走动画
        if (!sprite.getData('useSpritesheet')) {
            this.createDaveWalkAnimation(sprite);
        }

        // 添加名称标签
        sprite.nameLabel = this.add.text(x, y - 100, '疯狂戴夫', {
            fontSize: '12px',
            color: '#ff6b35',
            backgroundColor: '#000000aa',
            padding: { x: 3, y: 2 }
        }).setOrigin(0.5).setDepth(102);

        return sprite;
    }

    /**
     * 创建Dave的行走动画效果
     */
    createDaveWalkAnimation(sprite) {
        // 行走时的上下弹跳动画 - 更明显的效果
        sprite.walkTween = this.tweens.add({
            targets: sprite,
            scaleY: { from: 0.15, to: 0.13 },
            scaleX: { from: 0.15, to: 0.16 },
            y: '-=5',
            duration: 120,
            yoyo: true,
            repeat: -1,
            paused: true,
            ease: 'Sine.easeInOut'
        });

        // 摇摆动画 - 更明显的摇摆
        sprite.swayTween = this.tweens.add({
            targets: sprite,
            angle: { from: -8, to: 8 },
            duration: 180,
            yoyo: true,
            repeat: -1,
            paused: true,
            ease: 'Sine.easeInOut'
        });
    }

    /**
     * 更新Dave的动画状态
     */
    updateDaveAnimation(sprite, entityData) {
        const currentX = sprite.x;
        const currentY = sprite.y;
        const lastX = sprite.getData('lastX') || currentX;
        const lastY = sprite.getData('lastY') || currentY;

        // 检测是否在移动
        const dx = currentX - lastX;
        const dy = currentY - lastY;
        const isMoving = Math.abs(dx) > 0.5 || Math.abs(dy) > 0.5;

        // 更新朝向（根据移动方向翻转）
        if (Math.abs(dx) > 0.5) {
            sprite.setFlipX(dx < 0);
        }

        // 检查是否使用精灵表动画
        const useSpritesheet = sprite.getData('useSpritesheet');

        if (useSpritesheet) {
            // 使用精灵表动画
            if (isMoving && !sprite.getData('isMoving')) {
                // 开始移动 - 播放行走动画
                sprite.setData('isMoving', true);
                if (this.anims.exists('dave_walk_anim')) {
                    sprite.play('dave_walk_anim');
                }
            } else if (!isMoving && sprite.getData('isMoving')) {
                // 停止移动 - 停止动画，显示第一帧
                sprite.setData('isMoving', false);
                sprite.stop();
                sprite.setFrame(0);
            }
        } else {
            // 使用程序化tween动画
            if (isMoving && !sprite.getData('isMoving')) {
                // 开始移动
                sprite.setData('isMoving', true);
                if (sprite.walkTween) sprite.walkTween.resume();
                if (sprite.swayTween) sprite.swayTween.resume();
            } else if (!isMoving && sprite.getData('isMoving')) {
                // 停止移动
                sprite.setData('isMoving', false);
                if (sprite.walkTween) sprite.walkTween.pause();
                if (sprite.swayTween) sprite.swayTween.pause();
                // 重置角度和缩放
                sprite.setAngle(0);
                sprite.setScale(0.15);
            }
        }

        // 保存当前位置
        sprite.setData('lastX', currentX);
        sprite.setData('lastY', currentY);
    }

    /**
     * 播放Dave的攻击动画
     */
    playDaveAttackAnimation(sprite) {
        if (sprite.getData('isAttacking')) return;

        sprite.setData('isAttacking', true);

        // 攻击动画：快速前倾然后恢复
        this.tweens.add({
            targets: sprite,
            scaleX: 0.17,
            scaleY: 0.13,
            angle: sprite.flipX ? 15 : -15,
            duration: 100,
            yoyo: true,
            ease: 'Power2',
            onComplete: () => {
                sprite.setData('isAttacking', false);
                sprite.setScale(0.15);
                sprite.setAngle(0);
            }
        });
    }

    /**
     * 创建植物精灵 - 使用与僵尸相同的模式
     * 参考僵尸的createZombieSprite实现，确保动画播放一致
     */
    createPlantSprite(x, y, entityData) {
        let sprite;
        let textureKey, animKey;
        const scale = 1.0;  // 保持原始大小，避免变形

        // plantType可能是整数（从后端）或字符串
        // PlantType枚举: 0=PEA_SHOOTER, 1=DOUBLE_PEA_SHOOTER, 2=CHERRY_BOMB, 3=WALL_NUT
        let plantType = entityData.plantType;
        if (typeof plantType === 'number') {
            const plantTypeMap = {
                0: 'pea_shooter',
                1: 'double_pea_shooter',
                2: 'cherry_bomb',
                3: 'wall_nut'
            };
            plantType = plantTypeMap[plantType] || 'pea_shooter';
        }
        plantType = plantType || 'pea_shooter';

        // 根据植物类型确定纹理和动画（与僵尸相同的模式）
        switch (plantType) {
            case 'pea_shooter':
                textureKey = 'peashooter';
                animKey = 'peashooter_anim';
                break;
            case 'double_pea_shooter':
            case 'repeater':
                textureKey = 'repeater';
                animKey = 'repeater_anim';
                break;
            case 'cherry_bomb':
                textureKey = 'cherry_bomb';
                animKey = 'cherry_bomb_anim';
                break;
            case 'wall_nut':
                textureKey = 'wallnut';
                animKey = 'wallnut_anim';
                break;
            default:
                textureKey = 'peashooter';
                animKey = 'peashooter_anim';
        }

        // 使用精灵表创建精灵（与僵尸相同的模式）
        if (this.textures.exists(textureKey)) {
            sprite = this.add.sprite(x, y, textureKey, 0);
            if (this.anims.exists(animKey)) {
                sprite.play(animKey);
            }
        } else {
            // 后备精灵
            const fallbackNames = {
                'pea_shooter': '豌豆',
                'double_pea_shooter': '双豌豆',
                'cherry_bomb': '樱桃炸弹',
                'wall_nut': '坚果墙'
            };
            sprite = this.createFallbackPlantSprite(x, y, 0x00ff00, fallbackNames[plantType] || '植物');
            textureKey = 'fallback';
            animKey = null;
        }

        // 设置原点为中心（植物与僵尸不同，植物不需要"站立"效果）
        // 使用中心原点可以让植物动画看起来更稳定，也方便旋转
        sprite.setOrigin(0.5, 0.5);
        sprite.setScale(scale);

        // 记录动画状态key（与僵尸相同的模式，用于避免重复播放动画）
        sprite.setData('plantAnimKey', animKey);
        sprite.setData('plantType', plantType);

        // 根据攻击方向旋转植物
        // 后端Direction枚举: UP=0, DOWN=1, LEFT=2, RIGHT=3, NONE=4
        const attackDirection = entityData.attackDirection !== undefined ? entityData.attackDirection : 3;  // 默认向右(3)
        sprite.setData('attackDirection', attackDirection);

        // 精灵贴图默认发射口朝右
        // 根据攻击方向设置旋转和翻转
        switch (attackDirection) {
            case 0:  // UP - 逆时针旋转90度
                sprite.setRotation(-Math.PI / 2);
                sprite.setFlipX(false);
                break;
            case 1:  // DOWN - 顺时针旋转90度
                sprite.setRotation(Math.PI / 2);
                sprite.setFlipX(false);
                break;
            case 2:  // LEFT - 水平翻转
                sprite.setRotation(0);
                sprite.setFlipX(true);
                break;
            case 3:  // RIGHT - 默认方向
            default:
                sprite.setRotation(0);
                sprite.setFlipX(false);
                break;
        }

        return sprite;
    }

    createFallbackPlantSprite(x, y, color, name) {
        const textureKey = `plant_fallback_${color}`;
        if (!this.textures.exists(textureKey)) {
            const graphics = this.add.graphics();
            graphics.fillStyle(color, 1.0);
            graphics.fillCircle(15, 15, 15);
            graphics.fillStyle(0x228b22, 1.0);
            graphics.fillRect(12, 25, 6, 15);
            graphics.generateTexture(textureKey, 30, 40);
            graphics.destroy();
        }
        const sprite = this.add.sprite(x, y, textureKey);

        // 添加植物名称
        sprite.nameLabel = this.add.text(x, y - 30, name, {
            fontSize: '10px',
            color: '#ffffff',
            backgroundColor: '#000000aa',
            padding: { x: 2, y: 1 }
        }).setOrigin(0.5).setDepth(102);

        return sprite;
    }

    createItemSprite(x, y, entityData) {
        const itemType = entityData.itemType || 'unknown';
        let sprite;

        // 使用实际图片
        if (itemType === 'bucket' && this.textures.exists('item_bucket')) {
            sprite = this.add.sprite(x, y, 'item_bucket');
            sprite.setScale(0.06);  // 1024px * 0.06 ≈ 61px

            // 为铁桶添加黄色防御条（与铁桶僵尸头上的防御条样式一致）
            const armor = entityData.armor || 0;
            const maxArmor = entityData.maxArmor || 200;
            if (armor > 0) {
                const barWidth = 50;  // 与僵尸防御条一致
                const barHeight = 5;  // 与僵尸防御条一致
                const barY = -35;  // 在铁桶上方

                // 背景条（与僵尸防御条样式一致）
                const armorBarBg = this.add.graphics();
                armorBarBg.fillStyle(0x000000, 0.7);
                armorBarBg.fillRect(x - barWidth/2 - 1, y + barY - 1, barWidth + 2, barHeight + 2);
                armorBarBg.setDepth(100);
                sprite.armorBarBg = armorBarBg;

                // 黄色防御条（与僵尸防御条样式一致）
                const armorBar = this.add.graphics();
                armorBar.fillStyle(0xffcc00, 1.0);
                const armorPercent = armor / maxArmor;
                armorBar.fillRect(x - barWidth/2, y + barY, barWidth * armorPercent, barHeight);
                armorBar.setDepth(101);
                sprite.armorBar = armorBar;
            }
        } else if (itemType === 'pole' && this.textures.exists('item_pole')) {
            sprite = this.add.sprite(x, y, 'item_pole');
            sprite.setScale(0.035);  // 2364px * 0.035 ≈ 83px高
        } else if (itemType === 'health_potion' && this.textures.exists('item_health_potion')) {
            sprite = this.add.sprite(x, y, 'item_health_potion');
            sprite.setScale(0.17);  // 357px * 0.17 ≈ 61px高
        } else if (itemType === 'speed_potion' && this.textures.exists('item_speed_potion')) {
            sprite = this.add.sprite(x, y, 'item_speed_potion');
            sprite.setScale(0.03);  // 2048px * 0.03 ≈ 61px
        } else {
            // 其他道具使用生成的图形（后备）
            const textureKey = `item_${itemType}`;

            if (!this.textures.exists(textureKey)) {
                const graphics = this.add.graphics();
                let color = 0xffff00;

                switch (itemType) {
                    case 'potion':
                        color = 0xff00ff;
                        graphics.fillStyle(color, 1.0);
                        graphics.fillCircle(0, 5, 10);
                        graphics.fillRect(-4, -10, 8, 15);
                        break;
                    case 'boost':
                        color = 0xffa500;
                        graphics.fillStyle(color, 1.0);
                        graphics.fillTriangle(0, -15, -12, 10, 12, 10);
                        break;
                    default:
                        graphics.fillStyle(color, 1.0);
                        graphics.fillStar(0, 0, 5, 12, 6);
                }

                graphics.generateTexture(textureKey, 30, 40);
                graphics.destroy();
            }

            sprite = this.add.sprite(x, y, textureKey);
        }

        // 添加闪烁效果
        this.tweens.add({
            targets: sprite,
            alpha: 0.6,
            duration: 500,
            yoyo: true,
            repeat: -1
        });

        // 不显示任何道具标签

        return sprite;
    }

    createProjectileSprite(x, y, entityData) {
        let sprite;

        if (this.textures.exists('pea')) {
            sprite = this.add.sprite(x, y, 'pea');
            sprite.setScale(1.0);  // 增大豌豆尺寸
        } else {
            // 后备：程序化绿色圆形
            if (!this.textures.exists('pea_fallback')) {
                const graphics = this.add.graphics();
                graphics.fillStyle(0x90ee90, 1.0);
                graphics.fillCircle(12, 12, 12);  // 增大后备豌豆
                graphics.fillStyle(0xffffff, 0.5);
                graphics.fillCircle(8, 8, 4);
                graphics.generateTexture('pea_fallback', 24, 24);
                graphics.destroy();
            }
            sprite = this.add.sprite(x, y, 'pea_fallback');
        }

        return sprite;
    }

    createDefaultSprite(x, y, entityData) {
        if (!this.textures.exists('default_entity')) {
            const graphics = this.add.graphics();
            graphics.fillStyle(0xffffff, 1.0);
            graphics.fillCircle(10, 10, 10);
            graphics.generateTexture('default_entity', 20, 20);
            graphics.destroy();
        }
        return this.add.sprite(x, y, 'default_entity');
    }

    updateEntityAnimation(sprite, entityData) {
        // 根据实体状态更新动画
        if (entityData.type === 'zombie') {
            const isMoving = entityData.vx !== 0 || entityData.vy !== 0;
            const isAttacking = entityData.isAttacking;

            if (isAttacking && this.anims.exists('zombie_eat')) {
                if (sprite.anims.currentAnim?.key !== 'zombie_eat') {
                    sprite.play('zombie_eat');
                }
            } else if (isMoving && this.anims.exists('zombie_walk')) {
                if (sprite.anims.currentAnim?.key !== 'zombie_walk') {
                    sprite.play('zombie_walk');
                }
            }

            // 根据移动方向翻转精灵
            if (entityData.vx < 0) {
                sprite.setFlipX(false);
            } else if (entityData.vx > 0) {
                sprite.setFlipX(true);
            }
        } else if (entityData.type === 'plant') {
            // 更新植物动画 - 使用与僵尸相同的模式
            this.updatePlantAnimation(sprite, entityData);
        }
    }

    /**
     * 更新植物动画 - 使用与僵尸完全相同的精灵表动画模式
     * 参考updateZombieAnimation实现，确保动画播放一致
     * 只有当动画需要切换时才调用play()，避免重影
     */
    updatePlantAnimation(sprite, entityData) {
        let plantType = entityData.plantType;
        if (typeof plantType === 'number') {
            const plantTypeMap = {
                0: 'pea_shooter',
                1: 'double_pea_shooter',
                2: 'cherry_bomb',
                3: 'wall_nut'
            };
            plantType = plantTypeMap[plantType] || 'pea_shooter';
        }

        // 确定目标纹理和动画（与僵尸相同的模式）
        let targetTexture, targetAnim;
        const scale = 1.0;  // 保持原始大小，避免变形

        switch (plantType) {
            case 'pea_shooter':
                targetTexture = 'peashooter';
                targetAnim = 'peashooter_anim';
                break;
            case 'double_pea_shooter':
            case 'repeater':
                targetTexture = 'repeater';
                targetAnim = 'repeater_anim';
                break;
            case 'cherry_bomb':
                targetTexture = 'cherry_bomb';
                targetAnim = 'cherry_bomb_anim';
                break;
            case 'wall_nut':
                // 坚果墙需要根据生命值切换纹理
                if (entityData.health && entityData.maxHealth) {
                    const healthPercent = entityData.health / entityData.maxHealth;
                    if (healthPercent < 0.5) {
                        targetTexture = 'wallnut_cracked';
                        targetAnim = 'wallnut_cracked_anim';
                    } else {
                        targetTexture = 'wallnut';
                        targetAnim = 'wallnut_anim';
                    }
                } else {
                    targetTexture = 'wallnut';
                    targetAnim = 'wallnut_anim';
                }
                break;
            default:
                targetTexture = 'peashooter';
                targetAnim = 'peashooter_anim';
        }

        // 使用与僵尸完全相同的动画切换模式
        // 只在动画真正需要切换时才调用play()，避免频繁调用导致卡顿
        const currentAnimKey = sprite.anims.currentAnim ? sprite.anims.currentAnim.key : null;

        // 只有当动画key不同时才切换动画（与僵尸相同）
        if (currentAnimKey !== targetAnim) {
            // 检查纹理是否存在并切换（与僵尸相同）
            if (this.textures.exists(targetTexture)) {
                sprite.setTexture(targetTexture, 0);
                sprite.setScale(scale);
                // 播放新动画
                if (this.anims.exists(targetAnim)) {
                    sprite.play(targetAnim);
                }
            }
        }

        // 植物始终朝右，不需要翻转（与僵尸不同，僵尸根据移动方向翻转）
    }

    updateHealthBar(sprite, entityData) {
        if (!sprite.healthBar || !sprite.healthBarBg) return;

        const { health, maxHealth, armor, maxArmor } = entityData;
        if (health === undefined || maxHealth === undefined) return;

        const barWidth = 50;
        const barHeight = 5;
        const barSpacing = 2;  // 生命条和护甲条之间的间距

        // 根据实体类型计算生命值条位置偏移
        let barOffsetY = 35;
        if (entityData.type === 'zombie') {
            // 僵尸使用底部原点(0.5, 1)，生命值条需要在精灵上方更远处
            // 原始帧高139px，缩放0.9 = 125px高
            barOffsetY = 120;
        } else if (entityData.type === 'plant') {
            // 植物使用中心原点(0.5, 0.5)
            // 原始帧高78px，缩放0.8 = 62px，一半是31px
            barOffsetY = 40;
        } else if (entityData.type === 'dave') {
            // Dave使用底部原点(0.5, 1)，缩放0.15，原图约600px高，显示约90px
            barOffsetY = 95;
        }

        // 使用精灵当前位置
        const x = sprite.x - barWidth / 2;
        const y = sprite.y - barOffsetY;

        // 背景
        sprite.healthBarBg.clear();
        sprite.healthBarBg.fillStyle(0x000000, 0.7);
        sprite.healthBarBg.fillRect(x - 1, y - 1, barWidth + 2, barHeight + 2);

        // 生命值
        sprite.healthBar.clear();
        const healthPercent = Math.max(0, health / maxHealth);
        const color = healthPercent > 0.5 ? 0x00ff00 : healthPercent > 0.25 ? 0xffff00 : 0xff0000;
        sprite.healthBar.fillStyle(color, 1.0);
        sprite.healthBar.fillRect(x, y, barWidth * healthPercent, barHeight);

        // 护甲条（铁桶）- 在生命条上方
        if (sprite.armorBar && sprite.armorBarBg) {
            sprite.armorBarBg.clear();
            sprite.armorBar.clear();

            if (armor !== undefined && maxArmor !== undefined && armor > 0 && maxArmor > 0) {
                const armorY = y - barHeight - barSpacing - 1;  // 在生命条上方

                // 护甲条背景
                sprite.armorBarBg.fillStyle(0x000000, 0.7);
                sprite.armorBarBg.fillRect(x - 1, armorY - 1, barWidth + 2, barHeight + 2);

                // 护甲值 - 黄色
                const armorPercent = Math.max(0, armor / maxArmor);
                sprite.armorBar.fillStyle(0xffcc00, 1.0);  // 黄色
                sprite.armorBar.fillRect(x, armorY, barWidth * armorPercent, barHeight);
            }
        }
    }

    handleGameState(data) {
        // 只在状态真正变化且游戏结束画面未显示时处理
        const newStatus = data.status;

        // 如果游戏结束画面已经显示过，不再处理
        if (this.gameOverShown) {
            return;
        }

        // 只有当状态从 'playing' 变为 'win' 或 'lose' 时才显示游戏结束
        if (this.lastGameStatus === 'playing') {
            if (newStatus === 'win') {
                console.log('游戏胜利！');
                this.gameOverShown = true;
                // 多人模式使用分屏显示
                if (this.isMultiplayerMode && this.splitScreenEnabled) {
                    this.showMultiplayerGameOver({ winner: 'zombie' });  // 僵尸到达出口
                } else {
                    this.showGameOver('胜利！你成功逃出了迷宫！', 0x00ff00);
                }
            } else if (newStatus === 'lose') {
                console.log('游戏失败！');
                this.gameOverShown = true;
                // 多人模式使用分屏显示
                if (this.isMultiplayerMode && this.splitScreenEnabled) {
                    this.showMultiplayerGameOver({ winner: 'dave' });  // 僵尸被击败
                } else {
                    this.showGameOver('失败！被戴夫抓住了！', 0xff0000);
                }
            }
        }

        // 更新上一次的状态
        this.lastGameStatus = newStatus;
    }

    handleGameOver(data) {
        // 如果游戏结束画面已经显示过，不再处理
        if (this.gameOverShown) {
            return;
        }

        console.log('游戏结束:', data);
        this.gameOverShown = true;

        // 多人模式下使用分屏结局显示
        if (this.isMultiplayerMode && this.splitScreenEnabled) {
            this.showMultiplayerGameOver(data);
        } else {
            // 单人模式
            if (data.winner === 'zombie') {
                this.showGameOverWithImage('victory_zombie', '僵尸胜利！');
            } else {
                this.showGameOverWithImage('defeat_zombie', '僵尸失败！');
            }
        }
    }

    /**
     * 多人模式游戏结束显示
     * 根据胜负情况在左右屏幕分别显示不同内容
     */
    showMultiplayerGameOver(data) {
        const screenWidth = this.cameras.main.width;
        const screenHeight = this.cameras.main.height;
        const halfWidth = screenWidth / 2;

        // 停止所有相机跟随
        if (this.daveCamera) this.daveCamera.stopFollow();
        if (this.zombieCamera) this.zombieCamera.stopFollow();
        this.cameras.main.stopFollow();

        // 创建左侧（戴夫）背景
        const leftBg = this.add.graphics();
        leftBg.fillStyle(0x000000, 0.9);
        leftBg.fillRect(0, 0, halfWidth, screenHeight);
        leftBg.setScrollFactor(0);
        leftBg.setDepth(500);

        // 创建右侧（僵尸）背景
        const rightBg = this.add.graphics();
        rightBg.fillStyle(0x000000, 0.9);
        rightBg.fillRect(halfWidth, 0, halfWidth, screenHeight);
        rightBg.setScrollFactor(0);
        rightBg.setDepth(500);

        // 根据胜负情况显示不同内容
        // winner: 'zombie' 表示僵尸到达出口获胜
        // winner: 'dave' 表示僵尸被击败，戴夫获胜
        // winner: 'zombie_killed_dave' 表示僵尸击杀了戴夫（但游戏可能继续）

        let daveText, daveColor, zombieText, zombieColor;

        if (data.winner === 'zombie') {
            // 僵尸到达出口获胜
            daveText = 'DEFEAT';
            daveColor = '#ff0000';  // 红色
            zombieText = 'VICTORY';
            zombieColor = '#00ff00';  // 绿色
        } else if (data.winner === 'dave' || data.winner === 'plants') {
            // 僵尸被击败，戴夫/植物获胜
            daveText = 'VICTORY';
            daveColor = '#ff0000';  // 红色（用户要求）
            zombieText = 'DEFEAT';
            zombieColor = '#00ff00';  // 绿色（用户要求）
        } else {
            // 默认情况
            daveText = 'GAME OVER';
            daveColor = '#ffffff';
            zombieText = 'GAME OVER';
            zombieColor = '#ffffff';
        }

        // 在左侧屏幕中央显示戴夫的结果
        const daveResultText = this.add.text(halfWidth / 2, screenHeight / 2, daveText, {
            fontSize: '72px',
            color: daveColor,
            fontStyle: 'bold',
            stroke: '#000000',
            strokeThickness: 8
        });
        daveResultText.setOrigin(0.5);
        daveResultText.setScrollFactor(0);
        daveResultText.setDepth(501);

        // 在右侧屏幕中央显示僵尸的结果
        const zombieResultText = this.add.text(halfWidth + halfWidth / 2, screenHeight / 2, zombieText, {
            fontSize: '72px',
            color: zombieColor,
            fontStyle: 'bold',
            stroke: '#000000',
            strokeThickness: 8
        });
        zombieResultText.setOrigin(0.5);
        zombieResultText.setScrollFactor(0);
        zombieResultText.setDepth(501);

        // 底部操作提示（两边都显示）
        const hintY = screenHeight - 60;
        const leftHint = this.add.text(halfWidth / 2, hintY, 'Enter 再来一局 | ESC 返回主菜单', {
            fontSize: '18px',
            color: '#cccccc',
            fontStyle: 'bold'
        });
        leftHint.setOrigin(0.5);
        leftHint.setScrollFactor(0);
        leftHint.setDepth(502);

        const rightHint = this.add.text(halfWidth + halfWidth / 2, hintY, 'Enter 再来一局 | ESC 返回主菜单', {
            fontSize: '18px',
            color: '#cccccc',
            fontStyle: 'bold'
        });
        rightHint.setOrigin(0.5);
        rightHint.setScrollFactor(0);
        rightHint.setDepth(502);

        // 添加键盘监听
        this.input.keyboard.once('keydown-ENTER', () => {
            window.location.reload();
        });

        this.input.keyboard.once('keydown-ESC', () => {
            this.events.emit('returnToMenu');
        });

        // 触发游戏结束事件
        this.events.emit('gameOver');
    }

    /**
     * 显示游戏结束画面（带图片）
     * @param {string} imageKey - 图片资源键名
     * @param {string} fallbackText - 图片加载失败时的备用文字
     */
    showGameOverWithImage(imageKey, fallbackText) {
        // 停止相机跟随
        this.cameras.main.stopFollow();

        // 获取相机视口中心位置
        const centerX = this.cameras.main.scrollX + this.cameras.main.width / 2;
        const centerY = this.cameras.main.scrollY + this.cameras.main.height / 2;
        const screenWidth = this.cameras.main.width;
        const screenHeight = this.cameras.main.height;

        // 创建黑色背景覆盖全屏（使用相机位置）
        const bg = this.add.graphics();
        bg.fillStyle(0x000000, 1);
        bg.fillRect(
            this.cameras.main.scrollX,
            this.cameras.main.scrollY,
            screenWidth,
            screenHeight
        );
        bg.setDepth(500);

        // 尝试显示图片
        if (this.textures.exists(imageKey)) {
            const image = this.add.image(centerX, centerY - 30, imageKey);
            image.setDepth(501);

            // 计算缩放以适应屏幕（保持宽高比，不拉伸）
            // 限制图片最大显示为屏幕的60%，保持居中
            const maxWidth = screenWidth * 0.6;
            const maxHeight = screenHeight * 0.6;
            const scaleX = maxWidth / image.width;
            const scaleY = maxHeight / image.height;
            const scale = Math.min(scaleX, scaleY, 1); // 不超过原始大小

            image.setScale(scale);

            // 添加操作提示
            const hintY = centerY + screenHeight / 2 - 60;
            const hintText = this.add.text(centerX, hintY, '按 Enter 再来一局  |  按 ESC 返回主菜单', {
                fontSize: '20px',
                color: '#cccccc',
                fontStyle: 'bold'
            });
            hintText.setOrigin(0.5);
            hintText.setDepth(503);
        } else {
            // 图片不存在时使用文字显示
            const isVictory = imageKey.includes('victory');
            const color = isVictory ? 0x00ff00 : 0xff0000;
            const gameOverText = this.add.text(centerX, centerY - 50, fallbackText, {
                fontSize: '64px',
                color: `#${color.toString(16).padStart(6, '0')}`,
                fontStyle: 'bold',
                stroke: '#000000',
                strokeThickness: 6
            });
            gameOverText.setOrigin(0.5);
            gameOverText.setDepth(501);

            const hintText = this.add.text(centerX, centerY + 50, '按 Enter 再来一局  |  按 ESC 返回主菜单', {
                fontSize: '20px',
                color: '#cccccc'
            });
            hintText.setOrigin(0.5);
            hintText.setDepth(501);
        }

        // 添加键盘监听
        this.input.keyboard.once('keydown-ENTER', () => {
            window.location.reload();
        });

        this.input.keyboard.once('keydown-ESC', () => {
            // 触发返回主菜单事件
            this.events.emit('returnToMenu');
        });
    }

    // 保留旧方法以兼容
    showGameOver(text, color) {
        const imageKey = color === 0x00ff00 ? 'victory_zombie' : 'defeat_zombie';
        this.showGameOverWithImage(imageKey, text);
    }

    update(time, delta) {
        // 定期检查键盘焦点（每秒检查一次）
        if (!this.lastFocusCheck || time - this.lastFocusCheck > 1000) {
            this.lastFocusCheck = time;
            if (this.input.keyboard && !this.input.keyboard.enabled) {
                this.input.keyboard.enabled = true;
                console.log('重新启用键盘输入');
            }
        }

        // 平滑插值系数 - 值越小越平滑，但响应越慢
        const lerpFactor = 0.3;

        // 更新所有实体的位置（使用插值平滑）和UI
        this.entities.forEach((sprite, id) => {
            if (!sprite || !sprite.active) return;

            const entityData = sprite.getData('entityData');
            if (!entityData) return;

            // 植物不需要位置插值（它们不移动）
            // 参考原版PVZ，植物位置固定不变
            if (entityData.type === 'plant') {
                // 植物只需要在创建时设置位置，不需要每帧更新
                // 这样可以避免动画看起来像在"走动"
            } else {
                // 其他实体使用lerp平滑插值位置
                const targetX = entityData.x;
                const targetY = entityData.y;
                const currentX = sprite.x;
                const currentY = sprite.y;

                // 计算距离，如果太远则直接跳转（防止初始化或瞬移时的问题）
                const dx = targetX - currentX;
                const dy = targetY - currentY;
                const distance = Math.sqrt(dx * dx + dy * dy);

                if (distance > 100) {
                    // 距离太远，直接设置位置
                    sprite.setPosition(targetX, targetY);
                } else if (distance > 0.5) {
                    // 使用lerp平滑移动
                    const newX = currentX + dx * lerpFactor;
                    const newY = currentY + dy * lerpFactor;
                    sprite.setPosition(newX, newY);
                }
            }

            // 根据实体类型计算标签偏移
            let labelOffsetY = 50;
            if (entityData.type === 'zombie') {
                // 僵尸使用底部原点(0.5, 1)，标签需要在精灵上方更远处
                labelOffsetY = 135;
            } else if (entityData.type === 'plant') {
                // 植物使用中心原点(0.5, 0.5)
                labelOffsetY = 50;
            } else if (entityData.type === 'dave') {
                labelOffsetY = 75;
                // 更新Dave的动画状态
                this.updateDaveAnimation(sprite, entityData);
                // 存储Dave数据用于种子包UI更新
                this.storeDaveData(entityData);
            }

            // 更新名称标签位置
            if (sprite.nameLabel) {
                sprite.nameLabel.setPosition(sprite.x, sprite.y - labelOffsetY);
            }

            // 更新生命值条位置
            this.updateHealthBar(sprite, entityData);
        });

        // 实时更新小地图上的动态元素
        if (this.minimapVisible && this.currentMinimap) {
            this.updateMinimapDynamicElements(this.currentMinimap);
        }

        // 处理输入
        this.handleInput(delta);
    }

    handleInput(delta) {
        // 网络相关操作需要连接
        if (!this.networkClient || !this.networkClient.connected) {
            return;
        }

        // 安全检查：确保键盘已初始化
        if (!this.keys || !this.cursors) {
            console.warn('键盘未初始化，跳过输入处理');
            return;
        }

        // ==================== 僵尸控制 ====================
        // 单人模式：WASD + 方向键 + 小键盘方向键 都可以控制僵尸
        // 多人模式：只用方向键 + 小键盘方向键
        let zombieMoveDirection = null;

        // 在单人模式下，WASD也可以控制僵尸
        const wasdUp = !this.isMultiplayerMode && this.keys.W && this.keys.W.isDown;
        const wasdDown = !this.isMultiplayerMode && this.keys.S && this.keys.S.isDown;
        const wasdLeft = !this.isMultiplayerMode && this.keys.A && this.keys.A.isDown;
        const wasdRight = !this.isMultiplayerMode && this.keys.D && this.keys.D.isDown;

        const zombieUpPressed = (this.cursors.up && this.cursors.up.isDown) || (this.keys.NUMPAD_UP && this.keys.NUMPAD_UP.isDown) || wasdUp;
        const zombieDownPressed = (this.cursors.down && this.cursors.down.isDown) || (this.keys.NUMPAD_DOWN && this.keys.NUMPAD_DOWN.isDown) || wasdDown;
        const zombieLeftPressed = (this.cursors.left && this.cursors.left.isDown) || (this.keys.NUMPAD_LEFT && this.keys.NUMPAD_LEFT.isDown) || wasdLeft;
        const zombieRightPressed = (this.cursors.right && this.cursors.right.isDown) || (this.keys.NUMPAD_RIGHT && this.keys.NUMPAD_RIGHT.isDown) || wasdRight;

        // 发送僵尸移动指令
        if (zombieUpPressed) {
            this.networkClient.send('MOVE_UP', {});
            zombieMoveDirection = 'up';
        } else if (zombieDownPressed) {
            this.networkClient.send('MOVE_DOWN', {});
            zombieMoveDirection = 'down';
        }

        if (zombieLeftPressed) {
            this.networkClient.send('MOVE_LEFT', {});
            zombieMoveDirection = zombieMoveDirection ? zombieMoveDirection + '_left' : 'left';
        } else if (zombieRightPressed) {
            this.networkClient.send('MOVE_RIGHT', {});
            zombieMoveDirection = zombieMoveDirection ? zombieMoveDirection + '_right' : 'right';
        }

        // 如果僵尸没有按任何移动键，发送停止
        if (!zombieUpPressed && !zombieDownPressed && !zombieLeftPressed && !zombieRightPressed) {
            if (this.lastMoveDirection !== null) {
                this.networkClient.send('STOP_MOVE', {});
                this.lastMoveDirection = null;
            }
        } else {
            this.lastMoveDirection = zombieMoveDirection;
        }

        // ==================== 戴夫控制（多人模式） ====================
        // 使用WASD键
        if (this.isMultiplayerMode) {
            let daveMoveDirection = null;

            const daveUpPressed = this.keys.W && this.keys.W.isDown;
            const daveDownPressed = this.keys.S && this.keys.S.isDown;
            const daveLeftPressed = this.keys.A && this.keys.A.isDown;
            const daveRightPressed = this.keys.D && this.keys.D.isDown;

            // 发送戴夫移动指令
            if (daveUpPressed) {
                this.networkClient.send('DAVE_MOVE_UP', {});
                daveMoveDirection = 'up';
            } else if (daveDownPressed) {
                this.networkClient.send('DAVE_MOVE_DOWN', {});
                daveMoveDirection = 'down';
            }

            if (daveLeftPressed) {
                this.networkClient.send('DAVE_MOVE_LEFT', {});
                daveMoveDirection = daveMoveDirection ? daveMoveDirection + '_left' : 'left';
            } else if (daveRightPressed) {
                this.networkClient.send('DAVE_MOVE_RIGHT', {});
                daveMoveDirection = daveMoveDirection ? daveMoveDirection + '_right' : 'right';
            }

            // 如果戴夫没有按任何移动键，发送停止
            if (!daveUpPressed && !daveDownPressed && !daveLeftPressed && !daveRightPressed) {
                if (this.lastDaveMoveDirection !== null) {
                    this.networkClient.send('DAVE_STOP_MOVE', {});
                    this.lastDaveMoveDirection = null;
                }
            } else {
                this.lastDaveMoveDirection = daveMoveDirection;
            }

            // Q键 - 打开种植菜单
            if (Phaser.Input.Keyboard.JustDown(this.keys.Q)) {
                this.networkClient.send('DAVE_PLANT_MENU', {});
                console.log('打开种植菜单');
            }
        }

        // ==================== 撑杆跳（僵尸技能） ====================
        if (this.poleVaultCooldown > 0) {
            this.poleVaultCooldown -= delta;
        }
        if (this.keys.CTRL.isDown && this.poleVaultCooldown <= 0) {
            this.networkClient.send('POLE_VAULT', {});
            console.log('撑杆跳!');
            this.poleVaultCooldown = 500; // 500ms冷却
        }

        // ==================== 空格攻击 ====================
        if (Phaser.Input.Keyboard.JustDown(this.keys.SPACE)) {
            this.networkClient.send('ATTACK', {});
            console.log('攻击!');
        }
    }

    /**
     * 启用多人模式
     */
    enableMultiplayerMode() {
        console.log('===== enableMultiplayerMode() 被调用 =====');
        this.isMultiplayerMode = true;
        // 通知后端启用戴夫玩家控制
        console.log('networkClient:', this.networkClient);
        console.log('networkClient.connected:', this.networkClient ? this.networkClient.connected : 'N/A');
        if (this.networkClient && this.networkClient.connected) {
            this.networkClient.send('ENABLE_DAVE_PLAYER', {});
            console.log('已发送 ENABLE_DAVE_PLAYER 消息到后端');
        } else {
            console.warn('无法发送 ENABLE_DAVE_PLAYER：网络未连接');
        }
        // 启用分屏
        this.enableSplitScreen();
        console.log('分屏已启用');
        // 种植菜单默认隐藏，按Q键显示
        this.seedPacketVisible = false;
        console.log('种植菜单将在按Q键时显示');
        // 设置种植点击处理
        this.setupPlantingClickHandler();
        console.log('种植点击处理已设置');
        console.log('===== 多人模式已完全启用 =====');
    }

    /**
     * 禁用多人模式
     */
    disableMultiplayerMode() {
        this.isMultiplayerMode = false;
        // 通知后端禁用戴夫玩家控制
        if (this.networkClient && this.networkClient.connected) {
            this.networkClient.send('DISABLE_DAVE_PLAYER', {});
        }
        // 禁用分屏
        this.disableSplitScreen();
        // 隐藏种子包UI
        this.hideSeedPacketUI();
        console.log('多人模式已禁用');
    }

    /**
     * 启用分屏模式
     */
    enableSplitScreen() {
        if (this.splitScreenEnabled) return;

        const screenWidth = this.cameras.main.width;
        const screenHeight = this.cameras.main.height;

        // 隐藏主摄像机（或调整为仅显示UI）
        this.cameras.main.setVisible(false);

        // 创建戴夫视角摄像机（左半屏）
        this.daveCamera = this.cameras.add(0, 0, screenWidth / 2, screenHeight, false, 'daveCamera');
        this.daveCamera.setZoom(1.2);
        this.daveCamera.setBackgroundColor('#1a4d1a');
        if (this.mazeData) {
            this.daveCamera.setBounds(0, 0, this.mazeData.pixelWidth, this.mazeData.pixelHeight);
        }

        // 创建僵尸视角摄像机（右半屏）
        this.zombieCamera = this.cameras.add(screenWidth / 2, 0, screenWidth / 2, screenHeight, false, 'zombieCamera');
        this.zombieCamera.setZoom(1.2);
        this.zombieCamera.setBackgroundColor('#1a4d1a');
        if (this.mazeData) {
            this.zombieCamera.setBounds(0, 0, this.mazeData.pixelWidth, this.mazeData.pixelHeight);
        }

        // 设置摄像机跟随
        if (this.daveSprite) {
            this.daveCamera.startFollow(this.daveSprite, true, 0.1, 0.1);
            console.log('戴夫摄像机已跟随戴夫精灵');
        } else {
            console.log('警告：戴夫精灵尚未创建，摄像机稍后跟随');
        }
        if (this.zombieSprite) {
            this.zombieCamera.startFollow(this.zombieSprite, true, 0.1, 0.1);
            console.log('僵尸摄像机已跟随僵尸精灵');
        } else {
            console.log('警告：僵尸精灵尚未创建，摄像机稍后跟随');
        }

        // 添加分屏分隔线
        this.splitLine = this.add.graphics();
        this.splitLine.fillStyle(0x000000, 1);
        this.splitLine.fillRect(screenWidth / 2 - 2, 0, 4, screenHeight);
        this.splitLine.setScrollFactor(0);  // 固定在屏幕上
        this.splitLine.setDepth(1000);

        // 让僵尸摄像机忽略种植菜单UI（只显示在左侧戴夫屏幕）
        if (this.seedPacketContainer && this.zombieCamera) {
            this.zombieCamera.ignore(this.seedPacketContainer);
            // 还需要忽略容器内的所有子对象
            this.seedPacketContainer.list.forEach(child => {
                this.zombieCamera.ignore(child);
            });
        }

        this.splitScreenEnabled = true;
        console.log('分屏模式已启用');
    }

    /**
     * 禁用分屏模式
     */
    disableSplitScreen() {
        if (!this.splitScreenEnabled) return;

        // 移除分屏摄像机
        if (this.daveCamera) {
            this.cameras.remove(this.daveCamera);
            this.daveCamera = null;
        }
        if (this.zombieCamera) {
            this.cameras.remove(this.zombieCamera);
            this.zombieCamera = null;
        }

        // 移除分屏分隔线
        if (this.splitLine) {
            this.splitLine.destroy();
            this.splitLine = null;
        }

        // 恢复主摄像机
        this.cameras.main.setVisible(true);
        if (this.zombieSprite) {
            this.cameras.main.startFollow(this.zombieSprite, true, 0.1, 0.1);
        }

        this.splitScreenEnabled = false;
        console.log('分屏模式已禁用');
    }

    /**
     * 更新分屏摄像机跟随
     */
    updateSplitScreenCameras() {
        if (!this.splitScreenEnabled) return;

        // 确保戴夫摄像机跟随戴夫精灵
        if (this.daveCamera && this.daveSprite && !this.daveCamera._follow) {
            this.daveCamera.startFollow(this.daveSprite, true, 0.1, 0.1);
        }

        // 确保僵尸摄像机跟随僵尸精灵
        if (this.zombieCamera && this.zombieSprite && !this.zombieCamera._follow) {
            this.zombieCamera.startFollow(this.zombieSprite, true, 0.1, 0.1);
        }
    }

    // ==================== 小地图系统 ====================

    /**
     * 创建小地图
     * @param {string} viewType - 'dave' 或 'zombie'，决定显示内容
     * @param {number} x - 小地图位置X
     * @param {number} y - 小地图位置Y
     */
    createMinimap(viewType = 'zombie', x = 20, y = 20, forCamera = null) {
        if (!this.maze) {
            console.log('无法创建小地图：迷宫数据不存在');
            return null;
        }

        console.log('创建小地图，viewType:', viewType, '分屏模式:', this.splitScreenEnabled);
        console.log('迷宫数据:', this.maze.gridWidth, 'x', this.maze.gridHeight, 'cellSize:', this.maze.cellSize);

        // 获取屏幕尺寸用于计算小地图大小
        const fullScreenWidth = this.cameras.main.width;
        const screenWidth = this.splitScreenEnabled ? fullScreenWidth / 2 : fullScreenWidth;
        const screenHeight = this.cameras.main.height;

        // 小地图覆盖屏幕的80%
        const targetSize = Math.min(screenWidth, screenHeight) * 0.8;

        // 根据迷宫格子数计算缩放比例
        const cellSize = this.maze.cellSize || 50;
        const gridWidth = this.maze.gridWidth;
        const gridHeight = this.maze.gridHeight;

        // 计算单个格子在小地图上的大小
        const cellDisplaySize = targetSize / Math.max(gridWidth, gridHeight);

        // 小地图实际尺寸
        const minimapWidth = gridWidth * cellDisplaySize;
        const minimapHeight = gridHeight * cellDisplaySize;

        // 计算居中位置
        let centerX, centerY;
        if (this.splitScreenEnabled) {
            if (viewType === 'dave') {
                centerX = (screenWidth - minimapWidth) / 2;
            } else {
                centerX = screenWidth + (screenWidth - minimapWidth) / 2;
            }
            centerY = (screenHeight - minimapHeight) / 2;
        } else {
            centerX = (screenWidth - minimapWidth) / 2;
            centerY = (screenHeight - minimapHeight) / 2;
        }

        // 创建小地图容器
        const minimap = this.add.container(centerX, centerY);
        minimap.setScrollFactor(0);
        minimap.setDepth(999);

        // 保存小地图参数用于实时更新
        minimap.cellDisplaySize = cellDisplaySize;
        minimap.cellSize = cellSize;
        minimap.viewType = viewType;
        minimap.minimapWidth = minimapWidth;
        minimap.minimapHeight = minimapHeight;

        // 背景 - 半透明黑色背景
        const bg = this.add.graphics();
        bg.fillStyle(0x000000, 0.7);
        bg.fillRect(-5, -5, minimapWidth + 10, minimapHeight + 10);
        minimap.add(bg);

        // 使用grid数据绘制迷宫
        // 格子类型: 0=墙壁, 1=通道, 2=入口, 3=出口, 4=道具点
        const mazeGraphics = this.add.graphics();

        console.log('===== 小地图绘制 =====');
        console.log('maze.grid存在:', !!this.maze.grid);
        console.log('gridWidth:', gridWidth, 'gridHeight:', gridHeight);

        if (this.maze.grid && this.maze.grid.length > 0) {
            // 统计各类型格子数量
            let stats = {wall: 0, path: 0, entrance: 0, exit: 0, item: 0, unknown: 0};
            for (let y = 0; y < gridHeight; y++) {
                for (let x = 0; x < gridWidth; x++) {
                    const cellType = this.maze.grid[y][x];
                    const drawX = x * cellDisplaySize;
                    const drawY = y * cellDisplaySize;

                    // 根据格子类型选择颜色
                    if (cellType === 0) {
                        // 墙壁 - 深灰色
                        mazeGraphics.fillStyle(0x333333, 1);
                        stats.wall++;
                    } else if (cellType === 1 || cellType === 4) {
                        // 通道/道具点 - 棕色
                        mazeGraphics.fillStyle(0x8B6914, 1);
                        if (cellType === 1) stats.path++; else stats.item++;
                    } else if (cellType === 2) {
                        // 入口 - 红色
                        mazeGraphics.fillStyle(0xcc3333, 1);
                        stats.entrance++;
                    } else if (cellType === 3) {
                        // 出口 - 绿色
                        mazeGraphics.fillStyle(0x33cc33, 1);
                        stats.exit++;
                    } else {
                        // 未知 - 黑色
                        mazeGraphics.fillStyle(0x000000, 1);
                        stats.unknown++;
                    }

                    mazeGraphics.fillRect(drawX, drawY, cellDisplaySize, cellDisplaySize);
                }
            }
            console.log('小地图格子统计:', stats);
        } else {
            console.log('警告：迷宫grid数据不存在');
        }
        minimap.add(mazeGraphics);

        // 保存偏移量用于动态元素更新（这里偏移是0因为从0,0开始绘制）
        minimap.offsetX = 0;
        minimap.offsetY = 0;

        // 创建动态元素图形层（用于实时更新角色位置）
        const dynamicGraphics = this.add.graphics();
        minimap.dynamicGraphics = dynamicGraphics;
        minimap.add(dynamicGraphics);

        // 初始绘制动态元素
        this.updateMinimapDynamicElements(minimap);

        // 边框
        const border = this.add.graphics();
        border.lineStyle(3, 0xffffff, 1);
        border.strokeRect(-2, -2, minimapWidth + 4, minimapHeight + 4);
        minimap.add(border);

        return minimap;
    }

    /**
     * 更新小地图上的动态元素（位置实时变化的元素）
     */
    updateMinimapDynamicElements(minimap) {
        if (!minimap || !minimap.dynamicGraphics) return;

        const graphics = minimap.dynamicGraphics;
        const cellDisplaySize = minimap.cellDisplaySize;
        const cellSize = minimap.cellSize || this.maze?.cellSize || 50;
        const viewType = minimap.viewType;

        // 将像素坐标转换为小地图坐标
        const toMinimapX = (pixelX) => (pixelX / cellSize) * cellDisplaySize;
        const toMinimapY = (pixelY) => (pixelY / cellSize) * cellDisplaySize;

        graphics.clear();

        if (viewType === 'dave') {
            // 戴夫小地图：显示僵尸位置、植物位置

            // 显示植物位置
            this.entities.forEach((sprite, id) => {
                const data = sprite.getData('entityData');
                if (data && data.type === 'plant') {
                    const plantX = toMinimapX(sprite.x);
                    const plantY = toMinimapY(sprite.y);
                    graphics.fillStyle(0x00ff00, 1);  // 绿色植物
                    graphics.fillRect(plantX - 3, plantY - 3, 6, 6);
                }
            });

            // 显示僵尸位置（戴夫需要知道僵尸在哪）
            if (this.zombieSprite) {
                const zombieX = toMinimapX(this.zombieSprite.x);
                const zombieY = toMinimapY(this.zombieSprite.y);
                graphics.fillStyle(0xff00ff, 1);  // 紫色僵尸（敌人）
                graphics.fillCircle(zombieX, zombieY, 6);
            }

            // 显示自己的位置（戴夫）- 最后绘制确保在最上层
            if (this.daveSprite) {
                const selfX = toMinimapX(this.daveSprite.x);
                const selfY = toMinimapY(this.daveSprite.y);
                graphics.fillStyle(0x00ffff, 1);  // 青色自己
                graphics.fillCircle(selfX, selfY, 8);
                // 添加白色边框让自己更明显
                graphics.lineStyle(2, 0xffffff, 1);
                graphics.strokeCircle(selfX, selfY, 8);
            }
        } else {
            // 僵尸小地图：显示道具位置和出口

            // 显示道具位置
            this.entities.forEach((sprite, id) => {
                const data = sprite.getData('entityData');
                if (data && data.type === 'item') {
                    const itemX = toMinimapX(sprite.x);
                    const itemY = toMinimapY(sprite.y);
                    graphics.fillStyle(0xffff00, 1);  // 黄色道具
                    graphics.fillCircle(itemX, itemY, 5);
                }
            });

            // 显示自己的位置（僵尸）
            if (this.zombieSprite) {
                const selfX = toMinimapX(this.zombieSprite.x);
                const selfY = toMinimapY(this.zombieSprite.y);
                graphics.fillStyle(0x00ffff, 1);  // 青色自己
                graphics.fillCircle(selfX, selfY, 8);
                // 添加白色边框让自己更明显
                graphics.lineStyle(2, 0xffffff, 1);
                graphics.strokeCircle(selfX, selfY, 8);
            }
        }
    }

    /**
     * 显示小地图
     * @param {string} viewType - 'dave' 或 'zombie'
     */
    showMinimap(viewType = 'zombie') {
        // 如果已有小地图，先关闭
        this.hideMinimap();

        const screenWidth = this.cameras.main.width;
        const screenHeight = this.cameras.main.height;

        if (this.splitScreenEnabled) {
            // 分屏模式：小地图只在对应的屏幕显示
            const halfWidth = screenWidth / 2;

            if (viewType === 'dave') {
                // 戴夫小地图 - 居中显示在左半屏
                this.currentMinimap = this.createMinimap('dave', 20, 20, this.daveCamera);

                // 让僵尸摄像机忽略戴夫的小地图（只在左屏显示）
                if (this.zombieCamera && this.currentMinimap) {
                    this.zombieCamera.ignore(this.currentMinimap);
                    // 还需要忽略小地图容器内的所有子对象
                    this.currentMinimap.list.forEach(child => {
                        this.zombieCamera.ignore(child);
                    });
                }
            } else {
                // 僵尸小地图 - 居中显示在右半屏
                // 位置需要考虑右半屏的起始位置
                this.currentMinimap = this.createMinimap('zombie', halfWidth + 20, 20, this.zombieCamera);

                // 让戴夫摄像机忽略僵尸的小地图（只在右屏显示）
                if (this.daveCamera && this.currentMinimap) {
                    this.daveCamera.ignore(this.currentMinimap);
                    // 还需要忽略小地图容器内的所有子对象
                    this.currentMinimap.list.forEach(child => {
                        this.daveCamera.ignore(child);
                    });
                }
            }
        } else {
            // 单人模式 - 居中显示
            this.currentMinimap = this.createMinimap(viewType, 20, 20);
        }

        this.minimapVisible = true;
    }

    /**
     * 隐藏小地图
     */
    hideMinimap() {
        if (this.currentMinimap) {
            this.currentMinimap.destroy();
            this.currentMinimap = null;
        }
        this.minimapVisible = false;
    }

    /**
     * 切换小地图显示
     * @param {string} viewType - 'dave' 或 'zombie'
     */
    toggleMinimap(viewType = 'zombie') {
        if (this.minimapVisible) {
            this.hideMinimap();
        } else {
            this.showMinimap(viewType);
        }

        // 确保键盘焦点不丢失
        this.time.delayedCall(50, () => {
            this.input.keyboard.enabled = true;
            if (this.game.canvas) {
                this.game.canvas.focus();
            }
        });
    }

    // ==================== 种子包UI ====================

    /**
     * 创建种子包UI（戴夫的植物选择界面）
     */
    createSeedPacketUI() {
        // 种子包配置
        this.seedPackets = [
            { key: 'seedpacket_peashooter', name: '豌豆射手', cost: 100, cooldownKey: 'currentPeaShooterCooldown', maxCooldown: 10 },
            { key: 'seedpacket_repeater', name: '双发射手', cost: 200, cooldownKey: 'currentRepeaterCooldown', maxCooldown: 20 },
            { key: 'seedpacket_cherry_bomb', name: '樱桃炸弹', cost: 150, cooldownKey: 'currentCherryBombCooldown', maxCooldown: 30 },
            { key: 'seedpacket_wallnut', name: '坚果墙', cost: 50, cooldownKey: 'currentWallNutCooldown', maxCooldown: 20 }
        ];

        // UI容器（固定在屏幕上）
        this.seedPacketContainer = this.add.container(0, 0);
        this.seedPacketContainer.setScrollFactor(0);
        this.seedPacketContainer.setDepth(900);

        // 默认隐藏，只在多人模式按Q显示
        this.seedPacketContainer.setVisible(false);

        // 当前选中的植物索引（-1表示未选中）
        this.selectedPlantIndex = -1;

        // 种子包卡片配置 - 0.75倍大小
        this.seedPacketCards = [];
        this.seedPacketCooldownOverlays = [];

        const cardWidth = 90;   // 120 * 0.75
        const cardHeight = 120; // 160 * 0.75
        const cardSpacing = 9;  // 12 * 0.75
        const numCards = this.seedPackets.length;
        const totalCardsWidth = numCards * cardWidth + (numCards - 1) * cardSpacing;

        // 棕色背景框 - 根据卡片大小自适应
        const bgPadding = 12;
        const bgWidth = totalCardsWidth + bgPadding * 2;
        const bgHeight = cardHeight + bgPadding * 2;
        const bgX = 20;
        const bgY = 20;

        const bg = this.add.graphics();
        bg.fillStyle(0x3d2817, 0.95);  // 棕色木质背景
        bg.fillRoundedRect(bgX, bgY, bgWidth, bgHeight, 10);
        bg.lineStyle(2, 0x5a4032);
        bg.strokeRoundedRect(bgX, bgY, bgWidth, bgHeight, 10);
        this.seedPacketContainer.add(bg);

        const cardStartX = bgX + bgPadding;
        const cardY = bgY + bgPadding;

        for (let i = 0; i < this.seedPackets.length; i++) {
            const packet = this.seedPackets[i];
            const cardX = cardStartX + i * (cardWidth + cardSpacing);

            // 卡片背景
            const cardBg = this.add.graphics();
            cardBg.fillStyle(0x2d6b22, 1);  // 绿色背景
            cardBg.fillRoundedRect(cardX, cardY, cardWidth, cardHeight, 6);
            cardBg.lineStyle(2, 0x1a4d13);
            cardBg.strokeRoundedRect(cardX, cardY, cardWidth, cardHeight, 6);
            this.seedPacketContainer.add(cardBg);

            // 种子包图片 - 填满卡片 (先添加图片，这样hitArea在最上面)
            let img = null;
            if (this.textures.exists(packet.key)) {
                img = this.add.image(cardX + cardWidth / 2, cardY + cardHeight / 2, packet.key);
                const scaleX = (cardWidth - 12) / img.width;
                const scaleY = (cardHeight - 12) / img.height;
                const scale = Math.min(scaleX, scaleY);
                img.setScale(scale);
                this.seedPacketContainer.add(img);
            }

            // 选中高亮框（初始隐藏）
            const selectBorder = this.add.graphics();
            selectBorder.lineStyle(3, 0xffff00, 1);  // 黄色边框
            selectBorder.strokeRoundedRect(cardX - 2, cardY - 2, cardWidth + 4, cardHeight + 4, 8);
            selectBorder.setVisible(false);
            this.seedPacketContainer.add(selectBorder);
            packet.selectBorder = selectBorder;
            packet.cardBg = cardBg;

            // 创建可点击区域 - 最后添加，确保在最上层
            const hitArea = this.add.rectangle(
                cardX + cardWidth / 2,
                cardY + cardHeight / 2,
                cardWidth,
                cardHeight,
                0x000000,  // 给一个颜色
                0  // 完全透明
            );
            hitArea.setInteractive({ useHandCursor: true });
            this.seedPacketContainer.add(hitArea);

            // 点击选择植物
            const plantIndex = i;
            hitArea.on('pointerdown', (pointer) => {
                console.log('种子包点击，索引:', plantIndex);
                // 设置标志，防止全局点击处理器立即触发种植
                this.justClickedSeedPacket = true;
                this.time.delayedCall(100, () => {
                    this.justClickedSeedPacket = false;
                });
                this.selectPlant(plantIndex);
                // 阻止事件传播
                pointer.event.stopPropagation();
            });

            // 冷却遮罩（初始隐藏）
            const cooldownOverlay = this.add.graphics();
            cooldownOverlay.setVisible(false);
            this.seedPacketContainer.add(cooldownOverlay);
            this.seedPacketCooldownOverlays.push({
                graphics: cooldownOverlay,
                x: cardX,
                y: cardY,
                width: cardWidth,
                height: cardHeight,
                maxCooldown: packet.maxCooldown
            });

            this.seedPacketCards.push({
                x: cardX,
                y: cardY,
                width: cardWidth,
                height: cardHeight
            });
        }

        console.log('种子包UI创建完成');
    }

    /**
     * 显示种子包UI
     */
    showSeedPacketUI() {
        if (this.seedPacketContainer) {
            this.seedPacketContainer.setVisible(true);
            this.seedPacketVisible = true;
        }
    }

    /**
     * 隐藏种子包UI
     */
    hideSeedPacketUI() {
        if (this.seedPacketContainer) {
            this.seedPacketContainer.setVisible(false);
            this.seedPacketVisible = false;
        }
    }

    /**
     * 切换种子包UI显示/隐藏
     */
    toggleSeedPacketUI() {
        if (this.seedPacketVisible) {
            this.hideSeedPacketUI();
        } else {
            this.showSeedPacketUI();
        }

        // 确保键盘焦点不丢失
        this.time.delayedCall(50, () => {
            this.input.keyboard.enabled = true;
            if (this.game.canvas) {
                this.game.canvas.focus();
            }
        });
    }

    /**
     * 更新种子包UI
     * @param {object} daveData - 戴夫的实体数据（包含sunlight和cooldown）
     */
    updateSeedPacketUI(daveData) {
        if (!this.seedPacketContainer || !daveData) return;
        if (!this.seedPacketCooldownOverlays || this.seedPacketCooldownOverlays.length === 0) return;

        // 更新冷却遮罩
        for (let i = 0; i < this.seedPackets.length; i++) {
            const packet = this.seedPackets[i];
            const overlay = this.seedPacketCooldownOverlays[i];
            if (!overlay || !overlay.graphics) continue;

            // 获取当前冷却值（从Dave数据中读取）
            const currentCooldown = daveData[packet.cooldownKey] || 0;

            if (currentCooldown > 0) {
                // 显示冷却遮罩
                overlay.graphics.setVisible(true);
                overlay.graphics.clear();

                // 计算冷却百分比
                const cooldownPercent = Math.min(currentCooldown / overlay.maxCooldown, 1);
                const fillHeight = overlay.height * cooldownPercent;

                // 绘制半透明灰色遮罩（从底部向上）
                overlay.graphics.fillStyle(0x000000, 0.6);
                overlay.graphics.fillRoundedRect(
                    overlay.x,
                    overlay.y + (overlay.height - fillHeight),
                    overlay.width,
                    fillHeight,
                    5
                );
            } else {
                // 隐藏冷却遮罩
                overlay.graphics.setVisible(false);
                overlay.graphics.clear();
            }
        }
    }

    /**
     * 存储戴夫数据用于UI更新
     */
    storeDaveData(daveData) {
        this.currentDaveData = daveData;

        // 如果在多人模式且戴夫被玩家控制，更新UI
        if (this.isMultiplayerMode && daveData.isPlayerControlled) {
            this.updateSeedPacketUI(daveData);
        }
    }

    /**
     * 选择植物
     */
    selectPlant(index) {
        console.log('selectPlant 被调用，index:', index);
        const packet = this.seedPackets[index];
        if (!packet) {
            console.log('无法选择植物：植物包不存在');
            return;
        }

        // 如果没有Dave数据，仍然允许选择（后端会验证）
        if (this.currentDaveData) {
            // 检查冷却
            const currentCooldown = this.currentDaveData[packet.cooldownKey] || 0;
            if (currentCooldown > 0) {
                console.log(`${packet.name} 正在冷却中 (${currentCooldown.toFixed(1)}s)`);
                return;
            }

            // 检查阳光是否足够
            const sunlight = this.currentDaveData.sunlight || 0;
            if (sunlight < packet.cost) {
                console.log(`阳光不足：需要 ${packet.cost}，当前 ${sunlight}`);
                return;
            }
        } else {
            console.log('警告：没有Dave数据，直接选择植物');
        }

        // 取消之前的选中
        this.seedPackets.forEach((p, i) => {
            if (p.selectBorder) {
                p.selectBorder.setVisible(i === index);
            }
        });

        // 设置选中
        this.selectedPlantIndex = index;
        console.log(`选中植物: ${packet.name} (花费: ${packet.cost})`);

        // 隐藏种植菜单，准备种植
        this.hideSeedPacketUI();
    }

    /**
     * 在指定位置种植选中的植物
     */
    plantSelectedPlant(worldX, worldY) {
        console.log('===== 开始种植流程 =====');
        console.log('selectedPlantIndex:', this.selectedPlantIndex);

        if (this.selectedPlantIndex < 0) {
            console.log('种植失败：没有选中植物');
            return false;
        }

        const packet = this.seedPackets[this.selectedPlantIndex];
        if (!packet) {
            console.log('种植失败：植物包不存在');
            return false;
        }

        console.log(`尝试种植 ${packet.name} 在位置 (${worldX.toFixed(1)}, ${worldY.toFixed(1)})`);

        // 发送种植命令到后端
        const plantCommands = [
            'DAVE_PLANT_PEA',       // 0: 豌豆射手
            'DAVE_PLANT_REPEATER',  // 1: 双发射手
            'DAVE_PLANT_CHERRY',    // 2: 樱桃炸弹
            'DAVE_PLANT_NUT'        // 3: 坚果墙
        ];

        const command = plantCommands[this.selectedPlantIndex];
        console.log('准备发送命令:', command);
        console.log('networkClient:', this.networkClient ? '存在' : '不存在');
        console.log('connected:', this.networkClient?.connected);

        if (this.networkClient && this.networkClient.connected) {
            this.networkClient.send(command, {});
            console.log(`✓ 已发送种植命令: ${command}`);
        } else {
            console.error('✗ 无法发送种植命令：网络未连接');
        }

        // 清除选中状态
        this.seedPackets.forEach(p => {
            if (p.selectBorder) {
                p.selectBorder.setVisible(false);
            }
        });
        this.selectedPlantIndex = -1;

        return true;
    }

    /**
     * 设置地图点击事件（用于种植）
     */
    setupPlantingClickHandler() {
        // 监听左侧屏幕的点击（戴夫视角）
        this.input.on('pointerdown', (pointer) => {
            // 只在多人模式且有选中植物时响应
            if (!this.isMultiplayerMode || this.selectedPlantIndex < 0) return;

            // 如果刚刚点击了种子包，跳过（防止选择后立即种植）
            if (this.justClickedSeedPacket) {
                console.log('跳过种植：刚刚点击了种子包');
                return;
            }

            // 如果种子包菜单可见，不处理种植（避免点击菜单时触发种植）
            if (this.seedPacketVisible) {
                console.log('跳过种植：种子包菜单可见');
                return;
            }

            // 检查是否点击在左半屏（戴夫区域）
            const screenWidth = this.cameras.main.width;
            if (this.splitScreenEnabled && pointer.x > screenWidth / 2) {
                // 点击在右半屏，忽略
                return;
            }

            // 转换为世界坐标
            let worldX, worldY;
            if (this.daveCamera) {
                const cam = this.daveCamera;
                worldX = pointer.x + cam.scrollX;
                worldY = pointer.y + cam.scrollY;
            } else {
                worldX = pointer.worldX;
                worldY = pointer.worldY;
            }

            // 尝试种植
            this.plantSelectedPlant(worldX, worldY);
        });
    }
}
