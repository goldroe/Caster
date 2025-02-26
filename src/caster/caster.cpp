
global Back_Buffer g_back_buffer;
global Game_State *game_state;

global Font *default_font;

#define SCREEN_WIDTH   640
#define SCREEN_HEIGHT  480

#define TEX_WIDTH 64
#define TEX_HEIGHT 64

global f64 z_buffer[SCREEN_WIDTH];

#define MAP_HEIGHT 24
#define MAP_WIDTH 24
global u8 world_map[MAP_HEIGHT][MAP_WIDTH] = {
  {8,8,8,8,8,8,8,8,8,8,8,4,4,6,4,4,6,4,6,4,4,4,6,4},
  {8,0,0,0,0,0,0,0,0,0,8,4,0,0,0,0,0,0,0,0,0,0,0,4},
  {8,0,3,3,0,0,0,0,0,8,8,4,0,0,0,0,0,0,0,0,0,0,0,6},
  {8,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6},
  {8,0,3,3,0,0,0,0,0,8,8,4,0,0,0,0,0,0,0,0,0,0,0,4},
  {8,0,0,0,0,0,0,0,0,0,8,4,0,0,0,0,0,6,6,6,0,6,4,6},
  {8,8,8,8,0,8,8,8,8,8,8,4,4,4,4,4,4,6,0,0,0,0,0,6},
  {7,7,7,7,0,7,7,7,7,0,8,0,8,0,8,0,8,4,0,4,0,6,0,6},
  {7,7,0,0,0,0,0,0,7,8,0,8,0,8,0,8,8,6,0,0,0,0,0,6},
  {7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,6,0,0,0,0,0,4},
  {7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,6,0,0,0,6,0,6},
  {7,7,0,0,0,0,0,0,7,8,0,8,0,8,0,8,8,6,0,0,0,6,6,6},
  {7,7,7,7,0,7,7,7,7,8,8,4,0,6,8,4,8,3,0,0,0,3,3,3},
  {2,2,2,2,0,2,2,2,2,4,6,4,0,0,6,0,6,3,0,0,0,0,0,3},
  {2,2,0,0,0,0,0,2,2,4,0,0,0,0,0,0,4,3,0,0,0,0,0,3},
  {2,0,0,0,0,0,0,0,2,4,0,0,0,0,0,0,4,3,0,0,0,0,0,3},
  {1,0,0,0,0,0,0,0,1,4,4,4,4,4,6,0,6,3,3,0,0,0,3,3},
  {2,0,0,0,0,0,0,0,2,2,2,1,2,2,2,6,6,0,0,5,0,5,0,5},
  {2,2,0,0,0,0,0,2,2,2,0,0,0,2,2,0,5,0,5,0,0,0,5,5},
  {2,0,0,0,0,0,0,0,2,0,0,0,0,0,2,5,0,5,0,5,0,5,0,5},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5},
  {2,0,0,0,0,0,0,0,2,0,0,0,0,0,2,5,0,5,0,5,0,5,0,5},
  {2,2,0,0,0,0,0,2,2,2,0,0,0,2,2,0,5,0,5,0,0,0,5,5},
  {2,2,2,2,1,2,2,2,2,2,2,1,2,2,2,5,5,5,5,5,5,5,5,5}
};

#define NUM_SPRITES 19
Sprite sprites[NUM_SPRITES] =
{
  {20.5, 11.5, 1,1,0, 10}, //green light in front of playerstart
  //green lights in every room
  {18.5,4.5,  1,1,0, 10},
  {10.0,4.5,  1,1,0, 10},
  {10.0,12.5, 1,1,0, 10},
  {3.5, 6.5,  1,1,0, 10},
  {3.5, 20.5, 1,1,0, 10},
  {3.5, 14.5, 1,1,0, 10},
  {14.5,20.5, 1,1,0, 10},

  //row of pillars in fron1t of wall: fisheye test
  {18.5, 10.5, 1,1,0, 9},
  {18.5, 11.5, 1,1,0, 9},
  {18.5, 12.5, 1,1,0, 9},

  //some barrels around the map
  {18.5, 1.5, 1,1,0, 8},
  {15.5, 1.5, 1,1,0, 8},
  {16.0, 1.8, 1,1,0, 8},
  {16.2, 1.2, 1,1,0, 8},
  {18.5, 5.5, 2,2,128, 8},
  {15.5, 4.5, 2,2,128, 8},
  {19.0, 4.1, 2,2,128, 8},
  {14.5, 4.8, 2,2,128, 8},
};

internal void update_screen_buffer(Back_Buffer *buffer, int width, int height) {
    buffer->bytes_per_pixel = 4;
    buffer->pixels = (u8 *)realloc(buffer->pixels, buffer->bytes_per_pixel * width * height);
    buffer->width = width;
    buffer->height = height;;

    //@Note Update backbuffer texture
    if (buffer->tex) {
        R_D3D11_Tex2D *tex = (R_D3D11_Tex2D *)buffer->tex;
        tex->view->Release();
        tex->texture->Release();
    }
    buffer->tex = d3d11_create_texture(R_Tex2DFormat_R8G8B8A8, {width, height}, buffer->pixels);
}

internal void load_screen_buffer_texture(Back_Buffer *buffer) {
    R_D3D11_Tex2D *tex = (R_D3D11_Tex2D*)buffer->tex;
    ID3D11Texture2D *tex2d = tex->texture;
    r_d3d11_state->device_context->UpdateSubresource(tex2d, 0, NULL, buffer->pixels, buffer->width * buffer->bytes_per_pixel, 0);
}

internal void clear_buffer(Back_Buffer *buffer, f32 r, f32 g, f32 b, f32 a) {
    RGBA color;
    color.r = (u8)(r * 255);
    color.g = (u8)(g * 255);
    color.b = (u8)(b * 255);
    color.a = (u8)(a * 255);
    for (int y = 0; y < buffer->height; y++) {
        u32 *row = (u32 *)buffer->pixels + y * buffer->width;
        for (int x = 0; x < buffer->width; x++) {
            row[x] = color.v;
        }
    }
}

internal void fill_vertical_line(Back_Buffer *buffer, int x, int y0, int y1, u32 color) {
    u32 *pixels = (u32 *)buffer->pixels;
    for (int y = y0; y <= y1; y++) {
        pixels[y * buffer->width + x] = color;
    }
}

internal inline void fill_pixel(Back_Buffer *buffer, int x, int y, u32 color) {
    ((u32 *)buffer->pixels)[y * buffer->width + x] = color;
}

internal void load_texture(const char *file_name) {
    Texture texture = {};
    int n;
    texture.bitmap = (u8 *)stbi_load(file_name, &texture.width, &texture.height, &n, 4);
    texture.tex = d3d11_create_texture(R_Tex2DFormat_R8G8B8A8, {texture.width, texture.height}, texture.bitmap);
    game_state->textures.push(texture);
}

internal void update_and_render(OS_Event_List *events, OS_Handle window_handle, f32 dt) {
    local_persist bool first_call = true;
    if (first_call) {
        first_call = false;

        game_state = new Game_State();
        game_state->pos = v2_f32(22.0f, 1.5f);
        game_state->dir = v2_f32(-1.0f, 0.0f);
        game_state->plane_x = 0;
        game_state->plane_y = 0.66;


        load_texture("data/wolftex/eagle.png");
        load_texture("data/wolftex/redbrick.png");
        load_texture("data/wolftex/purplestone.png");
        load_texture("data/wolftex/greystone.png");
        load_texture("data/wolftex/bluestone.png");
        load_texture("data/wolftex/mossy.png");
        load_texture("data/wolftex/wood.png");
        load_texture("data/wolftex/colorstone.png");
        load_texture("data/wolftex/barrel.png");
        load_texture("data/wolftex/pillar.png");
        load_texture("data/wolftex/greenlight.png");

        update_screen_buffer(&g_back_buffer, SCREEN_WIDTH, SCREEN_HEIGHT);

        Arena *temp = make_arena(get_malloc_allocator());
        default_font = load_font(temp, str8_lit("data/fonts/consolas.ttf"), 20);
    }

    V2_F32 window_dim = os_get_window_dim(window_handle);

    input_begin(window_handle, events);

    //@Note Camera
    {
        f32 mouse_dx = get_mouse_delta().x;
        if (key_down(OS_KEY_COMMA)) {
            mouse_dx = -10.0;
        }
        if (key_down(OS_KEY_PERIOD)) {
            mouse_dx = 10.0;
        }
        f32 rot_speed = -0.125f * mouse_dx * dt;
        f64 plane_x = game_state->plane_x;
        f64 plane_y = game_state->plane_y;
        V2_F32 dir = game_state->dir;

        if (mouse_dx != 0) {
            f32 old_dir_x = dir.x;
            dir.x = old_dir_x * cosf(rot_speed) - dir.y * sinf(rot_speed);
            dir.y = old_dir_x * sinf(rot_speed) + dir.y * cosf(rot_speed);
            f64 old_plane_x = plane_x;
            plane_x = old_plane_x * cosf(rot_speed) - plane_y * sinf(rot_speed);
            plane_y = old_plane_x * sinf(rot_speed) + plane_y * cosf(rot_speed);
        }

        game_state->dir = dir;
        game_state->plane_x = plane_x;
        game_state->plane_y = plane_y;
    }

    //@Note Player movement
    {
        //@Note Get camera vectors
        V2_F32 dir = game_state->dir;
        V3_F32 up = v3_f32(0.0f, 0.0f, 1.0f);
        V3_F32 side = normalize_v3_f32(cross_v3_f32(v3_f32(dir.x, dir.y, 0), up));
        V2_F32 right = v2_f32(side.x, side.y);


        f32 forward_dt = 0.0f;
        f32 right_dt = 0.0f;
        if (key_down(OS_KEY_UP) || key_down(OS_KEY_W)) {
            forward_dt += 1;
        }
        if (key_down(OS_KEY_DOWN) || key_down(OS_KEY_S)) {
            forward_dt -= 1;
        }
        if (key_down(OS_KEY_LEFT) || key_down(OS_KEY_A)) {
            right_dt -= 1; 
        }
        if (key_down(OS_KEY_RIGHT) || key_down(OS_KEY_D)) {
            right_dt += 1; 
        }

        f32 speed = 4.0f;
        V2_F32 direction = normalize_v2_f32(dir * forward_dt + right * right_dt);
        V2_F32 pos = game_state->pos;
        V2_F32 new_pos = pos + speed * direction * dt;

        // Collision
        if (world_map[(int)new_pos.y][(int)new_pos.x] == 0) {
            game_state->pos = new_pos;
        }
    }

    clear_buffer(&g_back_buffer, 1, 0, 1, 1);

    f64 w = (f64)SCREEN_WIDTH;
    f64 h = (f64)SCREEN_HEIGHT;
    f64 plane_x = game_state->plane_x;
    f64 plane_y = game_state->plane_y;
    f64 pos_x = (f64)game_state->pos.x;
    f64 pos_y = (f64)game_state->pos.y;

    V2_F32 dir = game_state->dir;

    //@Note Floor raycasting
    for (int y = (int)h / 2 + 1; y < h; y++) {
        f64 ray_x0 = dir.x - plane_x;
        f64 ray_y0 = dir.y - plane_y;
        f64 ray_x1 = dir.x + plane_x;
        f64 ray_y1 = dir.y + plane_y;

        int p = y - (int)(h / 2);
        f32 pz = (f32)h * 0.5f;
        f32 row_distance = pz / p;

        V2_F32 floor_pos;
        floor_pos.x = (f32)(pos_x + row_distance * ray_x0);
        floor_pos.y = (f32)(pos_y + row_distance * ray_y0);

        V2_F32 floor_step;
        floor_step.x = (f32)(row_distance * (ray_x1 - ray_x0) / w);
        floor_step.y = (f32)(row_distance * (ray_y1 - ray_y0) / w);

        for (int x = 0; x < w; x++) {
            int cell_x = (int)floor_pos.x;
            int cell_y = (int)floor_pos.y;

            int tex_x = (int)(TEX_WIDTH * (floor_pos.x - cell_x)) & (TEX_WIDTH - 1);
            int tex_y = (int)(TEX_HEIGHT * (floor_pos.y - cell_y)) & (TEX_HEIGHT - 1);

            floor_pos += floor_step;

            Texture *floor_texture = &game_state->textures[3];
            Texture *ceiling_texture = &game_state->textures[6];

            RGBA color;
            color.v = ((u32 *)floor_texture->bitmap)[tex_y * TEX_WIDTH + tex_x];
            fill_pixel(&g_back_buffer, x, y, color.v);

            color.v = ((u32 *)ceiling_texture->bitmap)[tex_y * TEX_WIDTH + tex_x];
            fill_pixel(&g_back_buffer, x, (int)h - y - 1, color.v);
        }
    }

    //@Note Wall raycasting
    for (int x = 0; x < w; x++) {
        int map_x = (int)game_state->pos.x;
        int map_y = (int)game_state->pos.y;

        f64 camera_x = 2 * x / w - 1; // -1 to 1 from 0 to w

        f64 raydir_x = (f64)game_state->dir.x + plane_x * camera_x;
        f64 raydir_y = (f64)game_state->dir.y + plane_y * camera_x;

        f64 dx = (raydir_x != 0.0) ? Abs(1.0 / raydir_x) : 10000000.0;
        f64 dy = (raydir_y != 0.0) ? Abs(1.0 / raydir_y) : 10000000.0;

        f64 side_dx;
        f64 side_dy;

        int step_x;
        int step_y;

        if (raydir_x < 0) {
            step_x = -1;
            side_dx = (pos_x - map_x) * dx;
        } else {
            step_x = 1;
            side_dx = (map_x + 1.0 - pos_x) * dx;
        }
        if (raydir_y < 0) {
            step_y = -1; 
            side_dy = (pos_y - map_y) * dy;
        } else {
            step_y = 1;
            side_dy = (map_y + 1.0 - pos_y) * dy;
        }

        int hit = 0;
        int side;

        while (hit == 0) {
            if (side_dx < side_dy) {
                side_dx += dx;
                map_x += step_x;
                side = 0;
            } else {
                side_dy += dy;
                map_y += step_y;
                side = 1;
            }

            if (world_map[map_y][map_x] > 0) {
                hit = 1;
            }
        }

        f64 perp_wall_dist = 0;
        if (side == 0) perp_wall_dist = side_dx - dx;
        if (side == 1) perp_wall_dist = side_dy - dy;

        int line_height = (int)(h / perp_wall_dist);

        int y0 = (int)h / 2 - line_height / 2;
        int y1 = (int)h / 2 + line_height / 2; 
        if (y0 < 0) y0 = 0;
        if (y1 >= h) y1 = (int)h - 1;


        int wall = world_map[map_y][map_x];
        int tex_idx = wall - 1;

        Texture *texture = &game_state->textures[tex_idx];

        f64 wall_x;
        if (side == 0) wall_x = pos_y + perp_wall_dist * raydir_y;
        else wall_x = pos_x + perp_wall_dist * raydir_x;
        wall_x -= floor(wall_x);

        int tex_x = (int)(wall_x * TEX_WIDTH);
        if (side == 0 && raydir_x > 0) tex_x = TEX_WIDTH - tex_x - 1;
        if (side == 1 && raydir_y < 0) tex_x = TEX_WIDTH - tex_x - 1;

        f64 step = 1.0 * TEX_HEIGHT / line_height;
        f64 tex_pos = (y0 - h / 2 + line_height / 2) * step;

        for (int y = y0; y < y1; y++) {
            int tex_y = (int)tex_pos & (TEX_HEIGHT - 1);
            tex_pos += step;

            RGBA color;
            color.v = ((u32 *)texture->bitmap)[tex_y * TEX_WIDTH + tex_x];

            // //@Note Darken side
            if (side == 1) {
                color.r /= 2;
                color.g /= 2;
                color.b /= 2;
            }

            fill_pixel(&g_back_buffer, x, y, color.v);

            z_buffer[x] = perp_wall_dist;
        }
    }

    //@Note Sort sprites
    for (int i = 0; i < NUM_SPRITES; i++) {
        Sprite sprite = sprites[i];
        f64 dist = ((pos_x - sprites[i].x) * (pos_x - sprites[i].x) + (pos_y - sprites[i].y) * (pos_y - sprites[i].y));
        int j = i - 1;
        while (j >= 0) {
            f64 distj = (pos_x - sprites[j].x) * (pos_x - sprites[j].x) + (pos_y - sprites[j].y) * (pos_y - sprites[j].y);
            if (distj > dist) {
                break;
            }

            sprites[j + 1] = sprites[j];
            j = j - 1;
        }

        sprites[j + 1] = sprite;
    }

    //@Note Sprite raycasting
    for (int sprite_idx = 0; sprite_idx < NUM_SPRITES; sprite_idx++) {
        Sprite sprite = sprites[sprite_idx];
        f64 sprite_x = sprite.x - pos_x;
        f64 sprite_y = sprite.y - pos_y;

        //     [a b]
        // A = [   ]
        //     [c d]
        ///                            [d -b]
        // Inverse of A = 1/(ad-bc) *  [    ]
        //                             [-c a]

        //     [plane_x dir_x]
        // A = [             ]
        //     [plane_y dir_y]

        // [dir_y     -dir_x]
        // [                ] * 1/(ad - bc)  = A(-1)
        // [-plane_y plane_x]

        f64 det = 1.0 / (plane_x * dir.y - dir.x * plane_y);

        f64 xform_x = det * (sprite_x * dir.y + sprite_y * -dir.x);
        f64 xform_y = det * (sprite_x * -plane_y + sprite_y * plane_x);

        int sprite_screen_x = (int)(w / 2 * (1 + xform_x / xform_y));

        int sprite_h = abs((int)(h / xform_y)) / sprite.v_scale;
        int adjust_y = (int)(sprite.v_adjust / xform_y);

        int y0 = (int)h / 2 - sprite_h / 2 + adjust_y;
        int y1 = (int)h / 2 + sprite_h / 2 + adjust_y;
        if (y0 < 0) y0 = 0;
        if (y1 >= h) y1 = (int)h - 1;

        int sprite_w = abs((int)(h / xform_y)) / sprite.u_scale;
        int x0 = sprite_screen_x - sprite_w / 2;
        int x1 = sprite_screen_x + sprite_w / 2;
        if (x0 < 0) x0 = 0;
        if (x1 >= w) x1 = (int)w - 1;

        for (int x = x0; x < x1; x++) {
            int tex_x = (int)(256 * (x - (-sprite_w / 2 + sprite_screen_x)) * TEX_WIDTH / sprite_w / 256);
            if (xform_y > 0 && x > 0 && x < w && xform_y < z_buffer[x]) {
                for (int y = y0; y < y1; y++) {
                    int d = (y - adjust_y) * 256 - (int)h * 128 + sprite_h * 128;
                    int tex_y = ((d * TEX_HEIGHT) / sprite_h) / 256;

                    Texture texture = game_state->textures[sprite.tex];
                    u32 color = ((u32 *)texture.bitmap)[tex_y * TEX_WIDTH + tex_x];
                    if ((color & 0x00FFFFFF) != 0) fill_pixel(&g_back_buffer, x, y, color);
                }
            }
        }
    }

    //@Note Draw backbuffer texture
    load_screen_buffer_texture(&g_back_buffer);

    draw_begin(window_handle);
    M4_F32 ortho = ortho_rh_zo(0.0f, window_dim.x, 0.0f, window_dim.y, -1.0f, 1.0f);
    draw_set_xform(ortho);
    draw_quad(g_back_buffer.tex, make_rect(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT), make_rect(0, 0, 1, 1));

    M4_F32 ortho_text = ortho_rh_zo(0.0f, window_dim.x, window_dim.y, 0.0f, -1.0f, 1.0f);
    draw_set_xform(ortho_text);
    draw_textf(default_font, v4_f32(1.0f, 0.0f, 1.0f, 1.0f), V2_Zero, "%f %f", game_state->pos.x, game_state->pos.y);
    
    d3d11_render(window_handle, draw_bucket);

    r_d3d11_state->swap_chain->Present(1, 0);

    draw_end();

    g_input.capture_cursor = true;

    input_end(window_handle);
}
