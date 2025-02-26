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

struct Sprite {
    f64 x;
    f64 y;
    int u_scale;
    int v_scale;
    f32 v_adjust;
    int tex;
};

struct Game_State {
    V2_F32 pos;
    V2_F32 dir;

    f64 plane_x;
    f64 plane_y;

    Auto_Array<Texture> textures;
};

#endif // CASTER_H
