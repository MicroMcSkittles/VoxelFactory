import pygame
import math

# This is a prototype implementation of the voxel traversal algorithm (described in
# the paper "A Fast Voxel Traversal Algorithm for Ray Tracing" 
# by John Amanatides and Andrew Woo) in 2D to figure out how to use it in the
# actual engine

# Global vars
window_width = 800
window_height = 600
running = True

# Grid vars
grid_width = 9
grid_height = 6
grid_start_position = pygame.Vector2(100,100)
grid_padding = pygame.Vector2(100,100)
grid_cell_width = min(
    math.floor((window_width - grid_padding.x * 2) / grid_width),
    math.floor((window_height - grid_padding.y * 2) / grid_height)
)
grid = [
    0,0,0,1,0,1,0,1,0,
    0,1,1,1,0,1,0,0,1,
    0,1,0,1,1,0,0,0,0,
    0,1,0,1,0,1,0,1,1,
    1,0,0,0,1,0,0,1,0,
    0,1,1,1,0,0,0,1,0
]

# Player vars
player_speed = 125
player_position = pygame.Vector2(0,0)
player_direction = pygame.Vector2(0,1)

# Setup
pygame.init()
screen = pygame.display.set_mode((window_width,window_height))
clock = pygame.time.Clock()
delta_time = 0.1

def At(x,y):
    if x < 0 or x >= grid_width: return -1
    if y < 0 or y >= grid_height: return -1
    return grid[int(y * grid_width + x)]
def DrawGrid():
    # Displays grid to the screen
    for y in range(grid_height):
        for x in range(grid_width):
            cell_position = grid_start_position + (x * grid_cell_width, y * grid_cell_width)
            pygame.draw.rect(screen, (255,255,255), (cell_position.x, cell_position.y, grid_cell_width, grid_cell_width), 2)
            if (At(x,y) == 1): 
                pygame.draw.rect(screen, (255,255,255), (cell_position.x, cell_position.y, grid_cell_width, grid_cell_width))
def DrawPlayer():
    screen_position = grid_start_position + player_position
    pygame.draw.circle(screen, (255,0,255), screen_position, 5)
    pygame.draw.line(screen, (255,0,255), screen_position, screen_position + (player_direction * 15), 3)

def PlayerInput():
    global player_position
    global player_direction

    # Keyboard stuff
    key_states = pygame.key.get_pressed()
    move_delta = pygame.Vector2(0,0)
    if key_states[pygame.K_w]: move_delta.y = -1
    if key_states[pygame.K_s]: move_delta.y = 1
    if key_states[pygame.K_a]: move_delta.x = -1
    if key_states[pygame.K_d]: move_delta.x = 1
    if move_delta.x != 0 or move_delta.y != 0:
        move_delta = move_delta.normalize()
        player_position += (move_delta * player_speed * delta_time)
    
    # Mouse stuff
    player_direction = pygame.mouse.get_pos() - (player_position + grid_start_position)
    if player_direction.x != 0 or player_direction.y != 0:  
        player_direction = player_direction.normalize()
def ClampPoint(point):
    point.x = max(0, min(point.x, grid_width * grid_cell_width - 0.01))
    point.y = max(0, min(point.y, grid_height * grid_cell_width - 0.01))
    return point
def RayAABBIntersection(inv_direction):
    aabb_min = pygame.Vector2(0,0)
    aabb_max = pygame.Vector2(grid_width * grid_cell_width, grid_height * grid_cell_width)
    t1 = aabb_min - player_position
    t2 = aabb_max - player_position
    t1.x *= inv_direction.x
    t1.y *= inv_direction.y
    t2.x *= inv_direction.x
    t2.y *= inv_direction.y
    
    min_dist = 0
    max_dist = math.inf

    min_dist = max(min_dist, min(t1.x, t2.x))
    max_dist = min(max_dist, max(t1.x, t2.x))
    min_dist = max(min_dist, min(t1.y, t2.y))
    max_dist = min(max_dist, max(t1.y, t2.y))

    if max_dist >= min_dist:
        return min_dist
    else: return -1
def Traverse():
    point = player_position / grid_cell_width
    inv_direction = pygame.Vector2(0,0)
    if player_direction.x == 0: inv_direction.x = math.inf
    else: inv_direction.x = 1 / player_direction.x
    if player_direction.y == 0: inv_direction.y = math.inf
    else: inv_direction.y = 1 / player_direction.y
    if point.x < 0 or point.x >= grid_width or point.y < 0 or point.y >= grid_height:
        dist = RayAABBIntersection(inv_direction)
        if dist == -1: return
        point = ClampPoint(player_position + (player_direction * dist)) / grid_cell_width
    x = math.floor(point.x)
    y = math.floor(point.y)
    step_x = (player_direction.x > 0) - (player_direction.x < 0)
    step_y = (player_direction.y > 0) - (player_direction.y < 0)

    plane_x = x + 0.5 + 0.5 * step_x
    plane_y = y + 0.5 + 0.5 * step_y

    max_dist_x = (plane_x - point.x) * inv_direction.x
    max_dist_y = (plane_y - point.y) * inv_direction.y

    delta_dist_x = abs(inv_direction.x)
    delta_dist_y = abs(inv_direction.y)

    dist = 0
    hit = False
    normal = pygame.Vector2(0,0)

    while True:
        current_voxel = At(x,y)
        if current_voxel == 1:
            hit = True
            debug_int = (point + (player_direction * dist)) * grid_cell_width + grid_start_position
            pygame.draw.circle(screen, (255,0,0), debug_int, 4)
            break
        if current_voxel == -1:
            break
        
        if max_dist_x < max_dist_y:
            dist = max_dist_x
            normal = pygame.Vector2(-step_x,0)
            x += step_x
            max_dist_x += delta_dist_x
            debug_x_int = (point + (player_direction * dist)) * grid_cell_width + grid_start_position
            pygame.draw.circle(screen, (0,255,0), debug_x_int, 4)
        else:
            dist = max_dist_y
            normal = pygame.Vector2(0,-step_y)
            y += step_y
            max_dist_y += delta_dist_y
            debug_y_int = (point + (player_direction * dist)) * grid_cell_width + grid_start_position
            pygame.draw.circle(screen, (0,0,255), debug_y_int, 4)

    # debug stuff
    pygame.draw.circle(screen, (0,255,255), (point * grid_cell_width) + grid_start_position, 4)
    debug_int = (point + (player_direction * dist)) * grid_cell_width + grid_start_position
    pygame.draw.line(screen, (155,155,155), player_position + grid_start_position, debug_int, 2)
    if hit:
        cell_position = grid_start_position + (x * grid_cell_width, y * grid_cell_width)
        pygame.draw.rect(screen, (255,0,0), (cell_position.x, cell_position.y, grid_cell_width, grid_cell_width), 2)
        debug_normal = debug_int + (normal * 15)
        pygame.draw.line(screen, (0,255,255), debug_int, debug_normal, 2)



# Main loop
while running:

    screen.fill((0,0,0))

    PlayerInput()

    DrawGrid()
    DrawPlayer()

    Traverse()

    # Handle window events
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    pygame.display.flip()
    delta_time = clock.tick(60) / 1000
