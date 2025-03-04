#include <float.h>

using namespace simdjson;

global Arena *permanent_arena;
global Arena *temporary_arena;

global Back_Buffer g_back_buffer;
global Game_State *game_state;

global Font *default_font;

#define SCREEN_WIDTH   640
#define SCREEN_HEIGHT  480

#define TEX_WIDTH 64
#define TEX_HEIGHT 64

#define MAP_HEIGHT 24
#define MAP_WIDTH 24
global u8 world_map[MAP_HEIGHT][MAP_WIDTH] = {
  {8,8,8,8,8,8,8,8,8,8,8,4,4,6,4,4,6,4,6,4,4,4,6,4},
  {8,0,0,0,0,0,0,0,0,0,8,4,0,0,0,0,0,0,0,0,0,0,0,4},
  {8,0,3,3,0,0,0,0,0,8,8,4,0,0,0,0,0,0,0,0,0,0,0,6},
  {8,0,0,3,0,0,0,0,0,0,0,14,0,0,0,0,0,0,0,0,0,0,0,6},
  {8,0,3,3,0,0,0,0,0,8,8,4,0,0,0,0,0,0,0,0,0,0,0,4},
  {8,0,0,0,0,0,0,0,0,0,8,4,0,0,0,0,0,6,6,6,14,6,4,6},
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
  {1,0,0,0,0,0,0,0,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5},
  {2,0,0,0,0,0,0,0,2,0,0,0,0,0,2,5,0,5,0,5,0,5,0,5},
  {2,2,0,0,0,0,0,2,2,2,0,0,0,2,2,0,5,0,5,0,0,0,5,5},
  {2,2,2,2,1,2,2,2,2,2,2,1,2,2,2,5,5,5,5,5,5,5,5,5}
};

global Door door_map[MAP_HEIGHT][MAP_WIDTH];

global f64 z_buffer[SCREEN_WIDTH];

global Assets *g_assets;

internal inline f32 ease_in_circ(f32 x) {
    return 1.0f - sqrtf(1.0f - powf(x, 2));
}

internal inline f32 ease_out_sine(f32 x) {
    return sinf((x * PI) / 2);
}

internal void entity_free(Entity *e) {
    if (e) free(e);
}

internal Entity *entity_alloc() {
    Entity *node = (Entity *)calloc(sizeof(Entity), 1);
    return node;
}

internal void entity_destroy(Entity *e) {
    e->to_be_destroyed = true;
}

internal Entity *make_entity(int kind, f64 x, f64 y, int tex, int u, int v, f32 off) {
    Entity *e = entity_alloc();
    e->kind = kind;
    e->x = x;
    e->y = y;
    e->tex = tex;
    e->u_scale = u;
    e->v_scale = v;
    e->v_adjust = off;
    game_state->entities.push(e);
    return e;
}

global int a_star_adj[16] = {
    -1,0,  1,0,  0,1,  0,-1,  -1,1,  1,1,  -1,-1,  1,-1 
};

internal bool a_star_search(A_Star_Cell *start, A_Star_Cell *goal) {
    A_Star *a_star = &game_state->a_star;

    for (int y = 0; y < a_star->dim_y; y++) {
        for (int x = 0; x < a_star->dim_x; x++) {
            A_Star_Cell *cell = &a_star->cells[y * a_star->dim_x + x];
            cell->parent = NULL;
            cell->next = NULL;
            cell->f = INT_MAX;
            cell->g = INT_MAX;
            cell->h = 0;
        }
    }

    bool success = false;
    Auto_Array<A_Star_Cell*> open;

    open.push(start);
    start->f = 0;
    start->g = 0;

    while (!open.is_empty()) {
        int min_idx = 0;
        int min = INT_MAX;
        for (int i = 0; i < open.count; i++) {
            A_Star_Cell *o = open[i];
            if (o->f < min) {
                min_idx = i;
                min = o->f;
            }
        }
        A_Star_Cell *cell = open[min_idx];

        if (cell == goal) {
            success = true;
            break;
        }

        open.remove(min_idx);

        for (int i = 0; i < 8; i++) {
            int x = a_star_adj[i * 2], y = a_star_adj[i * 2 + 1];
            int cx = cell->x + x;
            int cy = cell->y + y;

            if (cx < 0 || cy < 0 || cx >= a_star->dim_x || cy >= a_star->dim_y) continue;

            A_Star_Cell *neighbor = &a_star->cells[cy * a_star->dim_x + cx];

            int dist = Abs(x) + Abs(y);
            int g = cell->g + dist;
            int f = Abs(cx - goal->x) + Abs(cy - goal->y);
            int h = dist;

            int blocked = false;
            int wall = world_map[cy][cx];
            if (wall != 0) {
                blocked = true;
                if (wall == 14) {
                    Door door = door_map[cy][cx];
                    if (door.state == DOOR_OPEN) {
                        blocked = false;
                    }
                }
            }

            if (!blocked && f < neighbor->f) {
                neighbor->parent = cell;
                neighbor->f = f;
                neighbor->g = g;
                neighbor->h = h;

                bool in = false;
                for (int j = 0; j < open.count; j++) {
                    if (open[j] == neighbor) in = true;
                }
                if (!in) {
                    open.push(neighbor);
                }
            }
        }
    }

    open.clear();
    return success;
}

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

internal u64 djb2_hash(const char *str) {
    u64 hash = 5381;
    int c;
    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}

internal u64 djb2_hash(String8 str) {
    u64 hash = 5381;
    for (int i = 0; i < str.count; i++) {
        int c = str.data[i];
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

internal Asset *asset_alloc() {
    Asset *asset = push_array(permanent_arena, Asset, 1);
    return asset; 
}

internal void anim_push(Asset *asset) {
    int index = asset->hash % g_assets->anim_table_size;
    Asset_Bucket *hash_bucket = g_assets->anim_table + index;
    DLLPushFront(hash_bucket->first, hash_bucket->last, asset, next, prev);
}

internal void texture_push(Asset *asset) {
    int index = asset->hash % g_assets->texture_table_size;
    Asset_Bucket *hash_bucket = g_assets->texture_table + index;
    DLLPushFront(hash_bucket->first, hash_bucket->last, asset, next, prev);
}

internal void asset_load_texture(const char *file_name) {
    Texture texture = {};
    int n;
    texture.bitmap = (u8 *)stbi_load(file_name, &texture.width, &texture.height, &n, 4);
    game_state->textures.push(texture);

    Asset *asset = asset_alloc();
    asset->hash = djb2_hash(file_name);
    asset->kind = ASSET_TEXTURE;
    asset->texture = texture;
    texture_push(asset);
}

internal Asset *asset_load_animation(String8 name) {
    Asset *asset = asset_alloc();
    asset->kind = ASSET_ANIMATION;
    String8 json_file = str8_pushf(permanent_arena, "%S.json", name);

    ondemand::parser parser;
    simdjson::padded_string json = simdjson::padded_string::load((const char *)json_file.data);
    ondemand::document document = parser.iterate(json);
    ondemand::object root_object = document.get_object();

    auto frames = root_object.find_field("frames").get_object();

    ondemand::object meta_object = root_object.find_field("meta");
    std::string_view image_name = meta_object.find_field("image").get_string();
    char *file_name = push_array(temporary_arena, char, image_name.length() + 1);
    strncpy_s(file_name, image_name.length() + 1, image_name.data(), image_name.length());
    file_name[image_name.length()] = 0;

    int x, y, n;
    u8 *bitmap = (u8 *)stbi_load(file_name, &x, &y, &n, 4);

    size_t frame_count = frames.count_fields();
    asset->anim.textures.reserve(frame_count);

    for (ondemand::field frame_field : frames) {
        ondemand::object frame = frame_field.value();
        auto frame_info = frame.find_field("frame");
        u64 x0 = u64(frame_info.find_field("x"));
        u64 y0 = u64(frame_info.find_field("y"));
        u64 w = u64(frame_info.find_field("w"));
        u64 h = u64(frame_info.find_field("h"));
        u64 y1 = y0 + h;

        int bpp = 4;
        Texture texture = {};
        texture.width = (int)w;
        texture.height = (int)w;

        //@Note Fill spritesheet texture
        texture.bitmap = (u8 *)calloc(texture.width * texture.height, bpp);
        for (int y = 0; y < h; y++) {
            MemoryCopy(texture.bitmap + y * texture.width * bpp, bitmap + (x0 + (y + y0) * x) * bpp, texture.width * bpp);
        }
        asset->anim.textures.push(texture);
    }

    asset->hash = djb2_hash(name);
    asset->name = file_name;
    anim_push(asset);
    return asset;
}

internal Asset *anim_load(const char *name) {
    Asset *result = NULL;
    u64 hash = djb2_hash(name);
    int index = hash % g_assets->anim_table_size;
    Asset_Bucket *hash_bucket = g_assets->anim_table + index;
    for (Asset *asset = hash_bucket->first; asset; asset = asset->next) {
        if (asset->hash == hash) {
            result = asset;
            break;
        }
    }
    return result;
}

internal Asset *texture_load(const char *name) {
    Asset *result = NULL;
    u64 hash = djb2_hash(name);
    int index = hash % g_assets->texture_table_size;
    Asset_Bucket *hash_bucket = g_assets->texture_table + index;
    for (Asset *asset = hash_bucket->first; asset; asset = asset->next) {
        if (asset->hash == hash) {
            result = asset;
            break;
        }
    }
    return result;
}


internal bool raycast(V2_F32 pos, V2_F32 dir, Raycast_Result *res) {
    int map_x = (int)pos.x;
    int map_y = (int)pos.y;

    f32 dx = (dir.x != 0.0f) ? Abs(1.0f / dir.x) : 1000000.0f;
    f32 dy = (dir.y != 0.0f) ? Abs(1.0f / dir.y) : 1000000.0f;

    f32 side_dx, side_dy;
    int step_x, step_y;

    if (dir.x < 0) {
        step_x = -1;
        side_dx = (pos.x - map_x) * dx;
    } else {
        step_x = 1;
        side_dx = (map_x + 1.0f - pos.x) * dx;
    }
    if (dir.y < 0) {
        step_y = -1;
        side_dy = (pos.y - map_y) * dx;
    } else {
        step_y = 1;
        side_dy = (map_y + 1.0f - pos.y) * dy;
    }

    bool hit = false;
    int side = 0;
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
            hit = true;
        }
    }

    f32 dist = 0;
    if (side == 0) dist = side_dx - dx;
    if (side == 1) dist = side_dy - dy;

    if (res) {
        res->dist = dist;
        res->side = side;
        res->dest_x = map_x;
        res->dest_y = map_y;
    }

    return hit;
}

internal void draw_map_cell(int x0, int y0, int cell_w, int cell_h, int tex) {
    Texture texture = game_state->textures[tex];
    int y1 = y0 + cell_h;
    int x1 = x0 + cell_w;

    f32 step_x = TEX_WIDTH  / (f32)cell_w;
    f32 step_y = TEX_HEIGHT / (f32)cell_h;
    f32 tex_y = 0;
    f32 tex_x = 0;
    for (int y = y0; y < y1; y++) {
        tex_x = 0;
        for (int x = x0; x < x1; x++) {
            u32 color = ((u32 *)texture.bitmap)[(int)tex_y * TEX_WIDTH + (int)tex_x];
            fill_pixel(&g_back_buffer, x, y, color);
            tex_x += step_x;
        }
        tex_y += step_y;
    }
}

internal void draw_map(V2_F32 map_dim) {
    int cell_w = (int)(map_dim.x / MAP_WIDTH);
    int cell_h = (int)(map_dim.y / MAP_HEIGHT);

    if (cell_w > cell_h) {
        cell_w = cell_h;
    } else if (cell_h > cell_w) {
        cell_h = cell_w;
    }

    if (cell_h < cell_w) {
        cell_w = cell_h;
    } else if (cell_w < cell_h) {
        cell_h = cell_w;
    }

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            int wall = world_map[y][x];
            int tex = wall - 1;
            if (wall) {
                draw_map_cell(x * cell_w, y * cell_h, cell_w, cell_h, tex);
            }
        }
    }

    for (int i = 0; i < game_state->entities.count; i++) {
        Entity *e = game_state->entities[i];

        f32 w = 0.75f * (f32)(cell_w / e->u_scale);
        f32 h = 0.75f * (f32)(cell_h / e->v_scale);

        int x0 = (int)((e->x - 0.5f) * cell_w);
        int x1 = (int)(x0 + w);
        int y0 = (int)((e->y - 0.5f) * cell_h);
        int y1 = (int)(y0 + h);
        u32 color = 0xFFFFFFFF;

        for (int y = y0; y < y1; y++) {
            for (int x = x0; x < x1; x++) {
                fill_pixel(&g_back_buffer, x, y, color);
            }
        }
    }
}

internal void set_anim(Entity *e, Asset *asset) {
    Assert(asset->kind == ASSET_ANIMATION);
    e->anim = &asset->anim;
    e->anim_t = 0;
    e->anim_frame = 0;
}

internal void update_mob(Entity *e) {
    int wall_x = (int)floor(e->x);
    int wall_y = (int)floor(e->y);
    A_Star_Cell *start = &game_state->a_star.cells[wall_y * game_state->a_star.dim_x + wall_x];

    int player_x = (int)floorf(game_state->pos.x);
    int player_y = (int)floorf(game_state->pos.y);
    A_Star_Cell *goal = &game_state->a_star.cells[player_y * game_state->a_star.dim_x + player_x];

    f64 new_x = e->x;
    f64 new_y = e->y;

    if (a_star_search(start, goal)) {
        for (A_Star_Cell *cell = goal; cell; cell = cell->parent) {
            A_Star_Cell *parent = cell->parent;
            if (parent) {
                parent->next = cell; 
            }
        }

        A_Star_Cell *next = start->next;
        if (next) {
            f32 speed = 2.0f;
            V2_F32 dir = normalize(v2_f32((f32)(next->x + 0.5f - e->x), (f32)(next->y + 0.5f - e->y)));
            new_x = e->x + dir.x * speed * game_state->delta_t;
            new_y = e->y + dir.y * speed * game_state->delta_t;
        }
    }

    if (e->state == STATE_DEFAULT) {
        e->state = STATE_IDLE;
        set_anim(e, anim_load("ghost_idle"));
    } else if (e->state == STATE_IDLE) {
        if (e->x != e->new_x || e->y != e->new_y) {
            e->state = STATE_RUN;
            // set_anim(e, anim_load("ghost_run"));
        }
    } else if (e->state == STATE_RUN) {
    } else if (e->state == STATE_DEAD) {
        entity_destroy(e);
    }

    if (e->anim) {
        e->anim_t += game_state->delta_t;
        e->anim_frame = (int)(e->anim_t * (f32)e->anim->textures.count);
        e->anim_frame %= e->anim->textures.count;
        if (e->anim_t >= 1.0f) {
            e->anim_t = 0;
            e->anim_frame = 0;
        }
    }

    e->x = new_x;
    e->y = new_y;
}

internal void update_game_world(Game_State *state) {
    input_set_mouse_capture();

    if (key_pressed(OS_KEY_ESCAPE)) {
        state->mode = GAME_MODE_PAUSE;
    }

    if (key_pressed(OS_KEY_M)) {
        game_state->draw_map = !game_state->draw_map;
    }

    for (int i = 0; i < state->entities.count; i++) {
        Entity *e = state->entities[i];
        if (e->to_be_destroyed) {
            entity_free(e);
            state->entities.remove(i);
            i -= 1;
        }
    }

    //@Note Camera
    {
        f32 mouse_dx = get_mouse_delta().x;
        if (key_down(OS_KEY_COMMA)) {
            mouse_dx = -10.0;
        }
        if (key_down(OS_KEY_PERIOD)) {
            mouse_dx = 10.0;
        }
        f32 rot_speed = -0.125f * mouse_dx * state->delta_t;
        f64 plane_x = state->plane_x;
        f64 plane_y = state->plane_y;
        V2_F32 dir = state->dir;

        if (mouse_dx != 0) {
            f32 old_dir_x = dir.x;
            dir.x = old_dir_x * cosf(rot_speed) - dir.y * sinf(rot_speed);
            dir.y = old_dir_x * sinf(rot_speed) + dir.y * cosf(rot_speed);
            f64 old_plane_x = plane_x;
            plane_x = old_plane_x * cosf(rot_speed) - plane_y * sinf(rot_speed);
            plane_y = old_plane_x * sinf(rot_speed) + plane_y * cosf(rot_speed);
        }

        state->dir = dir;
        state->plane_x = plane_x;
        state->plane_y = plane_y;
    }

    //@Note Player movement
    {
        //@Note Get camera vectors
        V2_F32 dir = state->dir;
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

        f32 speed = 8.0f;
        V2_F32 direction = normalize_v2_f32(dir * forward_dt + right * right_dt);
        V2_F32 pos = state->pos;
        V2_F32 new_pos = pos + speed * direction * state->delta_t;

        // Collision
        int map_x = (int)new_pos.x;
        int map_y = (int)new_pos.y;
        int wall = world_map[map_y][map_x];

        bool collides = false;
        if (wall != 0) {
            Door door = door_map[map_y][map_x];
            collides = true;
            if (door.state == DOOR_OPEN) {
                collides = false;
            }
        }

        if (collides) {
            //Raycast_Result ray;
            //raycast(pos, dir, &ray);
        } else {
            state->pos = new_pos;
        }
    }

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            Door *door = &door_map[y][x];
            if (door->delta_t >= 1.0f) {
                door->state = DOOR_OPEN;
            }

            if (door->state == DOOR_OPENING) {
                door->delta_t += state->delta_t;
            }
        }
    }

    //@Note Update entities
    for (int i = 0; i < state->entities.count; i++) {
        Entity *e = state->entities[i];
        switch (e->kind) {
        case E_MOB:
            update_mob(e);
            break;
        case E_BALL:
        {
            f32 speed = 10.0f;
            e->x += e->dir_x * speed * state->delta_t;
            e->y += e->dir_y * speed * state->delta_t;
            for (int i = 0; i < state->entities.count; i++) {
                Entity *hit = state->entities[i];
                if (hit == e) continue;
                if (hit->is_collidable() && 
                    e->x - 0.5 < hit->x + 0.5 && e->x + 0.5 > hit->x - 0.5 && 
                    e->y - 0.5 < hit->y + 0.5 && e->y + 0.5 > hit->y - 0.5) {
                    entity_destroy(e);
                }
            }
            break;
        }
        }
    }

    if (key_pressed(OS_KEY_LEFTMOUSE)) {
        Entity *ball = make_entity(E_BALL, state->pos.x, state->pos.y, 12, 1, 1, 0);
        ball->dir_x = state->dir.x;
        ball->dir_y = state->dir.y;
        ball->x += ball->dir_x * 0.5f;
        ball->y += ball->dir_y * 0.5f;
    }

    //@Note Open door
    if (key_pressed(OS_KEY_E)) {
        Raycast_Result ray;
        if (raycast(state->pos, state->dir, &ray)) {
            int wall = world_map[ray.dest_y][ray.dest_x];
            // door num
            if (wall == 14 && ray.dist < 1.0f) {
                Door *door = &door_map[ray.dest_y][ray.dest_x];
                if (door->state == DOOR_CLOSE) {
                    door->state = DOOR_OPENING;
                } else if (door->state == DOOR_OPEN) {
                    door->state = DOOR_CLOSE;
                    door->delta_t = 0.0f;
                }
            }
        }
    }

    f64 w = (f64)SCREEN_WIDTH;
    f64 h = (f64)SCREEN_HEIGHT;
    f64 plane_x = state->plane_x;
    f64 plane_y = state->plane_y;
    f64 pos_x = (f64)state->pos.x;
    f64 pos_y = (f64)state->pos.y;

    V2_F32 dir = state->dir;

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

            Texture *floor_texture = &state->textures[3];
            Texture *ceiling_texture = &state->textures[6];

            RGBA color;
            color.v = ((u32 *)floor_texture->bitmap)[tex_y * TEX_WIDTH + tex_x];
            fill_pixel(&g_back_buffer, x, y, color.v);

            color.v = ((u32 *)ceiling_texture->bitmap)[tex_y * TEX_WIDTH + tex_x];
            fill_pixel(&g_back_buffer, x, (int)h - y - 1, color.v);
        }
    }

    //@Note Wall raycasting
    for (int x = 0; x < w; x++) {
        int map_x = (int)state->pos.x;
        int map_y = (int)state->pos.y;

        f64 camera_x = 2 * x / w - 1; // -1 to 1 from 0 to w

        f64 raydir_x = (f64)state->dir.x + plane_x * camera_x;
        f64 raydir_y = (f64)state->dir.y + plane_y * camera_x;

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

        int wall = 0;

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

            wall = world_map[map_y][map_x];
            if (wall > 0) {
                hit = 1;

                //@Note Check open doors, don't render part of door that is being opened
                if (wall == 14) {
                    Door door = door_map[map_y][map_x];
                    if (door.state == DOOR_OPEN) {
                        hit = 0;
                    } else if (door.state == DOOR_OPENING) {
                        f64 perp_wall_dist = 0;
                        if (side == 0) perp_wall_dist = side_dx - dx;
                        if (side == 1) perp_wall_dist = side_dy - dy;

                        f64 wall_x;
                        if (side == 0) wall_x = pos_y + perp_wall_dist * raydir_y;
                        else wall_x = pos_x + perp_wall_dist * raydir_x;
                        wall_x -= floor(wall_x);

                        f32 visible_w = ease_out_sine(1.0f - door.delta_t);
                        if (wall_x > visible_w) {
                            hit = 0;
                        }
                    }
                }
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

        int tex_idx = wall - 1;

        Texture *texture = &state->textures[tex_idx];

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

    //@Note Sort entities
    for (int i = 0; i < state->entities.count; i++) {
        Entity *entity = state->entities[i];
        f64 dist = ((pos_x - entity->x) * (pos_x - entity->x) + (pos_y - entity->y) * (pos_y - entity->y));
        int j = i - 1;
        while (j >= 0) {
            Entity *ej = state->entities[j];
            f64 distj = (pos_x - ej->x) * (pos_x - ej->x) + (pos_y - ej->y) * (pos_y - ej->y);
            if (distj > dist) {
                break;
            }

            state->entities[j + 1] = state->entities[j];
            j = j - 1;
        }

        state->entities[j + 1] = entity;
    }

    //@Note entity raycasting
    for (int entity_idx = 0; entity_idx < state->entities.count; entity_idx++) {
        Entity *e = state->entities[entity_idx];
        f64 entity_x = e->x - pos_x;
        f64 entity_y = e->y - pos_y;

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

        f64 xform_x = det * (entity_x * dir.y + entity_y * -dir.x);
        f64 xform_y = det * (entity_x * -plane_y + entity_y * plane_x);

        int entity_screen_x = (int)(w / 2 * (1 + xform_x / xform_y));

        int entity_h = abs((int)(h / xform_y)) / e->v_scale;
        int adjust_y = (int)(e->v_adjust / xform_y);

        int y0 = (int)h / 2 - entity_h / 2 + adjust_y;
        int y1 = (int)h / 2 + entity_h / 2 + adjust_y;
        if (y0 < 0) y0 = 0;
        if (y1 >= h) y1 = (int)h - 1;

        int entity_w = abs((int)(h / xform_y)) / e->u_scale;
        int x0 = entity_screen_x - entity_w / 2;
        int x1 = entity_screen_x + entity_w / 2;
        if (x0 < 0) x0 = 0;
        if (x1 >= w) x1 = (int)w - 1;

        for (int x = x0; x < x1; x++) {
            int tex_x = (int)(256 * (x - (-entity_w / 2 + entity_screen_x)) * TEX_WIDTH / entity_w / 256);
            if (xform_y > 0 && x > 0 && x < w && xform_y < z_buffer[x]) {
                for (int y = y0; y < y1; y++) {
                    int d = (y - adjust_y) * 256 - (int)h * 128 + entity_h * 128;
                    int tex_y = ((d * TEX_HEIGHT) / entity_h) / 256;

                    Texture texture;
                    if (e->anim) {
                        texture = e->anim->textures[e->anim_frame];
                    } else {
                        texture = state->textures[e->tex];
                    }
                    u32 color = ((u32 *)texture.bitmap)[tex_y * TEX_WIDTH + tex_x];

                    if ((color & 0xFF000000) != 0) {
                        fill_pixel(&g_back_buffer, x, y, color);
                    }
                }
            }
        }
    }

    if (state->draw_map) {
        draw_map(v2_f32(SCREEN_WIDTH, SCREEN_HEIGHT));
    }
}

internal void update_and_render(OS_Event_List *events, OS_Handle window_handle, f32 delta_t) {
    local_persist bool first_call = true;
    if (first_call) {
        first_call = false;

        game_state = new Game_State();
        game_state->mode = GAME_MODE_WORLD;
        //game_state->pos = v2_f32(22.0f, 1.5f);
        game_state->pos = v2_f32(5.0f, 11.0f);
        game_state->dir = v2_f32(-1.0f, 0.0f);
        game_state->plane_x = 0;
        game_state->plane_y = 0.66;

        g_assets = push_array(permanent_arena, Assets, 1);
        g_assets->anim_table_size = 128;
        g_assets->anim_table = push_array(permanent_arena, Asset_Bucket, g_assets->anim_table_size);
        g_assets->texture_table_size = 128;
        g_assets->texture_table = push_array(permanent_arena, Asset_Bucket, g_assets->texture_table_size);

        os_chdir(str8_lit("data"));

        asset_load_texture("wolftex/eagle.png");
        asset_load_texture("wolftex/redbrick.png");
        asset_load_texture("wolftex/purplestone.png");
        asset_load_texture("wolftex/greystone.png");
        asset_load_texture("wolftex/bluestone.png");
        asset_load_texture("wolftex/mossy.png");
        asset_load_texture("wolftex/wood.png");
        asset_load_texture("wolftex/colorstone.png");
        asset_load_texture("wolftex/barrel.png");
        asset_load_texture("wolftex/pillar.png");
        asset_load_texture("wolftex/greenlight.png");
        asset_load_texture("mob.png");
        asset_load_texture("ball.png");
        asset_load_texture("door.png");

        asset_load_animation(str8_lit("ghost_idle"));

        Arena *temp = make_arena(get_malloc_allocator());
        default_font = load_font(temp, str8_lit("fonts/consolas.ttf"), 20);

        os_chdir(str8_lit(".."));


        update_screen_buffer(&g_back_buffer, SCREEN_WIDTH, SCREEN_HEIGHT);


        {
            make_entity(E_OBJ, 20.5, 11.5, 10, 1, 1, 0); //green light in front of playerstart
            
            // lights
            make_entity(E_OBJ, 18.5, 1.5, 8, 1, 1, 0);
            make_entity(E_OBJ, 15.5, 1.5, 8, 1, 1, 0);
            make_entity(E_OBJ, 16.0, 1.8, 8, 1, 1, 0);
            make_entity(E_OBJ, 16.2, 1.2, 8, 1, 1, 0);
            make_entity(E_OBJ, 18.5, 5.5, 8, 2, 2, 128);
            make_entity(E_OBJ, 15.5, 4.5, 8, 2, 2, 128);
            make_entity(E_OBJ, 19.0, 4.1, 8, 2, 2, 128);
            make_entity(E_OBJ, 14.5, 4.8, 8, 2, 2, 128);

            // barrels
            make_entity(E_WALL, 18.5, 4.5,  10, 1, 1, 0);
            make_entity(E_WALL, 10.0, 4.5,  10, 1, 1, 0);
            make_entity(E_WALL, 10.0, 12.5, 10, 1, 1, 0);
            make_entity(E_WALL, 3.5,  6.5,  10, 1, 1, 0);
            make_entity(E_WALL, 3.5,  20.5, 10, 1, 1, 0);
            make_entity(E_WALL, 3.5,  14.5, 10, 1, 1, 0);
            make_entity(E_WALL, 14.5, 20.5, 10, 1, 1, 0);

            // pillar
            make_entity(E_OBJ, 18.5, 10.5,  9, 1, 1, 0);
            make_entity(E_OBJ, 18.5, 11.5,  9, 1, 1, 0);
            make_entity(E_OBJ, 18.5, 12.5,  9, 1, 1, 0);
            make_entity(E_MOB, 17.0, 3.0,  11, 1, 1, 64);
        }

        A_Star *a_star = &game_state->a_star;
        a_star->dim_x = MAP_WIDTH;
        a_star->dim_y = MAP_HEIGHT;
        a_star->cells = (A_Star_Cell *)calloc(a_star->dim_y * a_star->dim_x, sizeof(A_Star_Cell));
        for (int y = 0; y < a_star->dim_y; y++) {
            for (int x = 0; x < a_star->dim_x; x++) {
                A_Star_Cell *cell = &a_star->cells[y * a_star->dim_x + x];
                cell->x = x;
                cell->y = y;
            }
        }
    }

    game_state->delta_t = delta_t;

    V2_F32 window_dim = os_get_window_dim(window_handle);

    input_begin(window_handle, events);

    clear_buffer(&g_back_buffer, 1, 0, 1, 1);

    switch (game_state->mode) {
        case GAME_MODE_MENU:
            break;
        case GAME_MODE_PAUSE:
            if (key_pressed(OS_KEY_ESCAPE)) {
                game_state->mode = GAME_MODE_WORLD;
            }
            break;
        case GAME_MODE_WORLD:
            update_game_world(game_state);
            break;
    }

    //@Note Draw backbuffer texture
    load_screen_buffer_texture(&g_back_buffer);

    draw_begin(window_handle);
    M4_F32 ortho = ortho_rh_zo(0.0f, window_dim.x, 0.0f, window_dim.y, -1.0f, 1.0f);
    draw_set_xform(ortho);
    draw_quad(g_back_buffer.tex, make_rect(0.0f, 0.0f, window_dim.x, window_dim.y), make_rect(0, 0, 1, 1));

    M4_F32 ortho_text = ortho_rh_zo(0.0f, window_dim.x, window_dim.y, 0.0f, -1.0f, 1.0f);
    draw_set_xform(ortho_text);
    draw_textf(default_font, v4_f32(1.0f, 0.0f, 1.0f, 1.0f), V2_Zero, "%f %f", game_state->pos.x, game_state->pos.y);
    draw_textf(default_font, v4_f32(1.0f, 0.0f, 1.0f, 1.0f), v2_f32(0.0f, 24.0f), "%f %f", game_state->dir.x, game_state->dir.y);
    
    d3d11_render(window_handle, draw_bucket);

    r_d3d11_state->swap_chain->Present(1, 0);

    draw_end();

    input_end(window_handle);
}
