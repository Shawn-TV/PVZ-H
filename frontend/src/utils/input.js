/**
 * @file input.js
 * @brief 输入处理
 *
 * 负责：
 * - 监听键盘输入（WASD/方向键）
 * - 监听鼠标输入
 * - 将输入转换为游戏命令发送到服务器
 */

export class InputHandler {
    constructor() {
        this.keys = new Map();
        this.mousePos = { x: 0, y: 0 };
        this.callbacks = new Map();

        this.setupListeners();
    }

    setupListeners() {
        // 键盘事件
        window.addEventListener('keydown', (e) => {
            this.keys.set(e.key, true);
            this.handleKeyPress(e.key);
        });

        window.addEventListener('keyup', (e) => {
            this.keys.set(e.key, false);
        });

        // 鼠标事件
        window.addEventListener('mousemove', (e) => {
            this.mousePos.x = e.clientX;
            this.mousePos.y = e.clientY;
        });

        window.addEventListener('click', (e) => {
            this.handleClick(e.clientX, e.clientY);
        });
    }

    isKeyPressed(key) {
        return this.keys.get(key) || false;
    }

    handleKeyPress(key) {
        // 发送移动命令到服务器
        const moveKeys = {
            'w': 'up',
            'ArrowUp': 'up',
            's': 'down',
            'ArrowDown': 'down',
            'a': 'left',
            'ArrowLeft': 'left',
            'd': 'right',
            'ArrowRight': 'right'
        };

        if (moveKeys[key]) {
            const callback = this.callbacks.get('move');
            if (callback) {
                callback(moveKeys[key]);
            }
        }

        // 数字键使用道具
        if (key >= '1' && key <= '9') {
            const slot = parseInt(key) - 1;
            const callback = this.callbacks.get('useItem');
            if (callback) {
                callback(slot);
            }
        }
    }

    handleClick(x, y) {
        // TODO: 处理点击事件
    }

    on(event, callback) {
        this.callbacks.set(event, callback);
    }
}
