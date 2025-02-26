#ifndef CASTER_H
#define CASTER_H

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

struct Game_State {
    V2_F32 pos;
    V2_F32 dir;

    f64 plane_x;
    f64 plane_y;

    Auto_Array<Texture> textures;
    Auto_Array<Entity*> entities;
};

#endif // CASTER_H
