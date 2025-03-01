#ifndef CASTER_H
#define CASTER_H

struct A_Star_Cell {
    A_Star_Cell *parent;
    A_Star_Cell *next;
    int x;
    int y;

    int f;
    int g;
    int h;
};

struct A_Star {
    int dim_x;
    int dim_y;
    A_Star_Cell *cells;
};

struct Door {
    int state;
    f32 delta_t;
    f32 target_t;
};

struct Raycast_Result {
    f32 dist;
    int dest_x;
    int dest_y;
    int side;
};

struct Back_Buffer {
    u8 *pixels;
    int width;
    int height;
    int bytes_per_pixel;
    R_Handle tex;
};

struct Texture {
    int width;
    int height;
    u8 *bitmap;
    R_Handle tex;
};

enum {
    STATE_CLOSE,
    STATE_OPENING,
    STATE_OPEN
};

enum {
    E_WALL,
    E_OBJ,
    E_PLAYER,
    E_MOB,
    E_BALL,
    E_COUNT
};

struct Entity {
    int kind;
    f64 x;
    f64 y;

    f64 dir_x;
    f64 dir_y;

    // sprite
    int tex;
    int u_scale;
    int v_scale;
    f32 v_adjust;

    b32 to_be_destroyed;

    bool inline is_collidable() {
        return kind == E_WALL || kind == E_MOB || kind == E_PLAYER;
    }
};

enum Game_Mode {
    GAME_MODE_MENU,
    GAME_MODE_PAUSE,
    GAME_MODE_WORLD,
};

struct Game_State {
    f32 delta_t;

    V2_F32 pos;
    V2_F32 dir;
    f64 plane_x;
    f64 plane_y;

    Game_Mode mode;

    Auto_Array<Texture> textures;
    Auto_Array<Entity*> entities;

    A_Star a_star;
};

#endif // CASTER_H
