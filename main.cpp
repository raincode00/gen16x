#include <cstdlib>
#include <string.h>
#include <cstdio>
#include <cmath>
#include <chrono>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <GL/glew.h>

#include "gen16x.h"

#include "shaders.h"


class Timer {
public:
    Timer() : beg_(clock_::now()) {}
    void reset() { beg_ = clock_::now(); }
    double elapsed() const {
        return std::chrono::duration_cast<second_>
        (clock_::now() - beg_).count();
    }
    
private:
    typedef std::chrono::high_resolution_clock clock_;
    typedef std::chrono::duration<double, std::ratio<1> > second_;
    std::chrono::time_point<clock_> beg_;
};


struct player_state {
    float pos_x;
    float pos_y;
    float height;
    float rot;
};

player_state player;

struct application_state {
    gen16x_ppu_state ppu;
    
    SDL_Window* window;
    SDL_GLContext context;
    
    bool quitting = false;
    double frame_no = 0.0;
    double delta_time = 0.0;
    
    uint32_t quad_va;
    uint32_t quad_vbo;
    uint32_t framebuffer_texture;
    uint32_t shader_prog;
    int texture_uni;
    int display_size_uni;
    int texture_size_uni;
    Timer timer;
    
    
};

application_state app;



uint64_t font_8x8[128] = {
    0x0000000000000000, 0x7E7E7E7E7E7E0000,    0x7E7E7E7E7E7E0000,    0x7E7E7E7E7E7E0000,
    0x7E7E7E7E7E7E0000,    0x7E7E7E7E7E7E0000,    0x7E7E7E7E7E7E0000,    0x7E7E7E7E7E7E0000,
    0x7E7E7E7E7E7E0000,    0x0000000000000000, 0x0000000000000000, 0x7E7E7E7E7E7E0000,
    0x7E7E7E7E7E7E0000,    0x7E7E7E7E7E7E0000,    0x7E7E7E7E7E7E0000,    0x7E7E7E7E7E7E0000,
    0x7E7E7E7E7E7E0000,    0x7E7E7E7E7E7E0000,    0x7E7E7E7E7E7E0000,    0x7E7E7E7E7E7E0000,
    0x7E7E7E7E7E7E0000,    0x7E7E7E7E7E7E0000,    0x7E7E7E7E7E7E0000,    0x7E7E7E7E7E7E0000,
    0x7E7E7E7E7E7E0000,    0x7E7E7E7E7E7E0000,    0x7E7E7E7E7E7E0000,    0x7E7E7E7E7E7E0000,
    0x7E7E7E7E7E7E0000,    0x7E7E7E7E7E7E0000,    0x7E7E7E7E7E7E0000,    0x7E7E7E7E7E7E0000,
    0x0000000000000000, 0x0808080800080000,    0x2828000000000000,    0x00287C287C280000,
    0x081E281C0A3C0800,    0x6094681629060000,    0x1C20201926190000,    0x0808000000000000,
    0x0810202010080000,    0x1008040408100000,    0x2A1C3E1C2A000000,    0x0008083E08080000,
    0x0000000000081000, 0x0000003C00000000,    0x0000000000080000, 0x0204081020400000,
    0x1824424224180000,    0x08180808081C0000,    0x3C420418207E0000,    0x3C420418423C0000,
    0x081828487C080000,    0x7E407C02423C0000,    0x3C407C42423C0000,    0x7E04081020400000,
    0x3C423C42423C0000,    0x3C42423E023C0000,    0x0000080000080000, 0x0000080000081000,
    0x0006186018060000,    0x00007E007E000000,    0x0060180618600000,    0x3844041800100000,
    0x003C449C945C201C,    0x1818243C42420000,    0x7844784444780000,    0x3844808044380000,
    0x7844444444780000,    0x7C407840407C0000,    0x7C40784040400000,    0x3844809C44380000,
    0x42427E4242420000,    0x3E080808083E0000,    0x1C04040444380000,    0x4448507048440000,
    0x40404040407E0000,    0x4163554941410000,    0x4262524A46420000,    0x1C222222221C0000,
    0x7844784040400000,    0x1C222222221C0200,    0x7844785048440000,    0x1C22100C221C0000,
    0x7F08080808080000,    0x42424242423C0000,    0x8142422424180000,    0x4141495563410000,
    0x4224181824420000,    0x4122140808080000,    0x7E040810207E0000,    0x3820202020380000,
    0x4020100804020000,    0x3808080808380000,    0x1028000000000000,    0x00000000007E0000,
    0x1008000000000000,    0x003C023E463A0000,    0x40407C42625C0000,    0x00001C20201C0000,
    0x02023E42463A0000,    0x003C427E403C0000,    0x0018103810100000,    0x0000344C44340438,
    0x2020382424240000,    0x0800080808080000,    0x0800180808080870,    0x20202428302C0000,
    0x1010101010180000,    0x0000665A42420000,    0x00002E3222220000,    0x00003C42423C0000,
    0x00005C62427C4040,    0x00003A46423E0202,    0x00002C3220200000,    0x001C201804380000,
    0x00103C1010180000,    0x00002222261A0000,    0x0000424224180000,    0x000081815A660000,
    0x0000422418660000,    0x0000422214081060,    0x00003C08103C0000,    0x1C103030101C0000,
    0x0808080808080800,    0x38080C0C08380000,    0x000000324C000000,    0x7E7E7E7E7E7E0000};


#include "assets/test_sprite_tiles.h"
#include "assets/test_sprite_tiles2.h"

const unsigned char test_tileset[256*256] = {
#include "assets/test_tileset.txt"
};


const unsigned char test_tilemap[32*32] = {
#include "assets/test_tilemap.txt"
};


const unsigned char test_background[1152 * 80] = {
#include "assets/test_background.txt"
};


void init_ppu() {
    memset(&app.ppu, 0, sizeof(app.ppu));
    app.ppu.screen_height = 208;
    app.ppu.screen_width = 384;
    
    for (int r = 0; r < 6; r++) {
        for (int g = 0; g < 6; g++) {
            for (int b = 0; b < 6; b++) {
                float rf = float(r) / 5;
                float gf = float(g) / 5;
                float bf = float(b) / 5;
                
                unsigned int argb[4] = {
                    0xFF,
                    (unsigned int)(rf*255.0f),
                    (unsigned int)(gf*255.0f),
                    (unsigned int)(bf*255.0f),
                };
                int i = r*36 + g*6 + b;
                gen16x_color32 color;
                color.a = argb[0];
                color.r = argb[1];
                color.g = argb[2];
                color.b = argb[3];
                //unsigned int argb32 = (argb[0]) | (argb[1] << 8) | (argb[2] << 16) | (argb[3] << 24);
                //app.ppu.cgram[i] = argb32_to_argb16(color);
                app.ppu.cgram32[i] = color;
                
            }
        }
    }
    
    //app.ppu.cgram[0x00] = 0b0111110000011111;
    //app.ppu.cgram[0xEF] = 0b1000000000000000;
    //app.ppu.cgram[0xFF] = 0b1111111111111111;
    
    app.ppu.cgram32[0x00].color_i = 0x00FF00FF;
    app.ppu.cgram32[0xEF].color_i = 0xFF000000;
    app.ppu.cgram32[0xFF].color_i = 0xFFFFFFFF;
    
    
    int offset = 0;
    app.ppu.layers[0].layer_type = GEN16X_LAYER_DIRECT;
    app.ppu.layers[0].vram_offset = 0;
    
    
    gen16x_ppu_layer_direct& direct_layer = *(gen16x_ppu_layer_direct*)(app.ppu.vram + app.ppu.layers[0].vram_offset);
    
    
    direct_layer.width = 1152;
    direct_layer.height = 80;
    
    /*for (int i = 0; i < direct_layer.height; i++) {
     for (int j = 0; j < direct_layer.width; j++) {
     direct_layer.map[i * direct_layer.width + j] = ((i / 16) * direct_layer.width / 16) + j / 16;
     
     if (i == j || i == direct_layer.width - j - 1) {
     direct_layer.map[i * direct_layer.width + j] = 200;
     }
     direct_layer.map[i*direct_layer.width + j] = test_background[i*direct_layer.width + j];
     }
     }*/
    memcpy(direct_layer.map, test_background, sizeof(test_background));
    
    /*for (int i = 0; i < 256; i++) {
     for (int j = 0; j < 256; j++) {
     direct_layer.map[(i + 16) * direct_layer.width + j + 16] = test_tileset[i*256 + j];
     }
     }*/
    
    offset += direct_layer.width*direct_layer.height + 128;
    
    app.ppu.layers[1].layer_type = GEN16X_LAYER_TILES;
    
    app.ppu.layers[1].vram_offset = offset;
    
    gen16x_ppu_layer_tiles& tile_layer = *(gen16x_ppu_layer_tiles*)(app.ppu.vram + app.ppu.layers[1].vram_offset);
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            int tile_index = i * 16 + j;
            for (int row = 0; row < 16; row++) {
                for (int col = 0; col < 16; col++) {
                    tile_layer.tile_palette[(tile_index << 8) | (row << 4) | (col)] = test_tileset[(i * 16 + row) * 256 + j*16 + col];
                }
                
            }
        }
    }
    
    memcpy(tile_layer.tile_map, test_tilemap, sizeof(test_tilemap));
    tile_layer.tile_size = GEN16X_TILE16;
    tile_layer.flags =  GEN16X_FLAG_TRANSFORM | GEN16X_FLAG_REPEAT_X | GEN16X_FLAG_REPEAT_Y;
    tile_layer.tilemap_width = 5;
    tile_layer.tilemap_height = 5;
    
    
    offset += (sizeof(gen16x_ppu_layer_tiles));
    
    
    app.ppu.layers[2].layer_type = GEN16X_LAYER_TILES;
    app.ppu.layers[2].vram_offset = offset;
    
    gen16x_ppu_layer_tiles& tile_layer2 = *(gen16x_ppu_layer_tiles*)(app.ppu.vram + app.ppu.layers[2].vram_offset);
    
    for (int i = 0; i < 128; i++) {
        uint64_t tmp = font_8x8[i];
        uint64_t mask = 0x8000000000000000L;
        for (int col = 0; col < 64; col++) {
            unsigned char b = (tmp & (mask >> col)) ? 32 : 0x00;
            tile_layer2.tile_palette[(i << 7) | (col)] = b;
        }
    }
    char test_text[] = "hello world";
    
    memcpy(tile_layer2.tile_map, test_text, sizeof(test_text));
    tile_layer2.tile_size = GEN16X_TILE8;
    tile_layer2.flags = 0;
    tile_layer2.tilemap_width = 4;
    tile_layer2.tilemap_height = 1;
    
    //tile_layer2.transform.base = 8;
    tile_layer2.transform.x = -10;
    tile_layer2.transform.y = -8;
    tile_layer2.transform.a = 1 << (tile_layer2.transform.base);
    tile_layer2.transform.b = 0;
    tile_layer2.transform.c = 0;
    tile_layer2.transform.d = 1 << (tile_layer2.transform.base);
    
    
    offset += (sizeof(gen16x_ppu_layer_tiles));
    
    

    app.ppu.layers[3].layer_type = GEN16X_LAYER_SPRITES;
    app.ppu.layers[3].vram_offset = offset;

    gen16x_ppu_layer_sprites& sprites_layer = *(gen16x_ppu_layer_sprites*)(app.ppu.vram + app.ppu.layers[3].vram_offset);


    memcpy(sprites_layer.sprite_palette, test_sprite_tiles, sizeof(test_sprite_tiles));
    memcpy(sprites_layer.sprite_palette + sizeof(test_sprite_tiles), test_sprite2_tiles, sizeof(test_sprite2_tiles));
    for (int i = 0; i < 15; i++) {
        app.ppu.cgram32[220 + i].color_i = test_sprite_palette[i];
    }
    for (int i = 0; i < 11; i++) {
        app.ppu.cgram32[235 + i].color_i = test_sprite2_palette[i];
    }

    sprites_layer.sprites[0].palette_offset = 220;
    sprites_layer.sprites[0].flags = GEN16X_FLAG_SPRITE_ENABLED;
    sprites_layer.sprites[0].size = GEN16X_MAKE_SPRITE_SIZE(5, 5);
    sprites_layer.sprites[0].tile_index = 0;
    sprites_layer.sprites[0].x = app.ppu.screen_width/2 - 16;
    sprites_layer.sprites[0].y = 80;

    sprites_layer.sprites[1].palette_offset = 220;
    sprites_layer.sprites[1].flags = GEN16X_FLAG_SPRITE_ENABLED;
    sprites_layer.sprites[1].size = GEN16X_MAKE_SPRITE_SIZE(5, 5);
    sprites_layer.sprites[1].tile_index = (4 * 4)*0;
    sprites_layer.sprites[1].x = 32;
    sprites_layer.sprites[1].y = 32;

    sprites_layer.sprites[2].palette_offset = 220;
    sprites_layer.sprites[2].flags =  GEN16X_FLAG_SPRITE_ENABLED;
    sprites_layer.sprites[2].size = GEN16X_MAKE_SPRITE_SIZE(5,5);
    sprites_layer.sprites[2].tile_index = (4 * 4)*6;
    sprites_layer.sprites[2].x = 47;
    sprites_layer.sprites[2].y = 65;

    sprites_layer.sprites[3].palette_offset = 220;
    sprites_layer.sprites[3].flags = GEN16X_FLAG_SPRITE_ENABLED;
    sprites_layer.sprites[3].size = GEN16X_MAKE_SPRITE_SIZE(5, 5);
    sprites_layer.sprites[3].tile_index = (4 * 4)*10;
    sprites_layer.sprites[3].x = 64 + 16;
    sprites_layer.sprites[3].y = 180;


    sprites_layer.sprites[4].palette_offset = 235;
    sprites_layer.sprites[4].flags = GEN16X_FLAG_SPRITE_ENABLED;
    sprites_layer.sprites[4].size = GEN16X_MAKE_SPRITE_SIZE(4, 4);
    sprites_layer.sprites[4].tile_index = sizeof(test_sprite_tiles) / 32;
    sprites_layer.sprites[4].x = 64 + 16;
    sprites_layer.sprites[4].y = 180;
    


    for (int i = 5; i < 16; i++) {


        sprites_layer.sprites[i].palette_offset = 235;
        sprites_layer.sprites[i].flags = GEN16X_FLAG_SPRITE_ENABLED;
        sprites_layer.sprites[i].size = GEN16X_MAKE_SPRITE_SIZE(4, 4);
        sprites_layer.sprites[i].tile_index = (sizeof(test_sprite_tiles) / 32)  + (2 * 2) * (i % 16);
        sprites_layer.sprites[i].x = ((i % 32) * 10);
        sprites_layer.sprites[i].y = 32 + ((i / 32) * 128) + 10*(i%7)*(i%3);

    }
    

    offset += (sizeof(gen16x_ppu_layer_sprites));



    app.ppu.row_callback = (gen16x_ppu_row_callback_t)[](unsigned int y) {
        if (y > 16) {
            app.ppu.layers[2].layer_type = GEN16X_LAYER_PASS;
        }
        static int z = 0;
        z += 1;
        gen16x_ppu_layer_sprites& sprites_layer = *(gen16x_ppu_layer_sprites*)(app.ppu.vram + app.ppu.layers[3].vram_offset);

        //sprites_layer.sprites[1].x = 100 + sinf(y/7.0f)*(app.ppu.screen_height - y)/5.0f;
        //sprites_layer.sprites[1].y = (sprites_layer.sprites[1].y/4 - y) + cosf(y / 7.0f)*5;

        int h = y - 80;
        
        if (h > 0) {
            
            app.ppu.layers[0].layer_type = GEN16X_LAYER_PASS;
            app.ppu.layers[1].layer_type = GEN16X_LAYER_TILES;
            float ay = player.height + 16;
            
            float lambda = ay / (float(h + 4.0f));
            
            
            float p_cos = cosf(player.rot*3.1415926f/180.0f);
            float p_sin = sinf(player.rot*3.1415926f/180.0f);
            
            
            float f_x = p_sin;
            float f_y = p_cos;
            
            gen16x_ppu_layer_tiles* layer_tiles = (gen16x_ppu_layer_tiles*)(app.ppu.vram + app.ppu.layers[1].vram_offset);
            
            layer_tiles->transform.a = (int)(((1 << layer_tiles->transform.base)* 1.0f *  p_cos)*lambda);
            layer_tiles->transform.b = (int)(((1 << layer_tiles->transform.base)* 1.0f *  p_sin)*lambda);
            layer_tiles->transform.c = (int)(((1 << layer_tiles->transform.base)* 1.0f * -p_sin)*lambda);
            layer_tiles->transform.d = (int)(((1 << layer_tiles->transform.base)* 1.0f *  p_cos)*lambda);
            
            layer_tiles->transform.x =  (int)(player.pos_x - 0*f_x*lambda);
            layer_tiles->transform.y =  (int)(player.pos_y - 0*f_y*lambda);
            layer_tiles->transform.cx = (int)(player.pos_x - 0*f_x*lambda + app.ppu.screen_width / 2);
            layer_tiles->transform.cy = (int)(player.pos_y - 0*f_y*lambda + app.ppu.screen_height - 16);

            


            
        }
    };
    app.ppu.framebuffer_offset = offset;
    
    printf("Initialized %d bytes of vram\n", (int)sizeof(app.ppu));
}


bool compile_shader(uint32_t shader_type, uint32_t shader, const char* shader_source) {
    glShaderSource(shader, 1, &shader_source, 0);
    
    int result;
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    
    if (result == GL_FALSE) {
        int log_size;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
        if (log_size > 0) {
            char* error_log = new char[log_size];
            glGetShaderInfoLog(shader, log_size, &log_size, error_log);
            printf("%s\n\n%s\n", shader_source, error_log);
            delete[] error_log;
            return false;
        }
    }
    
    return true;
}



int SDLCALL watch(void *userdata, SDL_Event* event) {
    
    if (event->type == SDL_APP_WILLENTERBACKGROUND) {
        app.quitting = true;
    }
    
    return 1;
}



bool init_sdl() {
    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO) != 0) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return 1;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    
    
    app.window = SDL_CreateWindow("gen16x", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, app.ppu.screen_width, app.ppu.screen_height, SDL_WINDOW_OPENGL|SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_RESIZABLE);
    
    //SDL_SetWindowResizable(g_window, SDL_TRUE);
    app.context = SDL_GL_CreateContext(app.window);
    SDL_GL_SetSwapInterval(0);
    
    //SDL_SetWindowFullscreen(g_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    
    SDL_AddEventWatch(watch, NULL);
    
    return true;
}

void cleanup_sdl() {
    
    SDL_DelEventWatch(watch, NULL);
    SDL_GL_DeleteContext(app.context);
    SDL_DestroyWindow(app.window);
}

bool init_opengl() {
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        printf("Error: %s\n", glewGetErrorString(err));
        return false;
    }
    
    if (!GLEW_VERSION_3_3) {
        printf("Error: OpenGL 3.3 not supported.\n");
        return false;
    }
    glGetError();
    printf("Status: Using OpenGL %s\n", glGetString(GL_VERSION));
    
    
    glEnable(GL_FRAMEBUFFER_SRGB);
    
    
    glGenTextures(1, &app.framebuffer_texture);
    glBindTexture(GL_TEXTURE_2D, app.framebuffer_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, app.ppu.screen_width,
                 app.ppu.screen_height, 0, GL_BGRA, GL_UNSIGNED_BYTE,
                 app.ppu.vram + app.ppu.framebuffer_offset);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    
    float quad[] =
    {
        -1.0f, -1.0f,
        -1.0,  3.0f,
         3.0f, -1.0f
    };
    
    
    
    glGenVertexArrays(1, &app.quad_va);
    
    glBindVertexArray(app.quad_va);
    glGenBuffers(1, &app.quad_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, app.quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    
    err = glGetError();
    if (err) {
        printf("OpenGL Error 1 - %d\n", err);
        return false;
    }
    
    
    app.shader_prog = glCreateProgram();
    uint32_t quad_vsh = glCreateShader(GL_VERTEX_SHADER);
    uint32_t quad_psh = glCreateShader(GL_FRAGMENT_SHADER);
    
    if (compile_shader(GL_VERTEX_SHADER, quad_vsh, g_quad_vsh)) {
        glAttachShader(app.shader_prog, quad_vsh);
    }
    
    if (compile_shader(GL_FRAGMENT_SHADER, quad_psh, g_quad_psh)) {
        glAttachShader(app.shader_prog, quad_psh);
    }
    
    int result;
    glLinkProgram(app.shader_prog);
    glGetProgramiv(app.shader_prog, GL_LINK_STATUS, &result);
    
    if (result == GL_FALSE) {
        int log_size;
        glGetProgramiv(app.shader_prog, GL_INFO_LOG_LENGTH, &log_size);
        if (log_size > 0) {
            char* error_log = new char[log_size];
            glGetProgramInfoLog(app.shader_prog, log_size, &log_size, error_log);
            printf("%s\n", error_log);
            delete[] error_log;
            return false;
        }
    }
    
    glDeleteShader(quad_vsh);
    glDeleteShader(quad_psh);
    
    int pos_attr = glGetAttribLocation(app.shader_prog, "position");
    
    glVertexAttribPointer(pos_attr, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(pos_attr);
    
    
    glUseProgram(app.shader_prog);
    app.texture_uni = glGetUniformLocation(app.shader_prog, "texture");
    app.display_size_uni = glGetUniformLocation(app.shader_prog, "display_size");
    app.texture_size_uni = glGetUniformLocation(app.shader_prog, "texture_size");
    
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, app.framebuffer_texture);
    
    err = glGetError();
    if (err) {
        printf("OpenGL Error 2 - %d\n", err);
        app.quitting = true;
    }
    
    
    glProgramUniform1i(app.shader_prog, app.texture_uni, 0);
    glProgramUniform2f(app.shader_prog, app.texture_size_uni,
                       app.ppu.screen_width, app.ppu.screen_height);
    int draw_w;
    int draw_h;
    
    SDL_GL_GetDrawableSize(app.window, &draw_w, &draw_h);
    glProgramUniform2f(app.shader_prog, app.display_size_uni, (float)draw_w, (float)draw_h);
    glViewport(0, 0, draw_w, draw_h);
    return true;
}

void cleanup_opengl() {
    glDeleteTextures(1, &app.framebuffer_texture);
    glDeleteBuffers(1, &app.quad_vbo);
    glDeleteVertexArrays(1, &app.quad_va);
}

void render_opengl() {
    
    
    
    
    
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, app.ppu.screen_width, app.ppu.screen_height,
                    GL_BGRA, GL_UNSIGNED_BYTE, app.ppu.vram + app.ppu.framebuffer_offset);
    
    int err = glGetError();
    if (err) {
        printf("OpenGL Error - %d\n", err);
        app.quitting = true;
    }
    
    
    int d_w, d_h;
    int w_w, w_h;
    
    SDL_GetWindowSize(app.window, &w_w, &w_h);
    SDL_GL_GetDrawableSize(app.window, &d_w, &d_h);
    
    float dpi_scale_w = (float)d_w/(float)w_w;
    float dpi_scale_h = (float)d_h/(float)w_h;
    
    float scale_w = float(d_w) / float(app.ppu.screen_width);
    float scale_h = float(d_h) / float(app.ppu.screen_height);
    
    int orig_w = d_w;
    int orig_h = d_h;
    
    float scale = (scale_w < scale_h) ? scale_w : scale_h;
    
    d_w = (int)((float)app.ppu.screen_width*scale/2)*2;
    d_h = (int)((float)app.ppu.screen_height*scale/2)*2;
    
    if (d_w < app.ppu.screen_width || d_w < app.ppu.screen_height) {
        d_w = (float)app.ppu.screen_width;
        d_h = (float)app.ppu.screen_height;
    }
    
    
    int offset_w = (orig_w - d_w) / 2;
    int offset_h = (orig_h - d_h) / 2;
    
    glProgramUniform2f(app.shader_prog, app.display_size_uni, d_w, d_h);
    glViewport(offset_w, offset_h, d_w, d_h);
    
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void handle_resize(int new_w, int new_h) {
    
    
    Uint32 flags = SDL_GetWindowFlags(app.window);
    if (!(flags & (SDL_WINDOW_FULLSCREEN|SDL_WINDOW_MAXIMIZED))) {
        
        // SDL_SetWindowSize(g_window, vp_width, vp_height);
    }
    //printf("Window resized to: %d x %d\n", vp_width, vp_height);
}

void handle_sdl_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            app.quitting = true;
        } else if (event.type == SDL_WINDOWEVENT) {
            switch (event.window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                    handle_resize(event.window.data1, event.window.data2);
                    break;
            }
            
        } else if (event.type == SDL_KEYDOWN) {
            //bool mod_ctrl = !!(event.key.keysym.mod & KMOD_CTRL);
            //bool mod_shift = !!(event.key.keysym.mod & KMOD_SHIFT);
            bool mod_alt = !!(event.key.keysym.mod & KMOD_ALT);
            
            switch (event.key.keysym.sym) {
                case SDLK_RETURN:
                    if (mod_alt) {
                        Uint32 flags = SDL_GetWindowFlags(app.window);
                        if (flags & SDL_WINDOW_FULLSCREEN) {
                            SDL_SetWindowFullscreen(app.window, SDL_FALSE);
                        }
                        else {
                            SDL_SetWindowFullscreen(app.window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                        }
                    }
                    break;
            }
            
        }
        
    }
    
}

void handle_sdl_input() {
    
    const Uint8 *kbstate = SDL_GetKeyboardState(NULL);
    
    float speed_scale = 20.0f;
    if (kbstate[SDL_SCANCODE_W]) {
        float forward_x = -sinf(3.1415926f*player.rot/ 180.0f);
        float forward_y = cosf(3.1415926f*player.rot / 180.0f);
        
        player.pos_x += speed_scale*10.0f*app.delta_time*forward_x;
        player.pos_y += speed_scale*10.0f*app.delta_time*-forward_y;
        
        
    }
    
    if (kbstate[SDL_SCANCODE_S]) {
        float forward_x = -sinf(3.1415926f*player.rot/180.0f);
        float forward_y = cosf(3.1415926f*player.rot / 180.0f);
        
        player.pos_x += 5.0f*speed_scale*app.delta_time*-forward_x;
        player.pos_y += 5.0f*speed_scale*app.delta_time*forward_y;
        
    }
    
    if (kbstate[SDL_SCANCODE_A]) {
        player.rot += 10.0f*speed_scale*app.delta_time;
    }
    
    if (kbstate[SDL_SCANCODE_D]) {
        player.rot -= 10.0f*speed_scale*app.delta_time;
    }
    
    if (kbstate[SDL_SCANCODE_F]) {
        player.height -= 4.0f*speed_scale*app.delta_time;
        if (player.height < 0) player.height = 0;
    }
    if (kbstate[SDL_SCANCODE_R]) {
        player.height += 4.0f*speed_scale*app.delta_time;
    }
    
}

int main() {
    
    player.pos_x = 0;
    player.pos_y = 0;
    player.height = 0;
    player.rot = 0;
    
    init_ppu();
    
    init_sdl();
    
    init_opengl();
    
    
    while (!app.quitting) {
        
        handle_sdl_events();
        handle_sdl_input();
        
        gen16x_ppu_layer_direct& direct_layer = *(gen16x_ppu_layer_direct*)(app.ppu.vram + app.ppu.layers[0].vram_offset);
        
        direct_layer.scroll_y = 0;
        direct_layer.scroll_x = (short)((-player.rot/120.0f)*app.ppu.screen_width);
        direct_layer.flags |= GEN16X_FLAG_REPEAT_X;
        
        
        app.ppu.layers[0].layer_type = GEN16X_LAYER_DIRECT;
        app.ppu.layers[1].layer_type = GEN16X_LAYER_PASS;
        app.ppu.layers[2].layer_type = GEN16X_LAYER_TILES;


        gen16x_ppu_layer_sprites& sprites_layer = *(gen16x_ppu_layer_sprites*)(app.ppu.vram + app.ppu.layers[3].vram_offset);

        static int frame_no = 0;

        frame_no++;
        sprites_layer.sprites[0].tile_index = 16*(int(frame_no/100.0)%20);

        //sprites_layer.sprites[1].tile_index = 16 * ((int(frame_no / 100.0) + 3)  % 20);
        sprites_layer.sprites[2].tile_index = 16 * ((int(frame_no / 500.0) + 6) % 20);
        sprites_layer.sprites[3].tile_index = 16 * ((int(frame_no / 500.0) + 9) % 20);

        sprites_layer.sprites[1].x = -16 + app.ppu.screen_width / 2 + cosf(-frame_no / 5000.0f)*app.ppu.screen_width / 2;
        sprites_layer.sprites[1].y = -16 + app.ppu.screen_height / 2 + sinf(-frame_no / 5000.0f)*app.ppu.screen_height / 2;
        //layer_tiles->transform.base = 12;
        
        
        gen16x_ppu_render(&app.ppu);
        render_opengl();
        
        if (app.timer.elapsed() > 1.0 && app.frame_no > 0.0) {
            char fps_text[32];
            app.delta_time = (float)(app.timer.elapsed() / app.frame_no);
            sprintf(fps_text, "FPS: %0.2f\n", app.frame_no / app.timer.elapsed());
            //printf("%s", fps_text);
            gen16x_ppu_layer_tiles& tile_layer2 = *(gen16x_ppu_layer_tiles*)(app.ppu.vram + app.ppu.layers[2].vram_offset);
            strncpy((char*)tile_layer2.tile_map, fps_text, 32);
            
            
            app.frame_no = 0.0;
            app.timer.reset();
        }
        app.frame_no++;
        SDL_GL_SwapWindow(app.window);
        
    }
    
    
    cleanup_opengl();
    cleanup_sdl();
    
    SDL_Quit();
    
    return 0;
}
