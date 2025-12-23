#!/usr/bin/env python3
"""
从 Crazy Dave.png 素材图中提取部件，去除紫色背景，
组合成行走动画精灵表
"""

from PIL import Image
import os

# 路径配置
INPUT_PATH = "/home/user/PVZ-H/frontend/raw_sprites/Crazy Dave.png"
OUTPUT_DIR = "/home/user/PVZ-H/frontend/public/assets/sprites"
OUTPUT_PATH = os.path.join(OUTPUT_DIR, "dave_walk_spritesheet.png")
PARTS_DIR = "/home/user/PVZ-H/frontend/raw_sprites/dave_parts"

# 紫色背景的颜色范围 (RGB)
PURPLE_BG = (139, 123, 168)
TOLERANCE = 45

def is_purple_bg(pixel, tolerance=TOLERANCE):
    """检查像素是否是紫色背景"""
    if len(pixel) == 4:
        r, g, b, a = pixel
        if a == 0:
            return False
    else:
        r, g, b = pixel

    return (abs(r - PURPLE_BG[0]) < tolerance and
            abs(g - PURPLE_BG[1]) < tolerance and
            abs(b - PURPLE_BG[2]) < tolerance)

def is_text_color(pixel):
    """检查像素是否是文字颜色（白色/浅灰色）"""
    if len(pixel) == 4:
        r, g, b, a = pixel
        if a == 0:
            return False
    else:
        r, g, b = pixel

    # 白色文字 (高亮度，低饱和度)
    brightness = (r + g + b) / 3
    if brightness > 200 and abs(r - g) < 30 and abs(g - b) < 30 and abs(r - b) < 30:
        return True
    return False

def remove_background_and_text(img):
    """移除紫色背景和白色文字"""
    img = img.convert("RGBA")
    pixels = img.load()
    width, height = img.size

    for y in range(height):
        for x in range(width):
            if is_purple_bg(pixels[x, y]) or is_text_color(pixels[x, y]):
                pixels[x, y] = (0, 0, 0, 0)

    return img

def extract_region(img, x, y, w, h):
    """从图片中提取指定区域"""
    return img.crop((x, y, x + w, y + h))

def trim_transparent(img):
    """裁剪掉图片周围的透明区域"""
    bbox = img.getbbox()
    if bbox:
        return img.crop(bbox)
    return img

def create_walk_frames(parts):
    """创建行走动画帧"""
    frames = []

    head = parts['head']
    pot = parts['pot']
    body = parts['body']
    left_arm = parts['left_arm']
    right_arm = parts['right_arm']
    legs = parts['legs']
    left_foot = parts['left_foot']
    right_foot = parts['right_foot']

    frame_width = 140
    frame_height = 240

    for frame_idx in range(6):
        frame = Image.new("RGBA", (frame_width, frame_height), (0, 0, 0, 0))

        # 动画参数
        body_bob = [0, -2, -3, -2, 0, 1][frame_idx]
        arm_swing = [-4, -2, 0, 2, 4, 2][frame_idx]
        left_foot_x = [0, -2, -5, -2, 0, 2][frame_idx]
        right_foot_x = [0, 2, 5, 2, 0, -2][frame_idx]
        left_foot_y = [0, -2, 0, 2, 0, -2][frame_idx]
        right_foot_y = [0, 2, 0, -2, 0, 2][frame_idx]

        base_x = frame_width // 2

        # 1. 锅
        pot_scale = 0.5
        pot_resized = pot.resize((int(pot.width * pot_scale), int(pot.height * pot_scale)), Image.Resampling.LANCZOS)
        pot_x = base_x - pot_resized.width // 2 + 3
        pot_y = 2 + body_bob
        frame.paste(pot_resized, (pot_x, pot_y), pot_resized)

        # 2. 头部
        head_scale = 0.55
        head_resized = head.resize((int(head.width * head_scale), int(head.height * head_scale)), Image.Resampling.LANCZOS)
        head_x = base_x - head_resized.width // 2
        head_y = pot_y + pot_resized.height - 15 + body_bob
        frame.paste(head_resized, (head_x, head_y), head_resized)

        # 3. 身体
        body_scale = 0.38
        body_resized = body.resize((int(body.width * body_scale), int(body.height * body_scale)), Image.Resampling.LANCZOS)
        body_x = base_x - body_resized.width // 2
        body_y = head_y + head_resized.height - 15 + body_bob

        # 4. 手臂
        arm_scale = 0.35
        right_arm_resized = right_arm.resize((int(right_arm.width * arm_scale), int(right_arm.height * arm_scale)), Image.Resampling.LANCZOS)
        left_arm_resized = left_arm.resize((int(left_arm.width * arm_scale), int(left_arm.height * arm_scale)), Image.Resampling.LANCZOS)

        right_arm_x = body_x + body_resized.width - 10 + arm_swing
        right_arm_y = body_y + 6
        left_arm_x = body_x - left_arm_resized.width + 12 - arm_swing
        left_arm_y = body_y + 5

        # 绘制层次
        if frame_idx < 3:
            frame.paste(left_arm_resized, (left_arm_x, left_arm_y), left_arm_resized)
            frame.paste(body_resized, (body_x, body_y), body_resized)
            frame.paste(right_arm_resized, (right_arm_x, right_arm_y), right_arm_resized)
        else:
            frame.paste(right_arm_resized, (right_arm_x, right_arm_y), right_arm_resized)
            frame.paste(body_resized, (body_x, body_y), body_resized)
            frame.paste(left_arm_resized, (left_arm_x, left_arm_y), left_arm_resized)

        # 5. 腿部
        legs_scale = 0.3
        legs_resized = legs.resize((int(legs.width * legs_scale), int(legs.height * legs_scale)), Image.Resampling.LANCZOS)
        legs_x = base_x - legs_resized.width // 2
        legs_y = body_y + body_resized.height - 12
        frame.paste(legs_resized, (legs_x, legs_y), legs_resized)

        # 6. 脚
        foot_scale = 0.3
        left_foot_resized = left_foot.resize((int(left_foot.width * foot_scale), int(left_foot.height * foot_scale)), Image.Resampling.LANCZOS)
        right_foot_resized = right_foot.resize((int(right_foot.width * foot_scale), int(right_foot.height * foot_scale)), Image.Resampling.LANCZOS)

        left_foot_final_x = legs_x + 2 + left_foot_x
        left_foot_final_y = legs_y + legs_resized.height - 12 + left_foot_y
        right_foot_final_x = legs_x + legs_resized.width - right_foot_resized.width - 2 + right_foot_x
        right_foot_final_y = legs_y + legs_resized.height - 12 + right_foot_y

        frame.paste(left_foot_resized, (left_foot_final_x, left_foot_final_y), left_foot_resized)
        frame.paste(right_foot_resized, (right_foot_final_x, right_foot_final_y), right_foot_resized)

        frames.append(frame)

    return frames

def main():
    print("加载源图片...")
    source = Image.open(INPUT_PATH)
    print(f"图片尺寸: {source.size}")

    source = source.convert("RGBA")

    print("移除紫色背景和白色文字...")
    source = remove_background_and_text(source)

    os.makedirs(OUTPUT_DIR, exist_ok=True)
    os.makedirs(PARTS_DIR, exist_ok=True)

    print("提取各部件...")
    # 更精确的坐标 - 进一步避开文字
    # 图片尺寸: 872 x 1300

    # 头部 - 向下偏移更多避开 "Head & Facial" 文字
    head = extract_region(source, 10, 70, 130, 125)
    head = trim_transparent(head)
    head.save(os.path.join(PARTS_DIR, "head.png"))
    print("  - 头部已提取")

    # 锅 - 向下偏移避开 "Pot" 文字
    pot = extract_region(source, 10, 238, 135, 60)
    pot = trim_transparent(pot)
    pot.save(os.path.join(PARTS_DIR, "pot.png"))
    print("  - 锅已提取")

    # 身体 - 只取白T恤部分，避开上方的锅和文字
    body = extract_region(source, 10, 370, 200, 230)
    body = trim_transparent(body)
    body.save(os.path.join(PARTS_DIR, "body.png"))
    print("  - 身体已提取")

    # 右手臂
    right_arm = extract_region(source, 290, 355, 80, 110)
    right_arm = trim_transparent(right_arm)
    right_arm.save(os.path.join(PARTS_DIR, "right_arm.png"))
    print("  - 右手臂已提取")

    # 左手臂
    left_arm = extract_region(source, 422, 355, 85, 100)
    left_arm = trim_transparent(left_arm)
    left_arm.save(os.path.join(PARTS_DIR, "left_arm.png"))
    print("  - 左手臂已提取")

    # 腿部
    legs = extract_region(source, 10, 700, 165, 255)
    legs = trim_transparent(legs)
    legs.save(os.path.join(PARTS_DIR, "legs.png"))
    print("  - 腿部已提取")

    # 左脚
    left_foot = extract_region(source, 230, 760, 110, 45)
    left_foot = trim_transparent(left_foot)
    left_foot.save(os.path.join(PARTS_DIR, "left_foot.png"))
    print("  - 左脚已提取")

    # 右脚
    right_foot = extract_region(source, 390, 760, 120, 45)
    right_foot = trim_transparent(right_foot)
    right_foot.save(os.path.join(PARTS_DIR, "right_foot.png"))
    print("  - 右脚已提取")

    parts = {
        'head': head,
        'pot': pot,
        'body': body,
        'left_arm': left_arm,
        'right_arm': right_arm,
        'legs': legs,
        'left_foot': left_foot,
        'right_foot': right_foot
    }

    print("\n创建行走动画帧...")
    frames = create_walk_frames(parts)

    print(f"生成精灵表 ({len(frames)} 帧)...")
    frame_width = frames[0].width
    frame_height = frames[0].height
    spritesheet = Image.new("RGBA", (frame_width * len(frames), frame_height), (0, 0, 0, 0))

    for i, frame in enumerate(frames):
        spritesheet.paste(frame, (i * frame_width, 0))

    spritesheet.save(OUTPUT_PATH)
    print(f"\n精灵表已保存到: {OUTPUT_PATH}")
    print(f"帧尺寸: {frame_width} x {frame_height}")
    print(f"总帧数: {len(frames)}")

if __name__ == "__main__":
    main()
