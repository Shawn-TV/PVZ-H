/**
 * Electron 预加载脚本
 * 在渲染进程中暴露安全的API
 */

const { contextBridge, ipcRenderer } = require('electron');

// 暴露安全的API到渲染进程
contextBridge.exposeInMainWorld('electronAPI', {
    // 发送游戏命令到后端
    sendCommand: (command) => {
        ipcRenderer.send('game-command', command);
    },

    // 监听游戏消息
    onGameMessage: (callback) => {
        ipcRenderer.on('game-message', (event, data) => {
            callback(data);
        });
    },

    // 移除游戏消息监听器
    removeGameMessageListener: () => {
        ipcRenderer.removeAllListeners('game-message');
    },

    // 请求启动游戏
    startGame: () => {
        ipcRenderer.send('start-game');
    },

    // 请求重启游戏
    restartGame: () => {
        ipcRenderer.send('restart-game');
    },

    // 检查是否在Electron环境中
    isElectron: true
});
