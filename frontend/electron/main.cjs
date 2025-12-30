/**
 * Electron 主进程
 * 负责创建窗口和管理游戏后端进程
 */

const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');
const { spawn } = require('child_process');

let mainWindow = null;
let gameProcess = null;

// 判断是否为开发模式
const isDev = !app.isPackaged;

/**
 * 获取后端可执行文件路径
 */
function getBackendPath() {
    if (isDev) {
        // 开发模式：使用项目中的后端
        return path.join(__dirname, '../../backend/build/pvz_game');
    } else {
        // 生产模式：使用打包的后端
        return path.join(process.resourcesPath, 'backend/pvz_game');
    }
}

/**
 * 启动游戏后端进程
 */
function startGameBackend() {
    const backendPath = getBackendPath();

    gameProcess = spawn(backendPath, [], {
        stdio: ['pipe', 'pipe', 'pipe']
    });

    // 处理后端输出（JSON消息）
    gameProcess.stdout.on('data', (data) => {
        const messages = data.toString().split('\n').filter(line => line.trim());
        messages.forEach(message => {
            try {
                const json = JSON.parse(message);
                // 发送到渲染进程
                if (mainWindow && !mainWindow.isDestroyed()) {
                    mainWindow.webContents.send('game-message', json);
                }
            } catch (e) {
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
