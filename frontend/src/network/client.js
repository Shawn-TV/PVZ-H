/**
 * @file client.js
 * @brief 网络客户端
 *
 * 负责：
 * - 建立WebSocket连接
 * - 发送玩家输入到服务器
 * - 接收游戏状态更新
 * - 缓存早期消息（在handler注册前收到的消息）
 */

export class NetworkClient {
    constructor(url) {
        this.url = url;
        this.ws = null;
        this.connected = false;
        this.messageHandlers = new Map();
        this.pendingMessages = []; // 缓存早期消息
    }

    connect() {
        return new Promise((resolve, reject) => {
            this.ws = new WebSocket(this.url);

            this.ws.onopen = () => {
                this.connected = true;
                resolve();
            };

            this.ws.onmessage = (event) => {
                this.handleMessage(event.data);
            };

            this.ws.onerror = (error) => {
                reject(error);
            };

            this.ws.onclose = () => {
                this.connected = false;
            };
        });
    }

    send(type, data) {
        if (!this.connected) return;

        const message = {
            type: type,
            data: data
        };

        this.ws.send(JSON.stringify(message));
    }

    handleMessage(data) {
        const message = JSON.parse(data);

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
        this.send('REQUEST_MAZE', {});
    }

    // 断开连接
    disconnect() {
        if (this.ws) {
            this.ws.close();
            this.ws = null;
            this.connected = false;
        }
    }
}
