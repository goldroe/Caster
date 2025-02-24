#ifndef CASTER_H
#define CASTER_H

struct Back_Buffer {
    u8 *pixels;
    int width;
    int height;
    int bytes_per_pixel;
    R_Handle tex;
};

struct Game_State {
    V2_F32 pos;
    V2_F32 dir;

    f64 plane_x;
    f64 plane_y;
};

#endif // CASTER_H
