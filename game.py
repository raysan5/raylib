from raylibpy import *
import math

# ---------- Constants ----------
ROWS = 3
COLS = 3
TILE_SIZE = 120
MARGIN = 20

# Animation settings
ANIM_SPEED = 0.3  # interpolation speed
FPS = 60

# ---------- Tile Class ----------
class Tile:
    def __init__(self, value, row, col):
        self.value = value
        self.row = row
        self.col = col
        self.x = MARGIN + col * TILE_SIZE
        self.y = MARGIN + row * TILE_SIZE
        self.target_row = row
        self.target_col = col

    def update(self):
        target_x = MARGIN + self.target_col * TILE_SIZE
        target_y = MARGIN + self.target_row * TILE_SIZE
        # Smooth interpolation
        self.x += (target_x - self.x) * ANIM_SPEED * FPS / 60
        self.y += (target_y - self.y) * ANIM_SPEED * FPS / 60

    def draw(self):
        color = LIGHTGRAY if self.value != 0 else GRAY
        draw_rectangle(int(self.x), int(self.y), TILE_SIZE - 4, TILE_SIZE - 4, color)
        if self.value != 0:
            text = str(self.value).encode('utf-8')
            tw = measure_text(text, 30)
            draw_text(text, int(self.x + TILE_SIZE / 2 - tw / 2),
                      int(self.y + TILE_SIZE / 2 - 15), 30, BLACK)

# ---------- Board ----------
tiles = []

def init_board():
    global tiles
    tiles = []
    n = ROWS * COLS
    values = [i + 1 for i in range(n - 1)] + [0]
    for idx, val in enumerate(values):
        r = idx // COLS
        c = idx % COLS
        tiles.append(Tile(val, r, c))

def find_empty():
    for idx, t in enumerate(tiles):
        if t.value == 0:
            return idx
    return -1

def swap_tiles(tile1, tile2):
    tile1.target_row, tile1.target_col, tile2.target_row, tile2.target_col = tile2.target_row, tile2.target_col, tile1.target_row, tile1.target_col
    tile1.row, tile1.col, tile2.row, tile2.col = tile2.row, tile2.col, tile1.row, tile1.col

def move_empty(direction):
    empty_idx = find_empty()
    er = tiles[empty_idx].row
    ec = tiles[empty_idx].col
    tr, tc = er, ec
    if direction == 0: tr -= 1  # Up
    elif direction == 1: tr += 1  # Down
    elif direction == 2: tc -= 1  # Left
    elif direction == 3: tc += 1  # Right
    if tr < 0 or tr >= ROWS or tc < 0 or tc >= COLS:
        return
    # Find target tile
    for t in tiles:
        if t.row == tr and t.col == tc:
            swap_tiles(t, tiles[empty_idx])
            break

# ---------- Main Loop ----------
def main():
    screenW = MARGIN * 2 + COLS * TILE_SIZE
    screenH = MARGIN * 2 + ROWS * TILE_SIZE
    init_window(screenW, screenH, b"Ultra Smooth Puzzle with Gestures")
    set_target_fps(FPS)
    init_board()

    swipe_start = None

    while not window_should_close():
        pos = get_mouse_position()

        # ---------- Mouse swipe detection ----------
        if is_mouse_button_pressed(MOUSE_LEFT_BUTTON):
            swipe_start = pos

        if is_mouse_button_released(MOUSE_LEFT_BUTTON) and swipe_start:
            swipe_end = pos
            dx = swipe_end.x - swipe_start.x
            dy = swipe_end.y - swipe_start.y
            dist = math.sqrt(dx * dx + dy * dy)
            if dist >= 8.0:
                if abs(dx) > abs(dy):
                    move_empty(3 if dx > 0 else 2)
                else:
                    move_empty(1 if dy > 0 else 0)
            swipe_start = None

        # ---------- Gesture detection ----------
        if is_gesture_detected(GESTURE_SWIPE_RIGHT):
            move_empty(3)
        elif is_gesture_detected(GESTURE_SWIPE_LEFT):
            move_empty(2)
        elif is_gesture_detected(GESTURE_SWIPE_UP):
            move_empty(0)
        elif is_gesture_detected(GESTURE_SWIPE_DOWN):
            move_empty(1)

        # ---------- Update tiles ----------
        for t in tiles:
            t.update()

        # ---------- Draw ----------
        begin_drawing()
        clear_background(RAYWHITE)
        draw_text(b"Swipe tiles! (Mouse or Gesture)", 10, 10, 18, DARKGRAY)
        for t in tiles:
            t.draw()
        end_drawing()

    close_window()

if __name__ == "__main__":
    main()
