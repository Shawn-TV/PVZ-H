/**
 * Electron 主进程
 * 负责创建窗口和管理游戏后端进程
 */

const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');
const { spawn } = require('child_process');

let mainWindow = null;
let gameProcess = null;
let stdoutBuffer = ''; // 缓冲区用于处理跨data事件的消息

// 判断是否为开发模式
const isDev = !app.isPackaged;

const fs = require('fs');

/**
 * 获取后端可执行文件路径
 */
function getBackendPath() {
    const isWindows = process.platform === 'win32';

    if (isDev) {
        // 开发模式：使用项目中的后端
        const basePath = path.join(__dirname, '../../backend/build');

        if (isWindows) {
            // Windows: 查找 Release/Debug 目录或根目录
            const possiblePaths = [
                path.join(basePath, 'Release', 'pvz_game.exe'),
                path.join(basePath, 'Debug', 'pvz_game.exe'),
                path.join(basePath, 'pvz_game.exe')
            ];
            for (const p of possiblePaths) {
                if (fs.existsSync(p)) return p;
            }
            return possiblePaths[0]; // 默认返回 Release 路径
        } else {
            return path.join(basePath, 'pvz_game');
        }
    } else {
        // 生产模式：使用打包的后端
        if (isWindows) {
            return path.join(process.resourcesPath, 'backend', 'pvz_game.exe');
        } else {
            return path.join(process.resourcesPath, 'backend', 'pvz_game');
        }
    }
}

/**
 * 启动游戏后端进程
 */
function startGameBackend() {
    if (gameProcess) {
        return; // 已经在运行
    }

    // 重置缓冲区
    stdoutBuffer = '';

    const backendPath = getBackendPath();

    gameProcess = spawn(backendPath, [], {
        stdio: ['pipe', 'pipe', 'pipe']
    });

    // 处理stdin错误
    gameProcess.stdin.on('error', () => {
        // 忽略EPIPE错误
    });

    // 发送模式选择（模式3：游戏主循环）
    gameProcess.stdin.write('3\n');

    // 处理后端输出（JSON消息）
    // 使用缓冲区确保完整行的解析（大消息可能被分割成多个data事件）
    gameProcess.stdout.on('data', (data) => {
        stdoutBuffer += data.toString();

        // 按换行符分割，最后一个元素可能是不完整的行
        const lines = stdoutBuffer.split('\n');

        // 保留最后一个不完整的行（如果有）
        stdoutBuffer = lines.pop() || '';

        // 处理所有完整的行
        lines.forEach(line => {
            const trimmed = line.trim();
            if (!trimmed) return;

            try {
                const json = JSON.parse(trimmed);
                // 转换消息类型以匹配前端期望的格式
                // 后端发送 ENTITIES，前端期望 ENTITIES_UPDATE
                if (json.type === 'ENTITIES') {
                    json.type = 'ENTITIES_UPDATE';
                }
                // 发送到渲染进程
                if (mainWindow && !mainWindow.isDestroyed()) {
                    mainWindow.webContents.send('game-message', json);
                }
            } catch {
                // 非JSON消息，忽略
            }
        });
    });

    gameProcess.stderr.on('data', () => {
        // 静默处理后端错误输出
    });

    gameProcess.on('close', () => {
        gameProcess = null;
    });

    gameProcess.on('error', () => {
        // 静默处理启动错误
    });
}

/**
 * 发送命令到游戏后端
 */
function sendToBackend(command) {
    if (gameProcess && gameProcess.stdin) {
        gameProcess.stdin.write(command);
    }
}

/**
 * 创建主窗口
 */
function createWindow() {
    mainWindow = new BrowserWindow({
        width: 1280,
        height: 720,
        minWidth: 800,
        minHeight: 600,
        title: 'PVZ Maze Edition',
        webPreferences: {
            nodeIntegration: false,
            contextIsolation: true,
            preload: path.join(__dirname, 'preload.cjs')
        },
        // 窗口样式
        backgroundColor: '#1a1a2e',
        show: false  // 先隐藏，加载完成后显示
    });

    // 加载页面
    if (isDev) {
        // 开发模式：连接到Vite开发服务器
        mainWindow.loadURL('http://localhost:5173');
        // 打开开发者工具
        mainWindow.webContents.openDevTools();
    } else {
        // 生产模式：加载打包后的文件
        mainWindow.loadFile(path.join(__dirname, '../dist/index.html'));
    }

    // 页面加载完成后显示窗口
    mainWindow.once('ready-to-show', () => {
        mainWindow.show();
        // 启动游戏后端
        startGameBackend();
    });

    // 窗口关闭时清理
    mainWindow.on('closed', () => {
        mainWindow = null;
        // 关闭后端进程
        if (gameProcess) {
            gameProcess.kill();
            gameProcess = null;
        }
    });
}

// 应用准备好后创建窗口
app.whenReady().then(() => {
    createWindow();

    // macOS: 点击dock图标时重新创建窗口
    app.on('activate', () => {
        if (BrowserWindow.getAllWindows().length === 0) {
            createWindow();
        }
    });
});

// 所有窗口关闭时退出应用（Windows & Linux）
app.on('window-all-closed', () => {
    if (process.platform !== 'darwin') {
        app.quit();
    }
});

// 应用退出前清理
app.on('before-quit', () => {
    if (gameProcess) {
        gameProcess.kill();
        gameProcess = null;
    }
});

// IPC: 接收渲染进程发来的游戏命令
ipcMain.on('game-command', (event, command) => {
    sendToBackend(command);
});

// IPC: 请求启动新游戏
ipcMain.on('start-game', () => {
    if (!gameProcess) {
        startGameBackend();
    }
});

// IPC: 请求重启游戏
ipcMain.on('restart-game', () => {
    if (gameProcess) {
        gameProcess.kill();
    }
    setTimeout(() => {
        startGameBackend();
    }, 500);
});
