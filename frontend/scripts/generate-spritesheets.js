/**
 * 生成精灵表脚本
 * 将单帧图片合并为精灵表（spritesheet）
 *
 * 原始帧尺寸: 204x408 (包含两个僵尸，上下堆叠)
 * 裁剪后尺寸: 204x204 (只保留上半部分的单个僵尸)
 */

const fs = require('fs');
const path = require('path');
const sharp = require('sharp');

// 配置
const SPRITE_WIDTH = 204;   // 裁剪后的精灵宽度
const SPRITE_HEIGHT = 204;  // 裁剪后的精灵高度（原始408的上半部分）
const COLUMNS = 10;         // 每行10个精灵

// 动画配置
const animations = [
    {
        name: 'zombie_walk',
        inputDir: '../assets/images/zombies/normal/walk',
        outputFile: '../assets/images/zombies/zombie_walk_spritesheet.png',
        frameCount: 46,  // 与旧项目一致
        cropTop: true    // 裁剪上半部分
    },
    {
        name: 'zombie_eat',
        inputDir: '../assets/images/zombies/normal/eat',
        outputFile: '../assets/images/zombies/zombie_eat_spritesheet.png',
        frameCount: 39,  // 与旧项目一致
        cropTop: true
    }
];

async function generateSpritesheet(config) {
    const { name, inputDir, outputFile, frameCount, cropTop } = config;

    const inputPath = path.resolve(__dirname, inputDir);
    const outputPath = path.resolve(__dirname, outputFile);

    console.log(`\n生成 ${name} 精灵表...`);
    console.log(`输入目录: ${inputPath}`);
    console.log(`输出文件: ${outputPath}`);

    // 计算精灵表尺寸
    const rows = Math.ceil(frameCount / COLUMNS);
    const sheetWidth = COLUMNS * SPRITE_WIDTH;
    const sheetHeight = rows * SPRITE_HEIGHT;

    console.log(`精灵表尺寸: ${sheetWidth}x${sheetHeight} (${COLUMNS}列 x ${rows}行)`);
    console.log(`帧数: ${frameCount}, 每帧: ${SPRITE_WIDTH}x${SPRITE_HEIGHT}`);

    // 准备合成层
    const composites = [];
    let loadedCount = 0;

    for (let i = 0; i < frameCount; i++) {
        const framePath = path.join(inputPath, `frame_${i}.png`);

        if (!fs.existsSync(framePath)) {
            console.warn(`  警告: 帧 ${i} 不存在: ${framePath}`);
            continue;
        }

        try {
            // 计算在精灵表中的位置
            const col = i % COLUMNS;
            const row = Math.floor(i / COLUMNS);
            const destX = col * SPRITE_WIDTH;
            const destY = row * SPRITE_HEIGHT;

            // 读取并裁剪图片
            let frameBuffer;
            if (cropTop) {
                // 获取图片元数据
                const metadata = await sharp(framePath).metadata();
                const cropHeight = Math.floor(metadata.height / 2);

                // 裁剪上半部分并调整大小
                frameBuffer = await sharp(framePath)
                    .extract({ left: 0, top: 0, width: metadata.width, height: cropHeight })
                    .resize(SPRITE_WIDTH, SPRITE_HEIGHT)
                    .toBuffer();
            } else {
                frameBuffer = await sharp(framePath)
                    .resize(SPRITE_WIDTH, SPRITE_HEIGHT)
                    .toBuffer();
            }

            composites.push({
                input: frameBuffer,
                left: destX,
                top: destY
            });

            loadedCount++;
        } catch (err) {
            console.error(`  错误: 加载帧 ${i} 失败:`, err.message);
        }
    }

    console.log(`  成功加载 ${loadedCount}/${frameCount} 帧`);

    // 确保输出目录存在
    const outputDir = path.dirname(outputPath);
    if (!fs.existsSync(outputDir)) {
        fs.mkdirSync(outputDir, { recursive: true });
    }

    // 创建透明背景并合成所有帧
    await sharp({
        create: {
            width: sheetWidth,
            height: sheetHeight,
            channels: 4,
            background: { r: 0, g: 0, b: 0, alpha: 0 }
        }
    })
        .composite(composites)
        .png()
        .toFile(outputPath);

    console.log(`  精灵表已保存: ${outputPath}`);

    // 返回精灵表信息（用于生成JSON配置）
    return {
        name,
        file: path.basename(outputFile),
        frameWidth: SPRITE_WIDTH,
        frameHeight: SPRITE_HEIGHT,
        columns: COLUMNS,
        rows,
        frameCount: loadedCount
    };
}

async function main() {
    console.log('=== 精灵表生成工具 ===');
    console.log(`精灵尺寸: ${SPRITE_WIDTH}x${SPRITE_HEIGHT}`);
    console.log(`每行列数: ${COLUMNS}`);

    const results = [];

    for (const anim of animations) {
        try {
            const result = await generateSpritesheet(anim);
            results.push(result);
        } catch (err) {
            console.error(`生成 ${anim.name} 失败:`, err);
        }
    }

    // 输出配置信息
    console.log('\n=== 精灵表配置 ===');
    console.log(JSON.stringify(results, null, 2));

    // 保存配置文件
    const configPath = path.resolve(__dirname, '../assets/images/zombies/spritesheet-config.json');
    fs.writeFileSync(configPath, JSON.stringify(results, null, 2));
    console.log(`\n配置已保存: ${configPath}`);
}

main().catch(console.error);
