/**
 * @file hud.js
 * @brief HUD界面管理
 *
 * 负责：
 * - 显示玩家生命值
 * - 显示道具栏
 * - 显示游戏信息（时间、分数等）
 */

export class HUD {
    constructor() {
        this.healthBar = document.getElementById('health-fill');
        this.inventory = document.getElementById('inventory');
    }

    updateHealth(current, max) {
        const percentage = (current / max) * 100;
        this.healthBar.style.width = `${percentage}%`;

        // 根据生命值改变颜色
        if (percentage > 60) {
            this.healthBar.style.backgroundColor = '#00ff00';
        } else if (percentage > 30) {
            this.healthBar.style.backgroundColor = '#ffff00';
        } else {
            this.healthBar.style.backgroundColor = '#ff0000';
        }
    }

    updateInventory(items) {
        // TODO: 更新道具栏显示
        this.inventory.innerHTML = '';
        items.forEach((item, index) => {
            const itemDiv = document.createElement('div');
            itemDiv.className = 'inventory-item';
            itemDiv.textContent = `${index + 1}`;
            // TODO: 添加道具图标
            this.inventory.appendChild(itemDiv);
        });
    }

    showMessage(text, duration = 3000) {
        // TODO: 显示临时消息（如"拾取道具"、"游戏胜利"等）
    }
}
