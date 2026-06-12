import pygame
import math

# AHHHHHHHHHHHHHHHHHHHHHH

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
    0,0,0,1,0,1,0,1,1,
    1,0,0,0,0,0,0,1,0,
    0,1,1,1,0,0,0,1,0
]

# Player vars
player_speed = 125
player_position = pygame.Vector2(0,0)
player_direction = pygame.Vector2(0,1)
#player_size = pygame.Vector2(grid_cell_width * 0.5, grid_cell_width * 0.5)
player_size = pygame.Vector2(grid_cell_width * 0.8, grid_cell_width * 1.8)
player_check_radius = 1
player_move_delta = pygame.Vector2(0,0)

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
    pygame.draw.rect(screen, (255,255,0), (screen_position - player_size * 0.5, player_size), 2)

def PlayerInput():
    global player_position
    global player_direction
    global player_move_delta

    # Keyboard stuff
    key_states = pygame.key.get_pressed()
    if key_states[pygame.K_w]: player_move_delta.y = -1
    if key_states[pygame.K_s]: player_move_delta.y = 1
    if key_states[pygame.K_a]: player_move_delta.x = -1
    if key_states[pygame.K_d]: player_move_delta.x = 1
    if player_move_delta.x != 0 or player_move_delta.y != 0:
        player_move_delta = player_move_delta.normalize() * player_speed * delta_time
    
    # Mouse stuff
    player_direction = pygame.mouse.get_pos() - (player_position + grid_start_position)
    if player_direction.x != 0 or player_direction.y != 0:  
        player_direction = player_direction.normalize()

def PointQuadCollision(point, quad_position, quad_size):
    if point.x < quad_position.x - quad_size.x * 0.5: return False
    if point.y < quad_position.y - quad_size.y * 0.5: return False
    if point.x > quad_position.x + quad_size.x * 0.5: return False
    if point.y > quad_position.y + quad_size.y * 0.5: return False
    return True
def QuadQuadCollision(quad1, quad2):
    if quad1[0].x - quad1[1].x * 0.5 >= quad2[0].x + quad2[1].x * 0.5: return False
    if quad1[0].x + quad1[1].x * 0.5 <= quad2[0].x - quad2[1].x * 0.5: return False
    if quad1[0].y - quad1[1].y * 0.5 >= quad2[0].y + quad2[1].y * 0.5: return False
    if quad1[0].y + quad1[1].y * 0.5 <= quad2[0].y - quad2[1].y * 0.5: return False
    return True
def LineQuadIntersection(ray, quad):

    ray_origin, ray_direction = ray
    if ray_direction.x == 0 and ray_direction.y == 0: return False, 0, pygame.Vector2(0,0)

    quad_position, quad_size = quad
    quad_min = quad_position - quad_size * 0.5
    quad_max = quad_position + quad_size * 0.5

    near_dist = 0
    far_dist = 0
    normal = pygame.Vector2(0,0)

    # Perfectly vertical
    if ray_direction.x == 0:
        near_dist = (quad_min.y - ray_origin.y) / ray_direction.y
        far_dist = (quad_max.y - ray_origin.y) / ray_direction.y

        if far_dist < near_dist: far_dist, near_dist = near_dist, far_dist

        if ray_origin.x < quad_min.x or ray_origin.x > quad_max.x: return False, 0, normal

        if ray_direction.y < 0: normal.y = 1
        else: normal.y = -1
    # Perfectly horizontal
    elif ray_direction.y == 0:
        near_dist = (quad_min.x - ray_origin.x) / ray_direction.x
        far_dist = (quad_max.x - ray_origin.x) / ray_direction.x

        if far_dist < near_dist: far_dist, near_dist = near_dist, far_dist

        if ray_origin.y < quad_min.y or ray_origin.y > quad_max.y: return False, 0, normal

        if ray_direction.x < 0: normal.x = 1
        else: normal.x = -1
    # Normal case
    else:
        near_dist_x = (quad_min.x - ray_origin.x) / ray_direction.x
        near_dist_y = (quad_min.y - ray_origin.y) / ray_direction.y
        far_dist_x = (quad_max.x - ray_origin.x) / ray_direction.x
        far_dist_y = (quad_max.y - ray_origin.y) / ray_direction.y

        if far_dist_x < near_dist_x: far_dist_x, near_dist_x = near_dist_x, far_dist_x
        if far_dist_y < near_dist_y: far_dist_y, near_dist_y = near_dist_y, far_dist_y

        if near_dist_x > far_dist_y or near_dist_y > far_dist_x: return False, 0, normal
        
        near_dist = max(near_dist_x, near_dist_y)
        far_dist = min(far_dist_x, far_dist_y)

        if near_dist_x > near_dist_y: 
            if ray[1].x < 0: normal.x = 1
            else: normal.x = -1
        else:
            if ray[1].y < 0: normal.y = 1
            else: normal.y = -1

    if far_dist < 0: return False, 0, pygame.Vector2(0,0)
    if near_dist > 1: return False, 0, pygame.Vector2(0,0)

    return True, near_dist, normal
def DynamicQuadQuadIntersection(quad, velocity, target):

    target_position = target[0]
    target_size = target[1] + quad[1]

    pygame.draw.rect(screen, (255,0,255), (target_position - target_size * 0.5 + grid_start_position, target_size), 2)

    return LineQuadIntersection((quad[0], velocity), (target_position, target_size))

def Col1():
    target_position = pygame.Vector2(window_width * 0.5, window_height * 0.5)
    target_size = pygame.Vector2(150,150)
    target_color = (255,255,255)

    quad_position = pygame.Vector2(100,100)
    quad_size = pygame.Vector2(50,50)

    mouse_position = pygame.Vector2(pygame.mouse.get_pos())

    preview_color = (255,255,255)
    quad_velocity = mouse_position - quad_position
    hit, dist, normal = DynamicQuadQuadIntersection((quad_position, quad_size), quad_velocity, (target_position, target_size))
    point = mouse_position
    if hit:
        target_color = (255,255,0)
        preview_color = (255,0,0)
        point = quad_position + (quad_velocity * dist)
        normal_point = point + (normal * 10)
        pygame.draw.line(screen, (0,255,255), point, normal_point, 2)

    pygame.draw.line(screen, target_color, quad_position, mouse_position, 2)
    pygame.draw.rect(screen, preview_color, (point - quad_size * 0.5, quad_size), 3)
    pygame.draw.rect(screen, target_color, (quad_position - quad_size * 0.5, quad_size), 3)
    pygame.draw.rect(screen, target_color, (target_position - target_size * 0.5, target_size), 3)

quad_position = pygame.Vector2(100,100)
quad_velocity = pygame.Vector2(0,0)
def Col2():
    global quad_position
    global quad_velocity

    target_position = pygame.Vector2(window_width * 0.5, window_height * 0.5)
    target_size = pygame.Vector2(150,150)
    target_color = (255,255,255)

    quad_size = pygame.Vector2(50,50)

    mouse_position = pygame.Vector2(pygame.mouse.get_pos())

    if pygame.mouse.get_pressed()[0]:
        quad_velocity += (mouse_position - quad_position) * delta_time

    hit, dist, normal = DynamicQuadQuadIntersection((quad_position, quad_size), quad_velocity * delta_time, (target_position, target_size))
    if hit:
        quad_velocity.x += normal.x * abs(quad_velocity.x)
        quad_velocity.y += normal.y * abs(quad_velocity.y)
        target_color = (255,255,0)
    
    quad_position += quad_velocity * delta_time

    pygame.draw.line(screen, target_color, quad_position, quad_position + (quad_velocity), 2)
    pygame.draw.rect(screen, target_color, (quad_position - quad_size * 0.5, quad_size), 3)
    pygame.draw.rect(screen, target_color, (target_position - target_size * 0.5, target_size), 3)

def Col3():
    global quad_position
    target_position = pygame.Vector2(window_width * 0.5, window_height * 0.5)
    target_size = pygame.Vector2(150,150)
    target_color = (255,255,255)

    quad_size = pygame.Vector2(50,50)

    mouse_position = pygame.Vector2(pygame.mouse.get_pos())

    if pygame.mouse.get_pressed()[0]:
        quad_position = mouse_position

    preview_color = (255,255,255)
    quad_velocity = mouse_position - quad_position
    hit, dist, normal = DynamicQuadQuadIntersection((quad_position, quad_size), quad_velocity, (target_position, target_size))
    point = mouse_position
    if hit:
        target_color = (255,255,0)
        preview_color = (255,0,0)
        point = quad_position + (quad_velocity * dist)
        normal_point = point + (normal * 10)
        pygame.draw.line(screen, (0,255,255), point, normal_point, 2)
        full_dist = math.sqrt((quad_position.x - mouse_position.x)**2 + (quad_position.y - mouse_position.y)**2)
        hit_dist = full_dist * dist
        print(f"full dist: {full_dist}, hit dist: {hit_dist}")

    pygame.draw.line(screen, target_color, quad_position, mouse_position, 2)
    pygame.draw.rect(screen, preview_color, (point - quad_size * 0.5, quad_size), 3)
    pygame.draw.rect(screen, target_color, (quad_position - quad_size * 0.5, quad_size), 3)
    pygame.draw.rect(screen, target_color, (target_position - target_size * 0.5, target_size), 3)
    return

def PlayerCollision():
    global player_position
    global player_move_delta
    # Find min and max of the search area
    player_grid_position = pygame.Vector2(math.floor(player_position.x / grid_cell_width), math.floor(player_position.y / grid_cell_width))
    search_area_min = player_grid_position - pygame.Vector2(player_check_radius, player_check_radius)
    search_area_max = player_grid_position + pygame.Vector2(player_check_radius + 1, player_check_radius + 1)
    search_area_min.x = max(0, search_area_min.x)
    search_area_min.y = max(0, search_area_min.y)
    search_area_max.x = min(grid_width, search_area_max.x)
    search_area_max.y = min(grid_height, search_area_max.y)

    # Find cells the player is potentially colliding with
    potential_collisions = []
    cell_size = pygame.Vector2(grid_cell_width, grid_cell_width)
    for y in range(int(search_area_min.y), int(search_area_max.y)):
        for x in range(int(search_area_min.x), int(search_area_max.x)):
            screen_pos = grid_start_position + pygame.Vector2(x,y) * grid_cell_width
            if At(x,y):
                potential_collisions.append(pygame.Vector2(x,y))
            pygame.draw.rect(screen, (255,0,255), (screen_pos, cell_size), 1)

    velocity = player_move_delta

    # check if potential collisions are collisions
    collisions = []
    for cell in potential_collisions:
        cell_pos = cell * grid_cell_width
        pygame.draw.rect(screen, (255,0,0), (grid_start_position + cell_pos, cell_size), 2)
        hit, dist, normal = DynamicQuadQuadIntersection((player_position, player_size), velocity, (cell_pos + cell_size * 0.5, cell_size))
        if hit: collisions.append((dist, cell))

    # Sort collisions by distance to player than resolve them
    collision_normal = pygame.Vector2(0,0)
    collisions.sort(key=lambda x: x[0])
    for collision in collisions:
        cell = collision[1]
        cell_pos = cell * grid_cell_width
        hit, dist, normal = DynamicQuadQuadIntersection((player_position, player_size), velocity, (cell_pos + cell_size * 0.5, cell_size))
        if hit:
            pygame.draw.rect(screen, (255,0,0), (grid_start_position + cell_pos, cell_size))

            # resolve collision
            velocity_correction = pygame.Vector2(0,0)
            velocity_correction.x = normal.x * (abs(velocity.x) * (1 - dist) + 0.001)
            velocity_correction.y = normal.y * (abs(velocity.y) * (1 - dist) + 0.001)
            velocity += velocity_correction

            collision_normal += normal

    player_move_delta = velocity
    
    # Show collision normal
    if collision_normal.x != 0 or collision_normal.y != 0:
        collision_normal = collision_normal.normalize()
        point = player_position + velocity + grid_start_position
        normal_point = point + collision_normal * 10
        pygame.draw.line(screen, (0,255,255), point, normal_point, 2)
    return

# Main loop
while running:

    screen.fill((0,0,0))

    player_move_delta = pygame.Vector2(0,0)

    PlayerInput()

    DrawGrid()
    DrawPlayer()

    PlayerCollision()
    #Col1()
    #Col2()
    #Col3()

    player_position += (player_move_delta)

    # Handle window events
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    pygame.display.flip()
    delta_time = clock.tick(60) / 1000
