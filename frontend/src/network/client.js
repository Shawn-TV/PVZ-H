/**
 * @file client.js
 * @brief 网络客户端
 *
 * 负责：
 * - 建立WebSocket连接
 * - 发送玩家输入到服务器
 * - 接收游戏状态更新
 * - 处理断线重连
 */

export class NetworkClient {
    constructor(url) {
        this.url = url;
        this.ws = null;
        this.connected = false;
        this.messageHandlers = new Map();
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
                // TODO: 重连逻辑
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
        // TODO: 解析消息并分发到对应处理器
        const message = JSON.parse(data);
        const handler = this.messageHandlers.get(message.type);
        if (handler) {
            handler(message.data);
        }
    }

    on(messageType, handler) {
        this.messageHandlers.set(messageType, handler);
    }
}
