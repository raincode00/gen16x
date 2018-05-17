#pragma once


struct Sprite {
    float x, y;
    float scale_x;
    float scale_y;
    int z_order;
    bool transparent;
    bool visible;
    int width;
    int height;
    int ppu_base_offset;
    int current_frame;
    int num_frames;
};


