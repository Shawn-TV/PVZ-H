#!/usr/bin/env python3
"""
从 Crazy_Dave.png 提取部件并组合成完整的 Dave 行走动画精灵表
"""

from PIL import Image
import os
import math

def is_blue_color(pixel):
    """检查像素是否是任何蓝色（背景或格子线）"""
    if len(pixel) >= 4 and pixel[3] < 128:
        return True
    r, g, b = pixel[:3]

    # 深蓝色背景 (24, 60, 139)
    if r < 80 and g < 120 and b > 100 and b > r + 30:
        return True

    # 浅蓝/亮蓝色格子线
    if r < 150 and g < 200 and b > 180:
        return True

    # 天蓝色 (0, 174, 239) 类似
    if r < 100 and g > 140 and b > 200:
        return True

    return False

def remove_all_blue(img):
    """移除所有蓝色"""
    img = img.convert('RGBA')
    pixels = img.load()
    w, h = img.size
    for y in range(h):
        for x in range(w):
            if is_blue_color(pixels[x, y]):
                pixels[x, y] = (0, 0, 0, 0)
    return img

def extract_and_clean(img, x, y, w, h):
    """提取区域并清理所有蓝色"""
    region = img.crop((x, y, x + w, y + h))
    region = remove_all_blue(region)
    bbox = region.getbbox()
    if bbox:
        region = region.crop(bbox)
    return region

def create_dave_walk_spritesheet():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)

    input_path = os.path.join(project_root, 'Crazy_Dave.png')
    output_path = os.path.join(project_root, 'frontend', 'raw_sprites', 'dave_walk_spritesheet.png')

    print(f"加载图片: {input_path}")
    img = Image.open(input_path).convert('RGBA')
    print(f"图片尺寸: {img.size}")

    # 提取部件 (根据分析得到的坐标)
    # 上半身 Dave (头+锅+T恤) - 在 x≈1050, y≈0 区域
    upper_body = extract_and_clean(img, 1050, 0, 150, 150)
    print(f"上半身尺寸: {upper_body.size}")

    # 牛仔裤+腿+脚 (精确裁剪) - x≈858-938, y≈15-200
    legs = extract_and_clean(img, 858, 15, 80, 190)
    print(f"腿部尺寸: {legs.size}")

    # 计算组合后的尺寸
    overlap = 40  # 增加重叠使身体和腿部连接更紧密

    # 帧大小
    total_height = upper_body.height + legs.height - overlap
    total_width = max(upper_body.width, legs.width)
    frame_size = max(total_width, total_height) + 20
    frame_size = ((frame_size + 7) // 8) * 8
    print(f"帧大小: {frame_size}x{frame_size}")

    # 创建行走动画帧
    num_frames = 16
    frames = []

    for i in range(num_frames):
        frame = Image.new('RGBA', (frame_size, frame_size), (0, 0, 0, 0))

        # 行走周期动画效果
        walk_phase = (i % 8) / 8.0
        angle = walk_phase * 2 * math.pi

        # 上下摆动
        body_bob = int(math.sin(angle * 2) * 2)
        # 腿部轻微摇摆
        leg_offset = int(math.sin(angle) * 3)
        # 身体微微晃动
        body_sway = int(math.sin(angle) * 2)

        # 计算放置位置（底部对齐）
        base_y = frame_size - 5

        # 先放腿（在下方）
        legs_x = (frame_size - legs.width) // 2 + leg_offset
        legs_y = base_y - legs.height
        frame.paste(legs, (legs_x, legs_y), legs)

        # 再放上半身（在上方，和腿部重叠）
        upper_x = (frame_size - upper_body.width) // 2 + body_sway
        upper_y = legs_y - upper_body.height + overlap + body_bob
        frame.paste(upper_body, (upper_x, upper_y), upper_body)

        frames.append(frame)

    # 创建精灵表 (4x4 = 16帧)
    cols, rows = 4, 4
    sheet = Image.new('RGBA', (cols * frame_size, rows * frame_size), (0, 0, 0, 0))

    for i, frame_img in enumerate(frames):
        x = (i % cols) * frame_size
        y = (i // cols) * frame_size
        sheet.paste(frame_img, (x, y))

    # 保存
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    sheet.save(output_path, 'PNG')
    print(f"\n已保存: {output_path}")
    print(f"精灵表尺寸: {sheet.size}")
    print(f"帧大小: {frame_size}x{frame_size}")
    print(f"总帧数: {num_frames}")

    # 复制到 public 目录
    public_path = os.path.join(project_root, 'frontend', 'public', 'assets', 'sprites', 'dave_walk_spritesheet.png')
    os.makedirs(os.path.dirname(public_path), exist_ok=True)
    sheet.save(public_path, 'PNG')
    print(f"已复制到: {public_path}")

    return frame_size, num_frames

if __name__ == '__main__':
    result = create_dave_walk_spritesheet()
    if result:
        frame_size, num_frames = result
        print(f"\n请更新 GameScene.js:")
        print(f"  frameWidth: {frame_size}")
        print(f"  frameHeight: {frame_size}")
        print(f"  endFrame: {num_frames - 1}")
