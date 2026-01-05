/**
 * @file electronClient.js
 * @brief Electron IPC 客户端
 */

export class ElectronClient {
    constructor() {
        this.connected = false;
        this.messageHandlers = new Map();
        this.pendingMessages = [];
    }

    connect() {
        return new Promise((resolve, reject) => {
            if (!window.electronAPI) {
                reject(new Error('不在Electron环境中'));
                return;
            }

            // 设置消息监听
            window.electronAPI.onGameMessage((data) => {
                this.handleMessage(data);
            });

            // 启动游戏后端
            window.electronAPI.startGame();

            this.connected = true;
            resolve();
        });
    }

    send(type, data = {}) {
        if (!this.connected || !window.electronAPI) return;

        // 将消息类型转换为后端命令
        let command = null;

        switch (type) {
            // 僵尸控制
            case 'MOVE_UP':
                command = 'w\n';
                break;
            case 'MOVE_DOWN':
                command = 's\n';
                break;
            case 'MOVE_LEFT':
                command = 'a\n';
                break;
            case 'MOVE_RIGHT':
                command = 'd\n';
                break;
            case 'STOP_MOVE':
                command = 'x\n';
                break;
            case 'ATTACK':
                command = ' \n';
                break;
            case 'POLE_VAULT':
                command = 'c\n';
                break;

            // 戴夫控制（多人模式）
            case 'DAVE_MOVE_UP':
                command = 'i\n';
                break;
            case 'DAVE_MOVE_DOWN':
                command = 'k\n';
                break;
            case 'DAVE_MOVE_LEFT':
                command = 'j\n';
                break;
            case 'DAVE_MOVE_RIGHT':
                command = 'l\n';
                break;
            case 'DAVE_STOP_MOVE':
                command = 'o\n';
                break;

            // 种植命令
            case 'DAVE_PLANT_PEA':
            case 'DAVE_PLANT_REPEATER':
            case 'DAVE_PLANT_CHERRY':
            case 'DAVE_PLANT_NUT': {
                const plantTypeMap = {
                    'DAVE_PLANT_PEA': 0,
                    'DAVE_PLANT_REPEATER': 1,
                    'DAVE_PLANT_CHERRY': 2,
                    'DAVE_PLANT_NUT': 3
                };
                const plantType = plantTypeMap[type];
                if (data && typeof data.x === 'number' && typeof data.y === 'number') {
                    command = `P${plantType},${data.x},${data.y}\n`;
                } else {
                    command = `${plantType + 1}\n`;
                }
                break;
            }

            case 'ENABLE_DAVE_PLAYER':
                command = 'm\n';
                break;
            case 'DISABLE_DAVE_PLAYER':
                command = 'n\n';
                break;

            // 游戏控制
            case 'PAUSE':
            case 'RESUME':
                command = 'p\n';
                break;

            case 'RESTART_GAME':
                // 重启游戏后端
                // 多人模式命令由GameScene在收到MAZE_INIT后发送
                window.electronAPI.restartGame();
                return;

            case 'START_GAME':
                window.electronAPI.startGame();
                return;

            case 'END_GAME':
                // Electron模式下不需要特殊处理
                return;
        }

        if (command) {
            window.electronAPI.sendCommand(command);
        }
    }

    handleMessage(data) {
        const message = data;
        const handler = this.messageHandlers.get(message.type);
        if (handler) {
            handler(message.data);
        } else {
            this.pendingMessages.push(message);
        }
    }

    on(messageType, handler) {
        this.messageHandlers.set(messageType, handler);

        const pending = this.pendingMessages.filter(msg => msg.type === messageType);
        pending.forEach(msg => {
            handler(msg.data);
        });
        this.pendingMessages = this.pendingMessages.filter(msg => msg.type !== messageType);
    }

    disconnect() {
        if (window.electronAPI) {
            window.electronAPI.removeGameMessageListener();
        }
        this.connected = false;
    }
}
