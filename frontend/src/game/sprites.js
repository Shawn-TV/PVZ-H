/**
 * @file sprites.js
 * @brief 精灵图管理
 *
 * 负责：
 * - 加载和缓存精灵图
 * - 提供精灵访问接口
 * - 动画帧管理
 */

export class SpriteManager {
    constructor() {
        this.sprites = new Map();
        this.loaded = false;
    }

    async loadAll() {
        // TODO: 加载所有精灵图
        // 僵尸、植物、戴夫、道具、迷宫墙壁等

        const spriteList = [
            { name: 'zombie', path: 'assets/images/zombies/zombie.png' },
            { name: 'peashooter', path: 'assets/images/plants/peashooter.png' },
            { name: 'cherrybomb', path: 'assets/images/plants/cherrybomb.png' },
            { name: 'wallnut', path: 'assets/images/plants/wallnut.png' },
            { name: 'dave', path: 'assets/images/dave.png' },
            { name: 'bucket', path: 'assets/images/items/bucket.png' },
            { name: 'potion', path: 'assets/images/items/potion.png' },
        ];

        // TODO: 异步加载所有图片
        this.loaded = true;
    }

    getSprite(name) {
        return this.sprites.get(name);
    }
}
