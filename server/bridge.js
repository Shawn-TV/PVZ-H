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
                console.log(`找到游戏可执行文件: ${exePath}`);
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
        // 不再自动启动游戏进程，等待客户端发送START_GAME消息
        console.log('等待客户端发送START_GAME消息以启动游戏...');
    }

    setupWebSocketServer() {
        console.log(`WebSocket服务器启动在端口 ${WS_PORT}`);

        this.wss.on('connection', (ws) => {
            console.log('客户端已连接');
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
                } catch (e) {
                    console.error('解析客户端消息失败:', e);
                }
            });

            ws.on('close', () => {
                console.log('客户端已断开');
                this.clients.delete(ws);
            });

            ws.on('error', (error) => {
                console.error('WebSocket错误:', error);
                this.clients.delete(ws);
            });
        });
    }

    startGameProcess(isMultiplayer = false) {
        console.log('启动游戏进程:', GAME_EXECUTABLE);

        // 使用模式3（仅游戏主循环）
        this.gameProcess = spawn(GAME_EXECUTABLE, [], {
            stdio: ['pipe', 'pipe', 'pipe']
        });

        // 向游戏进程发送模式选择（模式3）
        this.gameProcess.stdin.write('3\n');

        // 如果是多人模式，多次发送 ENABLE_DAVE_PLAYER 命令确保被接收
        if (isMultiplayer) {
            console.log('多人模式：准备发送 ENABLE_DAVE_PLAYER 命令');
            // 多次发送，确保命令被接收
            const sendEnableCommand = () => {
                if (this.gameProcess && this.gameProcess.stdin) {
                    this.gameProcess.stdin.write('m\n');
                    console.log('多人模式：发送 m 命令');
                }
            };
            // 在不同时间点发送，确保至少有一次成功
            setTimeout(sendEnableCommand, 300);
            setTimeout(sendEnableCommand, 600);
            setTimeout(sendEnableCommand, 1000);
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
                    } catch (e) {
                        // 不是有效JSON，忽略
                    }
                }
            }
        });

        this.gameProcess.stderr.on('data', (data) => {
            console.error('游戏错误:', data.toString());
        });

        this.gameProcess.on('close', (code) => {
            console.log(`游戏进程退出，代码: ${code}`);
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
            console.log('游戏状态已重置，等待新的START_GAME消息');
        });
    }

    handleGameOutput(json) {
        // 根据消息类型处理
        if (json.type === 'MAZE_DATA') {
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

    handleClientInput(msg) {
        // 处理START_GAME消息
        if (msg.type === 'START_GAME') {
            if (!this.gameStarted) {
                console.log('收到START_GAME消息，启动游戏进程');
                this.gameStarted = true;
                this.startGameProcess();
            }
            return;
        }

        // 处理RESTART_GAME消息 - 重新开始一局新游戏
        if (msg.type === 'RESTART_GAME') {
            const isMultiplayer = msg.data && msg.data.multiplayer === true;
            console.log('收到RESTART_GAME消息，重启游戏进程，多人模式:', isMultiplayer);
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
            console.log('收到END_GAME消息，终止游戏进程');
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
                this.gameProcess.stdin.write('w\n');
                break;
            case 'MOVE_DOWN':
                this.gameProcess.stdin.write('s\n');
                break;
            case 'MOVE_LEFT':
                this.gameProcess.stdin.write('a\n');
                break;
            case 'MOVE_RIGHT':
                this.gameProcess.stdin.write('d\n');
                break;
            case 'STOP_MOVE':
                this.gameProcess.stdin.write('x\n');
                break;
            case 'ATTACK':
                this.gameProcess.stdin.write(' \n');
                break;
            case 'POLE_VAULT':
                this.gameProcess.stdin.write('c\n');
                break;

            // ==================== 戴夫控制（多人模式） ====================
            case 'DAVE_MOVE_UP':
                this.gameProcess.stdin.write('i\n');
                break;
            case 'DAVE_MOVE_DOWN':
                this.gameProcess.stdin.write('k\n');
                break;
            case 'DAVE_MOVE_LEFT':
                this.gameProcess.stdin.write('j\n');
                break;
            case 'DAVE_MOVE_RIGHT':
                this.gameProcess.stdin.write('l\n');
                break;
            case 'DAVE_STOP_MOVE':
                this.gameProcess.stdin.write('o\n');
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
                    this.gameProcess.stdin.write(cmd);
                    console.log('发送种植命令:', cmd.trim());
                } else {
                    // 兼容旧格式：在Dave当前位置种植
                    this.gameProcess.stdin.write(`${plantType + 1}\n`);
                }
                break;
            }
            case 'ENABLE_DAVE_PLAYER':
                this.gameProcess.stdin.write('m\n');
                break;
            case 'DISABLE_DAVE_PLAYER':
                this.gameProcess.stdin.write('n\n');
                break;

            // ==================== 游戏控制 ====================
            case 'PAUSE':
                this.gameProcess.stdin.write('p\n');
                break;
            case 'RESUME':
                this.gameProcess.stdin.write('p\n');
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
        console.log('关闭桥接服务器...');
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
    console.log('\n收到SIGINT信号，正在关闭...');
    bridge.shutdown();
    process.exit(0);
});

process.on('SIGTERM', () => {
    console.log('\n收到SIGTERM信号，正在关闭...');
    bridge.shutdown();
    process.exit(0);
});
