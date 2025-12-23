#!/usr/bin/env python3
"""
从 Crazy_Dave.png 创建 Dave 行走动画精灵表
"""

from PIL import Image
import os
import math

def is_pure_background(pixel):
    """只检测纯背景蓝色，保留牛仔裤等深蓝色"""
    if len(pixel) >= 4 and pixel[3] < 128:
        return True
    r, g, b = pixel[:3]

    # 只移除非常明显的背景蓝色
    # 深蓝背景 (约24, 60, 139) - r很小, b很大
    if r < 40 and g < 80 and 120 < b < 160:
        return True

    # 亮蓝色背景/格子线 (约0, 174, 239)
    if r < 80 and g > 140 and b > 200:
        return True

    # 浅蓝色格子线 (各种变体)
    if r < 120 and g > 150 and b > 200:
        return True

    # 更多蓝色格子线
    if r < 80 and g > 120 and b > 180:
        return True

    return False

def remove_background(img):
    """移除背景"""
    img = img.convert('RGBA')
    pixels = img.load()
    w, h = img.size
    for y in range(h):
        for x in range(w):
            if is_pure_background(pixels[x, y]):
                pixels[x, y] = (0, 0, 0, 0)
    return img

def create_dave_spritesheet():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)

    input_path = os.path.join(project_root, 'Crazy_Dave.png')
    output_path = os.path.join(project_root, 'frontend', 'raw_sprites', 'dave_walk_spritesheet.png')

    print(f"加载: {input_path}")
    img = Image.open(input_path).convert('RGBA')

    # 提取完整的Dave区域（上半身+下半身一起提取）
    # 从分析可知，上半身在x≈1050，裤子在x≈850
    # 提取一个包含两者的大区域，然后分别处理

    # 上半身（头+锅+T恤）- 扩大区域包含完整的锅帽
    upper = remove_background(img.crop((1020, 0, 1200, 160)))
    upper_bbox = upper.getbbox()
    if upper_bbox:
        upper = upper.crop(upper_bbox)

    # 裤子+脚（保留更多，包括蓝色格子）
    pants_raw = img.crop((850, 0, 960, 210))
    pants = remove_background(pants_raw)
    pants_bbox = pants.getbbox()
    if pants_bbox:
        pants = pants.crop(pants_bbox)

    # 手（用于摆动动画）
    hand1 = remove_background(img.crop((705, 50, 760, 120)))
    hand2 = remove_background(img.crop((785, 45, 855, 125)))

    h1_bbox = hand1.getbbox()
    h2_bbox = hand2.getbbox()
    if h1_bbox:
        hand1 = hand1.crop(h1_bbox)
    if h2_bbox:
        hand2 = hand2.crop(h2_bbox)

    print(f"上半身: {upper.size}")
    print(f"裤子: {pants.size}")
    print(f"手1: {hand1.size if hand1.getbbox() else 'empty'}")
    print(f"手2: {hand2.size if hand2.getbbox() else 'empty'}")

    # 帧设置
    frame_w, frame_h = 180, 300
    num_frames = 16

    frames = []
    for i in range(num_frames):
        frame = Image.new('RGBA', (frame_w, frame_h), (0, 0, 0, 0))

        # 动画周期
        t = (i % 8) / 8.0
        angle = t * 2 * math.pi

        # 动画效果
        bob = int(math.sin(angle * 2) * 2)
        sway = int(math.sin(angle) * 2)
        hand_swing = int(math.sin(angle) * 12)

        # 位置计算
        cx = frame_w // 2
        base_y = frame_h - 5

        # 放置裤子
        px = cx - pants.width // 2 + sway
        py = base_y - pants.height
        frame.paste(pants, (px, py), pants)

        # 放置上半身（与裤子紧密重叠）
        overlap = 50
        ux = cx - upper.width // 2
        uy = py - upper.height + overlap + bob
        frame.paste(upper, (ux, uy), upper)

        # 放置手（摆动）
        if hand1.getbbox():
            hx1 = ux - hand1.width + 15 + hand_swing
            hy1 = uy + 30 + abs(hand_swing) // 4
            frame.paste(hand1, (hx1, hy1), hand1)

        if hand2.getbbox():
            hx2 = ux + upper.width - 15 - hand_swing
            hy2 = uy + 30 + abs(hand_swing) // 4
            frame.paste(hand2, (hx2, hy2), hand2)

        frames.append(frame)

    # 创建精灵表
    cols, rows = 4, 4
    sheet = Image.new('RGBA', (cols * frame_w, rows * frame_h), (0, 0, 0, 0))
    for i, f in enumerate(frames):
        sheet.paste(f, ((i % cols) * frame_w, (i // cols) * frame_h))

    # 保存
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    sheet.save(output_path, 'PNG')

    public_path = os.path.join(project_root, 'frontend', 'public', 'assets', 'sprites', 'dave_walk_spritesheet.png')
    os.makedirs(os.path.dirname(public_path), exist_ok=True)
    sheet.save(public_path, 'PNG')

    print(f"\n保存: {output_path}")
    print(f"帧: {frame_w}x{frame_h}, 共{num_frames}帧")

    return frame_w, frame_h, num_frames

if __name__ == '__main__':
    result = create_dave_spritesheet()
    if result:
        print(f"\nGameScene.js: frameWidth={result[0]}, frameHeight={result[1]}, endFrame={result[2]-1}")
