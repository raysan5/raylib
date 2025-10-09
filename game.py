from raylibpy import *
import math

# Grid setup
ROWS = 3
COLS = 3
TILE_SIZE = 120
MARGIN = 20
SWIPE_MIN_DISTANCE = 30.0  # Minimum distance to detect swipe

# Board: 0 means empty tile, others are numbered
board = []

def init_board():
    global board
    board = [i + 1 for i in range(ROWS * COLS - 1)] + [0]  # 1..8 + 0 empty

def idx_row(idx):
    return idx // COLS

def idx_col(idx):
    return idx % COLS

def draw_board():
    for i, value in enumerate(board):
        r, c = idx_row(i), idx_col(i)
        x = MARGIN + c * TILE_SIZE
        y = MARGIN + r * TILE_SIZE
        if value == 0:
            draw_rectangle(x, y, TILE_SIZE - 4, TILE_SIZE - 4, DARKGRAY)
        else:
            draw_rectangle(x, y, TILE_SIZE - 4, TILE_SIZE - 4, LIGHTGRAY)
            draw_text(str(value), x + TILE_SIZE // 2 - 10, y + TILE_SIZE // 2 - 12, 24, BLACK)

def find_empty():
    return board.index(0)

def move_empty(direction):
    empty = find_empty()
    er, ec = idx_row(empty), idx_col(empty)
    tr, tc = er, ec

    if direction == 0: tr = er - 1   # up
    elif direction == 1: tr = er + 1 # down
    elif direction == 2: tc = ec - 1 # left
    elif direction == 3: tc = ec + 1 # right

    if tr < 0 or tr >= ROWS or tc < 0 or tc >= COLS:
        return  # out of bounds

    target = tr * COLS + tc
    board[empty], board[target] = board[target], board[empty]

def main():
    screenW = MARGIN * 2 + COLS * TILE_SIZE
    screenH = MARGIN * 2 + ROWS * TILE_SIZE

    init_window(screenW, screenH, b"Puzzle Swipe Test (Python)")
    set_target_fps(60)

    init_board()

    # Swipe variables
    touch_start = Vector2(0, 0)
    touch_end = Vector2(0, 0)
    touching = False

    while not window_should_close():
        # Detect touch or mouse drag
        if is_mouse_button_pressed(MOUSE_LEFT_BUTTON):
            touching = True
            touch_start = get_mouse_position()

        if touching and is_mouse_button_released(MOUSE_LEFT_BUTTON):
            touching = False
            touch_end = get_mouse_position()

            dx = touch_end.x - touch_start.x
            dy = touch_end.y - touch_start.y
            dist = math.sqrt(dx * dx + dy * dy)

            if dist >= SWIPE_MIN_DISTANCE:
                # Determine swipe direction
                if abs(dx) > abs(dy):
                    if dx > 0:
                        move_empty(3)  # right
                    else:
                        move_empty(2)  # left
                else:
                    if dy > 0:
                        move_empty(1)  # down
                    else:
                        move_empty(0)  # up

        begin_drawing()
        clear_background(RAYWHITE)
        draw_text(b"Swipe or drag to move empty tile", 10, 10, 18, DARKGRAY)
        draw_board()
        end_drawing()

    close_window()

if __name__ == "__main__":
    main()
