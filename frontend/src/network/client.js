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
                console.log('已连接到服务器');
                this.connected = true;
                resolve();
            };

            this.ws.onmessage = (event) => {
                this.handleMessage(event.data);
            };

            this.ws.onerror = (error) => {
                console.error('WebSocket错误:', error);
                reject(error);
            };

            this.ws.onclose = () => {
                console.log('连接已断开');
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
        // 减少控制台输出 - 只记录非常规消息
        // console.log('收到消息:', message.type);

        const handler = this.messageHandlers.get(message.type);
        if (handler) {
            handler(message.data);
        } else {
            // 没有handler，缓存消息
            // console.log('缓存消息（handler未注册）:', message.type);
            this.pendingMessages.push(message);
        }
    }

    on(messageType, handler) {
        this.messageHandlers.set(messageType, handler);

        // 处理之前缓存的该类型消息
        const pending = this.pendingMessages.filter(msg => msg.type === messageType);
        pending.forEach(msg => {
            console.log('处理缓存的消息:', msg.type);
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
