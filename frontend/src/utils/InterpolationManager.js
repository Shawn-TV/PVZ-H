/**
 * @file InterpolationManager.js
 * @brief 帧插值管理器 - 实现60FPS平滑动画
 *
 * 功能：
 * - 预测性位置插值（基于速度）
 * - 三次Hermite插值算法
 * - 自适应延迟补偿
 * - 减少网络抖动
 */

export class InterpolationManager {
    constructor() {
        // 存储每个实体的插值状态
        this.entityStates = new Map(); // entityId -> { current, previous, target, timestamp }

        // 插值设置
        this.interpolationDelay = 100; // 100ms延迟缓冲（约6帧）
        this.maxExtrapolation = 200;   // 最大预测时间（ms）
        this.smoothingFactor = 0.2;    // 平滑系数（0-1，越小越平滑）
    }

    /**
     * 更新实体状态（从服务器接收新数据时调用）
     * @param {number} entityId - 实体ID
     * @param {object} newData - 新的实体数据 {x, y, vx, vy, ...}
     */
    updateEntity(entityId, newData) {
        const now = performance.now();

        if (!this.entityStates.has(entityId)) {
            // 新实体：初始化状态
            this.entityStates.set(entityId, {
                current: { ...newData },
                previous: { ...newData },
                target: { ...newData },
                timestamp: now,
                lastUpdateTime: now
            });
        } else {
            // 已存在实体：更新目标状态
            const state = this.entityStates.get(entityId);

            // 保存上一个目标为previous
            state.previous = { ...state.target };

            // 设置新目标
            state.target = { ...newData };
            state.timestamp = now;
            state.lastUpdateTime = now;
        }
    }

    /**
     * 获取插值后的实体位置（每帧调用）
     * @param {number} entityId - 实体ID
     * @returns {object|null} 插值后的位置 {x, y} 或 null
     */
    getInterpolatedPosition(entityId) {
        if (!this.entityStates.has(entityId)) {
            return null;
        }

        const state = this.entityStates.get(entityId);
        const now = performance.now();

        // 计算插值时间
        const timeSinceUpdate = now - state.timestamp;
        const renderTime = now - this.interpolationDelay;

        // 如果数据太旧，使用预测性外推
        if (timeSinceUpdate > this.interpolationDelay + 50) {
            return this.extrapolatePosition(state, timeSinceUpdate);
        }

        // 使用Hermite插值算法获得平滑位置
        return this.hermiteInterpolation(state, renderTime);
    }

    /**
     * Hermite插值算法（三次样条插值）
     * 提供比线性插值更平滑的运动
     */
    hermiteInterpolation(state, renderTime) {
        const { previous, target, timestamp } = state;

        // 计算插值因子 t (0-1)
        const updateInterval = 16.67; // 60fps = 16.67ms
        const t = Math.min(1.0, (renderTime - (timestamp - updateInterval)) / updateInterval);

        // 三次Hermite插值公式
        const t2 = t * t;
        const t3 = t2 * t;

        // Hermite基函数
        const h00 = 2*t3 - 3*t2 + 1;   // 起点位置权重
        const h10 = t3 - 2*t2 + t;     // 起点切线权重
        const h01 = -2*t3 + 3*t2;      // 终点位置权重
        const h11 = t3 - t2;           // 终点切线权重

        // 计算切线（速度）
        const m0x = target.vx || 0;
        const m0y = target.vy || 0;
        const m1x = target.vx || 0;
        const m1y = target.vy || 0;

        // 应用Hermite插值
        const x = h00 * previous.x + h10 * m0x + h01 * target.x + h11 * m1x;
        const y = h00 * previous.y + h10 * m0y + h01 * target.y + h11 * m1y;

        return { x, y };
    }

    /**
     * 预测性外推（当数据延迟时使用）
     * 基于速度向量预测未来位置
     */
    extrapolatePosition(state, timeSinceUpdate) {
        const { target } = state;

        // 限制预测时间
        const extrapolationTime = Math.min(timeSinceUpdate - this.interpolationDelay, this.maxExtrapolation);

        // 速度外推（单位转换：像素/秒 -> 像素/毫秒）
        const vx = target.vx || 0;
        const vy = target.vy || 0;

        // 预测位置 = 当前位置 + 速度 * 时间
        const x = target.x + vx * (extrapolationTime / 1000);
        const y = target.y + vy * (extrapolationTime / 1000);

        return { x, y };
    }

    /**
     * 线性插值（LERP）- 简单版本，备用
     */
    linearInterpolation(state, alpha) {
        const { previous, target } = state;

        return {
            x: previous.x + (target.x - previous.x) * alpha,
            y: previous.y + (target.y - previous.y) * alpha
        };
    }

    /**
     * 平滑插值（结合历史数据的加权平均）
     */
    smoothInterpolation(entityId, newPosition) {
        const state = this.entityStates.get(entityId);
        if (!state.current) {
            state.current = { ...newPosition };
            return newPosition;
        }

        // 指数平滑
        state.current.x += (newPosition.x - state.current.x) * this.smoothingFactor;
        state.current.y += (newPosition.y - state.current.y) * this.smoothingFactor;

        return { ...state.current };
    }

    /**
     * 移除实体
     */
    removeEntity(entityId) {
        this.entityStates.delete(entityId);
    }

    /**
     * 清空所有实体
     */
    clear() {
        this.entityStates.clear();
    }

    /**
     * 获取统计信息（用于调试）
     */
    getStats() {
        const now = performance.now();
        let totalLatency = 0;
        let count = 0;

        this.entityStates.forEach(state => {
            totalLatency += now - state.lastUpdateTime;
            count++;
        });

        return {
            entityCount: count,
            averageLatency: count > 0 ? totalLatency / count : 0,
            interpolationDelay: this.interpolationDelay
        };
    }
}
