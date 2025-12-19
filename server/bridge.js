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

        this.setupWebSocketServer();
        this.startGameProcess();
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

    startGameProcess() {
        console.log('启动游戏进程:', GAME_EXECUTABLE);

        // 使用模式3（仅游戏主循环）
        this.gameProcess = spawn(GAME_EXECUTABLE, [], {
            stdio: ['pipe', 'pipe', 'pipe']
        });

        // 向游戏进程发送模式选择（模式3）
        this.gameProcess.stdin.write('3\n');

        let buffer = '';

        this.gameProcess.stdout.on('data', (data) => {
            buffer += data.toString();

            // 尝试解析JSON消息（每行一个JSON对象）
            const lines = buffer.split('\n');
            buffer = lines.pop(); // 保留不完整的行

            for (const line of lines) {
                if (line.trim().startsWith('{')) {
                    try {
                        const json = JSON.parse(line);
                        this.handleGameOutput(json);
                    } catch (e) {
                        // 不是JSON，可能是调试输出，忽略
                    }
                }
            }
        });

        this.gameProcess.stderr.on('data', (data) => {
            console.error('游戏错误:', data.toString());
        });

        this.gameProcess.on('close', (code) => {
            console.log(`游戏进程退出，代码: ${code}`);
            // 通知所有客户端游戏结束
            this.broadcast({
                type: 'GAME_END',
                data: { exitCode: code }
            });
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
        if (!this.gameProcess) return;

        // 将输入转发给游戏进程
        // 根据消息类型发送对应的命令
        switch (msg.type) {
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
