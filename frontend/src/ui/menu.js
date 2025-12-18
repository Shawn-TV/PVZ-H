/**
 * @file menu.js
 * @brief 菜单界面
 *
 * 负责：
 * - 主菜单
 * - 暂停菜单
 * - 游戏结束界面
 */

export class Menu {
    constructor() {
        this.visible = false;
    }

    showMainMenu() {
        // TODO: 显示主菜单
        // 开始游戏、设置、退出
    }

    showPauseMenu() {
        // TODO: 显示暂停菜单
        // 继续、重新开始、返回主菜单
    }

    showGameOver(won) {
        // TODO: 显示游戏结束界面
        // 显示胜利或失败信息
        // 重新开始、返回主菜单
    }

    hide() {
        this.visible = false;
    }
}
