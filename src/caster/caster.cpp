
global Back_Buffer g_back_buffer;
global Game_State *game_state;

#define MAP_HEIGHT 32
#define MAP_WIDTH 32
global u8 world_map[MAP_HEIGHT][MAP_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,2,2,2,2,2,0,0,0,0,0,0,0,3,3,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,4,4,4,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
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

internal void update_and_render(OS_Event_List *events, OS_Handle window_handle, f32 dt) {
    local_persist bool first_call = true;
    if (first_call) {
        first_call = false;

        game_state = new Game_State();
        game_state->pos = v2_f32(16.0f, 30.0f);
        game_state->dir = v2_f32(-1.0f, 0.0f);
        game_state->plane_x = 0;
        game_state->plane_y = 0.66;
    }

    V2_F32 window_dimension = os_get_window_dim(window_handle);

    if (window_dimension.x != g_back_buffer.width || window_dimension.y != g_back_buffer.height) {
        update_screen_buffer(&g_back_buffer, (int)window_dimension.x, (int)window_dimension.y);
    }

    input_begin(window_handle, events);

    //@Note Camera
    {
        f32 mouse_dx = get_mouse_delta().x;
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
   


    clear_buffer(&g_back_buffer, 0, 0, 0, 1);

    f64 w = (f64)window_dimension.x;
    f64 h = (f64)window_dimension.y;
    f64 plane_x = game_state->plane_x;
    f64 plane_y = game_state->plane_y;
    f64 pos_x = (f64)game_state->pos.x;
    f64 pos_y = (f64)game_state->pos.y;

    //@Note Raycasting
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

        RGBA color = {};
        int wall = world_map[map_y][map_x];
        switch (wall) {
        case 1:
            color.v = 0xff0000ff;
            break;
        case 2:
            color.v = 0xff00ff00;
            break;
        case 3:
            color.v = 0xffff0000;
            break;
        case 4:
            color.v = 0xffffffff;
            break;
        }

        //@Note Darken side
        if (side == 1) {
            color.r /= 2;
            color.g /= 2;
            color.b /= 2;
        }

        fill_vertical_line(&g_back_buffer, x, y0, y1, color.v);
    }


    //@Note Draw backbuffer texture
    load_screen_buffer_texture(&g_back_buffer);

    draw_begin(window_handle);
    M4_F32 ortho = ortho_rh_zo(0.0f, window_dimension.x, 0.0f, window_dimension.y, -1.0f, 1.0f);
    draw_set_xform(ortho);
    draw_quad(g_back_buffer.tex, make_rect(0.0f, 0.0f, window_dimension.x, window_dimension.y), make_rect(0, 0, 1, 1));
    d3d11_render(window_handle, draw_bucket);


    r_d3d11_state->swap_chain->Present(1, 0);

    draw_end();

    g_input.capture_cursor = true;

    input_end(window_handle);
}
