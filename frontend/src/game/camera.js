/**
 * @file camera.js
 * @brief 摄像机系统
 *
 * 负责：
 * - 跟随玩家（僵尸）
 * - 限制视野范围（不能看到整个迷宫）
 * - 平滑移动
 */

export class Camera {
    constructor(viewWidth, viewHeight) {
        this.x = 0;
        this.y = 0;
        this.viewWidth = viewWidth;
        this.viewHeight = viewHeight;
        this.target = null;
        this.smoothFactor = 0.1; // 平滑跟随系数
    }

    follow(target) {
        // 设置跟随目标（玩家僵尸）
        this.target = target;
    }

    update() {
        if (!this.target) return;

        // TODO: 平滑跟随目标
        // 摄像机中心对准目标
        const targetX = this.target.x - this.viewWidth / 2;
        const targetY = this.target.y - this.viewHeight / 2;

        // 平滑插值
        this.x += (targetX - this.x) * this.smoothFactor;
        this.y += (targetY - this.y) * this.smoothFactor;

        // TODO: 限制摄像机边界（不超出迷宫范围）
    }

    worldToScreen(worldX, worldY) {
        // 世界坐标转屏幕坐标
        return {
            x: worldX - this.x,
            y: worldY - this.y
        };
    }

    screenToWorld(screenX, screenY) {
        // 屏幕坐标转世界坐标
        return {
            x: screenX + this.x,
            y: screenY + this.y
        };
    }
}
