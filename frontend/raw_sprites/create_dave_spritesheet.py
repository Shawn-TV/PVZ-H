#!/usr/bin/env python3
"""
Create Dave walk animation spritesheet from Crazy_Dave.png body parts
Since the original image doesn't have legs, we'll create them programmatically
"""

from PIL import Image, ImageDraw
import os
import math

# Background colors in Crazy_Dave.png
LIGHT_BLUE_BG = (0, 176, 248)
DARK_BLUE_BG = (0, 44, 122)

# Dave's jeans colors (for drawing legs)
JEANS_DARK = (70, 90, 140)
JEANS_LIGHT = (90, 110, 160)
JEANS_HIGHLIGHT = (110, 130, 180)
SHOE_COLOR = (60, 50, 40)

def is_bg(pixel, tolerance=40):
    if len(pixel) >= 4 and pixel[3] < 128:
        return True
    r, g, b = pixel[:3]
    if abs(r - 0) < tolerance and abs(g - 176) < tolerance and abs(b - 248) < tolerance:
        return True
    if abs(r - 0) < tolerance and abs(g - 44) < tolerance and abs(b - 122) < tolerance:
        return True
    return False

def remove_bg(img, tolerance=40):
    img = img.convert('RGBA')
    pixels = img.load()
    w, h = img.size
    for y in range(h):
        for x in range(w):
            if is_bg(pixels[x, y], tolerance):
                pixels[x, y] = (0, 0, 0, 0)
    return img

def extract_upper_body(img_path):
    """Extract Dave's upper body from the sprite sheet by combining head and torso"""
    img = Image.open(img_path)
    img = img.convert('RGBA')

    # Extract head with beard (from the left part of the assembled figure)
    head = img.crop((468, 0, 560, 180))
    head = remove_bg(head, 50)
    head_bbox = head.getbbox()

    # Extract torso (from the right part)
    torso = img.crop((560, 0, 720, 180))
    torso = remove_bg(torso, 50)
    torso_bbox = torso.getbbox()

    if not head_bbox or not torso_bbox:
        # Fallback to simple extraction
        upper_body = img.crop((468, 0, 720, 180))
        upper_body = remove_bg(upper_body, 50)
        bbox = upper_body.getbbox()
        if bbox:
            upper_body = upper_body.crop(bbox)
        return upper_body

    # Crop to content
    head = head.crop(head_bbox)
    torso = torso.crop(torso_bbox)

    head_w, head_h = head.size
    torso_w, torso_h = torso.size

    # Create composite: head on left, torso on right, with overlap
    overlap = 30
    total_w = head_w + torso_w - overlap
    total_h = max(head_h, torso_h)

    composite = Image.new('RGBA', (total_w, total_h), (0, 0, 0, 0))

    # Paste torso first (so head overlaps it)
    torso_x = head_w - overlap
    torso_y = total_h - torso_h
    composite.paste(torso, (torso_x, torso_y), torso)

    # Paste head
    head_x = 0
    head_y = total_h - head_h
    composite.paste(head, (head_x, head_y), head)

    # Trim any empty space
    bbox = composite.getbbox()
    if bbox:
        composite = composite.crop(bbox)

    return composite

def draw_legs(width, height, walk_phase=0):
    """Draw Dave's legs (blue jeans) with walking animation

    walk_phase: 0-1 value representing position in walk cycle
    """
    leg_img = Image.new('RGBA', (width, height), (0, 0, 0, 0))
    draw = ImageDraw.Draw(leg_img)

    center_x = width // 2
    leg_width = width // 3
    leg_height = height - 10

    # Walking animation - legs swing forward and back
    angle = walk_phase * 2 * math.pi

    # Left leg offset
    left_offset_x = int(math.sin(angle) * 6)
    left_offset_y = int(abs(math.sin(angle)) * 3)

    # Right leg offset (opposite phase)
    right_offset_x = int(math.sin(angle + math.pi) * 6)
    right_offset_y = int(abs(math.sin(angle + math.pi)) * 3)

    # Draw left leg
    left_x = center_x - leg_width // 2 - 4 + left_offset_x
    left_top = 0
    left_bottom = leg_height - left_offset_y

    # Left leg shape (jeans)
    draw.polygon([
        (left_x, left_top),
        (left_x + leg_width, left_top),
        (left_x + leg_width - 1, left_bottom - 6),
        (left_x + 1, left_bottom - 6),
    ], fill=JEANS_DARK + (255,))

    # Left shoe
    draw.ellipse([
        left_x - 2, left_bottom - 8,
        left_x + leg_width + 2, left_bottom
    ], fill=SHOE_COLOR + (255,))

    # Draw right leg
    right_x = center_x + 4 + right_offset_x
    right_top = 0
    right_bottom = leg_height - right_offset_y

    # Right leg shape (jeans)
    draw.polygon([
        (right_x, right_top),
        (right_x + leg_width, right_top),
        (right_x + leg_width - 1, right_bottom - 6),
        (right_x + 1, right_bottom - 6),
    ], fill=JEANS_LIGHT + (255,))

    # Right shoe
    draw.ellipse([
        right_x - 2, right_bottom - 8,
        right_x + leg_width + 2, right_bottom
    ], fill=SHOE_COLOR + (255,))

    # Add some jeans detail (lighter stripe for fold)
    draw.line([
        (left_x + leg_width//2, left_top + 2),
        (left_x + leg_width//2, left_bottom - 8)
    ], fill=JEANS_HIGHLIGHT + (255,), width=1)

    return leg_img

def create_complete_dave(upper_body, legs):
    """Combine upper body and legs into complete Dave figure"""
    ub_w, ub_h = upper_body.size
    legs_w, legs_h = legs.size

    # Calculate total height
    overlap = 8  # Overlap between torso and legs
    total_h = ub_h + legs_h - overlap
    total_w = max(ub_w, legs_w)

    complete = Image.new('RGBA', (total_w, total_h), (0, 0, 0, 0))

    # Paste upper body
    ub_x = (total_w - ub_w) // 2
    complete.paste(upper_body, (ub_x, 0), upper_body)

    # Paste legs
    legs_x = (total_w - legs_w) // 2
    legs_y = ub_h - overlap
    complete.paste(legs, (legs_x, legs_y), legs)

    return complete

def create_walk_spritesheet(img_path, output_path, num_frames=47):
    """Create Dave walk animation spritesheet"""
    print(f"Loading {img_path}...")

    # Extract upper body
    upper_body = extract_upper_body(img_path)
    print(f"Extracted upper body: {upper_body.size}")

    # Target frame size (similar to zombie walk)
    frame_w = 128
    frame_h = 128

    # Create frames
    frames = []

    # Scale upper body to fit
    ub_scale = 0.55
    ub_w = int(upper_body.width * ub_scale)
    ub_h = int(upper_body.height * ub_scale)
    scaled_upper = upper_body.resize((ub_w, ub_h), Image.Resampling.LANCZOS)

    # Leg dimensions
    leg_w = int(ub_w * 0.65)
    leg_h = int(ub_h * 0.40)

    for i in range(num_frames):
        frame = Image.new('RGBA', (frame_w, frame_h), (0, 0, 0, 0))

        # Walk cycle phase (12 frames per cycle)
        walk_phase = (i % 12) / 12.0

        # Draw legs for this frame
        legs = draw_legs(leg_w, leg_h, walk_phase)

        # Combine upper body and legs
        complete_dave = create_complete_dave(scaled_upper, legs)

        # Add body bob
        bob_y = int(math.sin(walk_phase * 2 * math.pi) * 2)

        # Center in frame
        dave_w, dave_h = complete_dave.size
        x = (frame_w - dave_w) // 2
        y = (frame_h - dave_h) // 2 + bob_y + 3

        frame.paste(complete_dave, (x, y), complete_dave)
        frames.append(frame)

    # Create spritesheet (8 columns x 6 rows)
    cols = 8
    rows = (num_frames + cols - 1) // cols
    sheet_w = cols * frame_w
    sheet_h = rows * frame_h

    spritesheet = Image.new('RGBA', (sheet_w, sheet_h), (0, 0, 0, 0))

    for i, frame in enumerate(frames):
        col = i % cols
        row = i // cols
        x = col * frame_w
        y = row * frame_h
        spritesheet.paste(frame, (x, y))

    spritesheet.save(output_path, 'PNG')
    print(f"Saved spritesheet to {output_path}")
    print(f"Spritesheet size: {spritesheet.size}")
    print(f"Frames: {num_frames} ({cols}x{rows})")

    return spritesheet

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(script_dir)

    input_path = 'Crazy_Dave.png'
    output_path = 'dave_walk_spritesheet.png'

    if not os.path.exists(input_path):
        print(f"Error: {input_path} not found")
        return

    create_walk_spritesheet(input_path, output_path)
    print("Done!")

if __name__ == '__main__':
    main()
