/**
 * @file electronClient.js
 * @brief Electron IPC 客户端
 *
 * 负责：
 * - 通过Electron IPC与后端通信
 * - 与NetworkClient接口兼容
 */

export class ElectronClient {
    constructor() {
        this.connected = false;
        this.messageHandlers = new Map();
        this.pendingMessages = []; // 缓存早期消息
    }

    connect() {
        return new Promise((resolve) => {
            // 检查是否在Electron环境中
            if (!window.electronAPI) {
                throw new Error('不在Electron环境中');
            }

            // 设置消息监听
            window.electronAPI.onGameMessage((data) => {
                this.handleMessage(data);
            });

            this.connected = true;
            resolve();
        });
    }

    send(type, data) {
        if (!this.connected || !window.electronAPI) return;

        // Electron模式下，直接发送单字符命令到后端
        // 后端通过stdin接收字符命令
        if (type === 'INPUT') {
            window.electronAPI.sendCommand(data.key);
        } else if (type === 'PLANT') {
            // 种植命令格式: P植物类型,格子X,格子Y
            const { plantType, gridX, gridY } = data;
            window.electronAPI.sendCommand(`P${plantType},${gridX},${gridY}\n`);
        }
    }

    handleMessage(data) {
        // 后端发送的JSON消息已被解析
        const message = data;

        const handler = this.messageHandlers.get(message.type);
        if (handler) {
            handler(message.data);
        } else {
            // 没有handler，缓存消息
            this.pendingMessages.push(message);
        }
    }

    on(messageType, handler) {
        this.messageHandlers.set(messageType, handler);

        // 处理之前缓存的该类型消息
        const pending = this.pendingMessages.filter(msg => msg.type === messageType);
        pending.forEach(msg => {
            handler(msg.data);
        });
        // 移除已处理的消息
        this.pendingMessages = this.pendingMessages.filter(msg => msg.type !== messageType);
    }

    // 请求重新发送迷宫数据
    requestMazeData() {
        // Electron模式下不需要这个功能，后端启动时自动发送
    }

    // 断开连接
    disconnect() {
        if (window.electronAPI) {
            window.electronAPI.removeGameMessageListener();
        }
        this.connected = false;
    }
}
