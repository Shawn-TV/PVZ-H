#!/usr/bin/env python3
"""
从 Crazy_Dave.png 提取部件并组合成完整的 Dave 行走动画精灵表
参照用户提供的完整 Dave 参考图
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

    # 提取各部件
    # 1. 上半身（头+锅+T恤，包含手臂交叉在胸前的姿势）
    upper_body = extract_and_clean(img, 1050, 0, 160, 155)
    print(f"上半身尺寸: {upper_body.size}")

    # 2. 左手臂（向前伸出）
    left_arm = extract_and_clean(img, 700, 30, 80, 100)
    print(f"左手臂尺寸: {left_arm.size}")

    # 3. 右手臂（向前伸出）
    right_arm = extract_and_clean(img, 780, 30, 80, 100)
    print(f"右手臂尺寸: {right_arm.size}")

    # 4. 牛仔裤+腿+脚
    legs = extract_and_clean(img, 855, 10, 90, 195)
    print(f"腿部尺寸: {legs.size}")

    # 帧大小
    frame_w, frame_h = 200, 280
    print(f"帧大小: {frame_w}x{frame_h}")

    # 创建行走动画帧
    num_frames = 16
    frames = []

    for i in range(num_frames):
        frame = Image.new('RGBA', (frame_w, frame_h), (0, 0, 0, 0))

        # 行走周期动画效果
        walk_phase = (i % 8) / 8.0
        angle = walk_phase * 2 * math.pi

        # 动画偏移
        body_bob = int(math.sin(angle * 2) * 3)  # 身体上下摆动
        arm_swing = int(math.sin(angle) * 8)     # 手臂前后摆动
        leg_swing = int(math.sin(angle) * 4)     # 腿部左右晃动

        # 基准位置（从下往上构建）
        base_y = frame_h - 10

        # 1. 放置腿部（底部）
        legs_x = (frame_w - legs.width) // 2 + leg_swing
        legs_y = base_y - legs.height
        frame.paste(legs, (legs_x, legs_y), legs)

        # 2. 放置上半身（与腿部重叠）
        overlap = 50  # 上半身和腿部重叠量
        upper_x = (frame_w - upper_body.width) // 2
        upper_y = legs_y - upper_body.height + overlap + body_bob
        frame.paste(upper_body, (upper_x, upper_y), upper_body)

        # 3. 放置手臂（在身体两侧）
        # 左手臂
        left_arm_x = upper_x - left_arm.width + 25 - arm_swing
        left_arm_y = upper_y + 40 + abs(arm_swing) // 2
        frame.paste(left_arm, (left_arm_x, left_arm_y), left_arm)

        # 右手臂
        right_arm_x = upper_x + upper_body.width - 25 + arm_swing
        right_arm_y = upper_y + 40 + abs(arm_swing) // 2
        frame.paste(right_arm, (right_arm_x, right_arm_y), right_arm)

        frames.append(frame)

    # 创建精灵表 (4x4 = 16帧)
    cols, rows = 4, 4
    sheet = Image.new('RGBA', (cols * frame_w, rows * frame_h), (0, 0, 0, 0))

    for i, frame_img in enumerate(frames):
        x = (i % cols) * frame_w
        y = (i // cols) * frame_h
        sheet.paste(frame_img, (x, y))

    # 保存
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    sheet.save(output_path, 'PNG')
    print(f"\n已保存: {output_path}")
    print(f"精灵表尺寸: {sheet.size}")
    print(f"帧大小: {frame_w}x{frame_h}")
    print(f"总帧数: {num_frames}")

    # 复制到 public 目录
    public_path = os.path.join(project_root, 'frontend', 'public', 'assets', 'sprites', 'dave_walk_spritesheet.png')
    os.makedirs(os.path.dirname(public_path), exist_ok=True)
    sheet.save(public_path, 'PNG')
    print(f"已复制到: {public_path}")

    return frame_w, frame_h, num_frames

if __name__ == '__main__':
    result = create_dave_walk_spritesheet()
    if result:
        frame_w, frame_h, num_frames = result
        print(f"\n请更新 GameScene.js:")
        print(f"  frameWidth: {frame_w}")
        print(f"  frameHeight: {frame_h}")
        print(f"  endFrame: {num_frames - 1}")
