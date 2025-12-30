/**
 * @file bridge.js
 * @brief WebSocket桥接服务器
 *
 * 功能：
 * - 启动C++游戏进程
 * - 从stdin读取游戏状态JSON
 * - 通过WebSocket广播给前端
 * - 接收前端输入并转发给C++进程
 */

const WebSocket = require('ws');
const { spawn } = require('child_process');
const path = require('path');
const fs = require('fs');

const WS_PORT = 8080;

// 根据操作系统和编译配置查找游戏可执行文件
function findGameExecutable() {
    const isWindows = process.platform === 'win32';
    const basePath = path.join(__dirname, '../backend/build');

    // Windows上的可能路径（Visual Studio编译）
    const windowsPaths = [
        path.join(basePath, 'Release', 'pvz_game.exe'),
        path.join(basePath, 'Debug', 'pvz_game.exe'),
        path.join(basePath, 'pvz_game.exe')
    ];

    // Linux/Mac上的路径
    const unixPath = path.join(basePath, 'pvz_game');

    if (isWindows) {
        // 在Windows上依次查找
        for (const exePath of windowsPaths) {
            if (fs.existsSync(exePath)) {
                return exePath;
            }
        }
        // 都没找到，返回默认的Release路径（让错误信息更清楚）
        return windowsPaths[0];
    } else {
        return unixPath;
    }
}

const GAME_EXECUTABLE = findGameExecutable();

class GameBridge {
    constructor() {
        this.wss = new WebSocket.Server({ port: WS_PORT });
        this.clients = new Set();
        this.gameProcess = null;
        this.gameState = null;
        this.mazeData = null;
        this.entities = [];
        this.gameStarted = false;

        this.setupWebSocketServer();
    }

    setupWebSocketServer() {
        this.wss.on('connection', (ws) => {
            this.clients.add(ws);

            // 发送初始迷宫数据
            if (this.mazeData) {
                ws.send(JSON.stringify({
                    type: 'MAZE_INIT',
                    data: this.mazeData
                }));
            }

            // 发送当前游戏状态
            if (this.gameState) {
                ws.send(JSON.stringify({
                    type: 'GAME_STATE',
                    data: this.gameState
                }));
            }

            // 接收客户端输入
            ws.on('message', (message) => {
                try {
                    const msg = JSON.parse(message);
                    this.handleClientInput(msg);
                } catch {
                    // 静默处理解析错误
                }
            });

            ws.on('close', () => {
                this.clients.delete(ws);
            });

            ws.on('error', () => {
                this.clients.delete(ws);
            });
        });
    }

    startGameProcess(isMultiplayer = false) {
        // 使用模式3（仅游戏主循环）
        this.gameProcess = spawn(GAME_EXECUTABLE, [], {
            stdio: ['pipe', 'pipe', 'pipe']
        });

        // 处理stdin错误，防止EPIPE错误崩溃
        this.gameProcess.stdin.on('error', () => {
            // 忽略EPIPE错误，进程已经终止
        });

        // 向游戏进程发送模式选择（模式3）
        this.safeWrite('3\n');

        // 如果是多人模式，发送 ENABLE_DAVE_PLAYER 命令
        if (isMultiplayer) {
            setTimeout(() => {
                this.safeWrite('m\n');
            }, 50);
        }

        let buffer = '';

        this.gameProcess.stdout.on('data', (data) => {
            const output = data.toString();
            buffer += output;

            // 尝试解析JSON消息（每行一个JSON对象）
            const lines = buffer.split('\n');
            buffer = lines.pop(); // 保留不完整的行

            for (const line of lines) {
                if (line.trim().startsWith('{')) {
                    try {
                        const json = JSON.parse(line);
                        this.handleGameOutput(json);
                    } catch {
                        // 不是有效JSON，忽略
                    }
                }
            }
        });

        this.gameProcess.stderr.on('data', () => {
            // 静默处理后端错误输出
        });

        this.gameProcess.on('close', (code) => {
            // 重置游戏状态，允许重新开始
            this.gameProcess = null;
            this.gameStarted = false;
            this.mazeData = null;
            this.gameState = null;
            this.entities = [];
            // 通知所有客户端游戏结束
            this.broadcast({
                type: 'GAME_END',
                data: { exitCode: code }
            });
        });
    }

    handleGameOutput(json) {
        // 根据消息类型处理
        if (json.type === 'MAZE_INIT') {
            this.mazeData = json.data;
            this.broadcast({
                type: 'MAZE_INIT',
                data: this.mazeData
            });
        } else if (json.type === 'GAME_STATE') {
            this.gameState = json.data;
            this.broadcast({
                type: 'GAME_STATE',
                data: this.gameState
            });
        } else if (json.type === 'ENTITIES') {
            this.entities = json.data;
            this.broadcast({
                type: 'ENTITIES_UPDATE',
                data: this.entities
            });
        } else if (json.type === 'GAME_OVER') {
            this.broadcast({
                type: 'GAME_OVER',
                data: json.data
            });
        }
    }

    /**
     * 安全地向游戏进程写入数据，处理EPIPE等错误
     */
    safeWrite(data) {
        if (this.gameProcess && this.gameProcess.stdin && !this.gameProcess.stdin.destroyed) {
            try {
                this.gameProcess.stdin.write(data);
            } catch {
                // 静默处理写入错误
            }
        }
    }

    handleClientInput(msg) {
        // 处理START_GAME消息
        if (msg.type === 'START_GAME') {
            if (!this.gameStarted) {
                this.gameStarted = true;
                this.startGameProcess();
            }
            return;
        }

        // 处理RESTART_GAME消息 - 重新开始一局新游戏
        if (msg.type === 'RESTART_GAME') {
            const isMultiplayer = msg.data && msg.data.multiplayer === true;
            if (this.gameProcess) {
                // 杀死现有游戏进程
                this.gameProcess.kill('SIGKILL');
                this.gameProcess = null;
            }
            // 重置状态
            this.gameStarted = false;
            this.mazeData = null;
            this.gameState = null;
            this.entities = [];
            // 启动新游戏
            this.gameStarted = true;
            this.startGameProcess(isMultiplayer);
            return;
        }

        // 处理END_GAME消息 - 结束当前游戏（返回主菜单时发送）
        if (msg.type === 'END_GAME') {
            if (this.gameProcess) {
                this.gameProcess.kill('SIGKILL');
                this.gameProcess = null;
            }
            // 重置状态
            this.gameStarted = false;
            this.mazeData = null;
            this.gameState = null;
            this.entities = [];
            return;
        }

        if (!this.gameProcess) return;

        // 将输入转发给游戏进程
        // 根据消息类型发送对应的命令
        switch (msg.type) {
            // ==================== 僵尸控制 ====================
            case 'MOVE_UP':
                this.safeWrite('w\n');
                break;
            case 'MOVE_DOWN':
                this.safeWrite('s\n');
                break;
            case 'MOVE_LEFT':
                this.safeWrite('a\n');
                break;
            case 'MOVE_RIGHT':
                this.safeWrite('d\n');
                break;
            case 'STOP_MOVE':
                this.safeWrite('x\n');
                break;
            case 'ATTACK':
                this.safeWrite(' \n');
                break;
            case 'POLE_VAULT':
                this.safeWrite('c\n');
                break;

            // ==================== 戴夫控制（多人模式） ====================
            case 'DAVE_MOVE_UP':
                this.safeWrite('i\n');
                break;
            case 'DAVE_MOVE_DOWN':
                this.safeWrite('k\n');
                break;
            case 'DAVE_MOVE_LEFT':
                this.safeWrite('j\n');
                break;
            case 'DAVE_MOVE_RIGHT':
                this.safeWrite('l\n');
                break;
            case 'DAVE_STOP_MOVE':
                this.safeWrite('o\n');
                break;
            case 'DAVE_PLANT_MENU':
                // Q键不再直接发送，改为由前端处理植物菜单
                break;
            case 'DAVE_PLANT_PEA':
            case 'DAVE_PLANT_REPEATER':
            case 'DAVE_PLANT_CHERRY':
            case 'DAVE_PLANT_NUT': {
                // 获取植物类型索引
                const plantTypeMap = {
                    'DAVE_PLANT_PEA': 0,
                    'DAVE_PLANT_REPEATER': 1,
                    'DAVE_PLANT_CHERRY': 2,
                    'DAVE_PLANT_NUT': 3
                };
                const plantType = plantTypeMap[msg.type];

                // 检查是否有位置数据
                if (msg.data && typeof msg.data.x === 'number' && typeof msg.data.y === 'number') {
                    // 发送带位置的种植命令: P<type>,<x>,<y>
                    const cmd = `P${plantType},${msg.data.x},${msg.data.y}\n`;
                    this.safeWrite(cmd);
                } else {
                    // 兼容旧格式：在Dave当前位置种植
                    this.safeWrite(`${plantType + 1}\n`);
                }
                break;
            }
            case 'ENABLE_DAVE_PLAYER':
                this.safeWrite('m\n');
                break;
            case 'DISABLE_DAVE_PLAYER':
                this.safeWrite('n\n');
                break;

            // ==================== 游戏控制 ====================
            case 'PAUSE':
                this.safeWrite('p\n');
                break;
            case 'RESUME':
                this.safeWrite('p\n');
                break;
        }
    }

    broadcast(message) {
        const data = JSON.stringify(message);
        this.clients.forEach(client => {
            if (client.readyState === WebSocket.OPEN) {
                client.send(data);
            }
        });
    }

    shutdown() {
        if (this.gameProcess) {
            this.gameProcess.kill();
        }
        this.wss.close();
    }
}

// 启动服务器
const bridge = new GameBridge();

// 优雅关闭
process.on('SIGINT', () => {
    bridge.shutdown();
    process.exit(0);
});

process.on('SIGTERM', () => {
    bridge.shutdown();
    process.exit(0);
});
