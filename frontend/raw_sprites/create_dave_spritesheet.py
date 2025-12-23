#!/usr/bin/env python3
"""
Create Dave walk animation spritesheet from 'Crazy Dave.png' body parts
"""

from PIL import Image
import os
import math

PURPLE_BG = (143, 133, 198)

def is_bg(pixel, tolerance=35):
    if len(pixel) >= 4 and pixel[3] < 128:
        return True
    r, g, b = pixel[:3]
    pr, pg, pb = PURPLE_BG
    if abs(r - pr) < tolerance and abs(g - pg) < tolerance and abs(b - pb) < tolerance:
        return True
    if r > 240 and g > 240 and b > 240:
        return True
    return False

def remove_bg(img, tolerance=35):
    img = img.convert('RGBA')
    pixels = img.load()
    w, h = img.size
    for y in range(h):
        for x in range(w):
            if is_bg(pixels[x, y], tolerance):
                pixels[x, y] = (0, 0, 0, 0)
    return img

def extract_part(img, x, y, w, h):
    part = img.crop((x, y, x + w, y + h))
    part = remove_bg(part)
    bbox = part.getbbox()
    if bbox:
        part = part.crop(bbox)
    return part

def create_dave_walk_spritesheet(input_path, output_path, num_frames=47):
    print(f"Loading {input_path}...")

    img = Image.open(input_path).convert('RGBA')

    # Extract parts
    head = extract_part(img, 0, 45, 155, 160)
    pot = extract_part(img, 0, 210, 200, 100)
    body = extract_part(img, 0, 310, 245, 370)
    legs = extract_part(img, 0, 700, 245, 400)

    # Scale to fit in 128x128 frame
    scale = 0.14
    head_s = head.resize((int(head.width * scale), int(head.height * scale)), Image.Resampling.LANCZOS)
    pot_s = pot.resize((int(pot.width * scale * 0.45), int(pot.height * scale * 0.45)), Image.Resampling.LANCZOS)
    body_s = body.resize((int(body.width * scale), int(body.height * scale)), Image.Resampling.LANCZOS)
    legs_s = legs.resize((int(legs.width * scale), int(legs.height * scale)), Image.Resampling.LANCZOS)

    print(f"Parts: Head {head_s.size}, Pot {pot_s.size}, Body {body_s.size}, Legs {legs_s.size}")

    frame_w, frame_h = 128, 128
    frames = []

    # Overlaps
    ol_bl = 12  # body-legs
    ol_hb = 10  # head-body
    ol_ph = 8   # pot-head

    for i in range(num_frames):
        frame = Image.new('RGBA', (frame_w, frame_h), (0, 0, 0, 0))

        # Walk cycle
        walk_phase = (i % 12) / 12.0
        angle = walk_phase * 2 * math.pi

        # Animation offsets
        bob_y = int(math.sin(angle) * 2)
        leg_swing = int(math.sin(angle) * 3)
        head_tilt = int(math.sin(angle * 0.5) * 1)

        # Position from bottom up
        legs_y = frame_h - legs_s.height - 3 + bob_y
        legs_x = (frame_w - legs_s.width) // 2 + leg_swing
        frame.paste(legs_s, (legs_x, legs_y), legs_s)

        body_y = legs_y - body_s.height + ol_bl
        body_x = (frame_w - body_s.width) // 2
        frame.paste(body_s, (body_x, body_y), body_s)

        head_y = body_y - head_s.height + ol_hb
        head_x = (frame_w - head_s.width) // 2 + 2 + head_tilt
        frame.paste(head_s, (head_x, head_y), head_s)

        pot_y = head_y - pot_s.height + ol_ph
        pot_x = (frame_w - pot_s.width) // 2 + 4 + head_tilt
        frame.paste(pot_s, (pot_x, pot_y), pot_s)

        frames.append(frame)

    # Create spritesheet (8x6)
    cols, rows = 8, 6
    sheet = Image.new('RGBA', (cols * frame_w, rows * frame_h), (0, 0, 0, 0))

    for i, frame in enumerate(frames):
        x = (i % cols) * frame_w
        y = (i // cols) * frame_h
        sheet.paste(frame, (x, y))

    sheet.save(output_path, 'PNG')
    print(f"Saved: {output_path} ({sheet.size[0]}x{sheet.size[1]}, {num_frames} frames)")

def main():
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    create_dave_walk_spritesheet('Crazy Dave.png', 'dave_walk_spritesheet.png')
    print("Done!")

if __name__ == '__main__':
    main()
