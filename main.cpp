#include <cstdlib>
#include <string.h>
#include <cstdio>
#include <cmath>
#include <chrono>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_audio.h>
#include <GL/glew.h>

#include "gen16x.h"
#include "physics.h"

#include "shaders.h"
const int SAMPLE_RATE = 44000;

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
    vec2 pos;
    float height;
    float rot;
    vec2 move;
    vec2 forward;
    bool moving;
    int state;
    int dir;
};

player_state player;

struct application_state {
    gen16x_ppu ppu;
    gen16x_spu spu;
    Timer timer;

    SDL_Window* window;
    SDL_Surface* surface;
    SDL_GLContext context;
    
    SDL_AudioSpec audio_want;
    SDL_AudioSpec audio_have;
    int audio_sample_nr = 0;
    
    
    bool quitting = false;
    double frame_no = 0.0;
    double delta_time = 0.0;
    double current_time = 0.0;
    bool opengl_enabled;
    uint32_t quad_va;
    uint32_t quad_vbo;
    uint32_t framebuffer_texture;
    uint32_t shader_prog;
    int texture_uni;
    int display_size_uni;
    int texture_size_uni;
    
    
    
};

static application_state app;



//#include "assets/test_sprite_tiles.h"
#include "assets/test_sprite_tiles2.h"
#include "assets/test_sprite3.h"
#include "assets/test_tileset_2.h"
#include "assets/test_tilemap.h"
#include "assets/slash_anim.h"
#include "assets/test_font.h"
/*
const unsigned char test_tileset[256*256] = {
#include "assets/test_tileset.txt"
};

const unsigned char test_tilemap[32*32] = {
#include "assets/test_tilemap.txt"
};

*/


const unsigned char test_background[1152 * 80] = {
#include "assets/test_background.txt"
};


void init_ppu() {
    memset(&app.ppu, 0, sizeof(app.ppu));

    
    app.ppu.screen_height = 216;
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
    /*
    app.ppu.layers[0].layer_type = GEN16X_LAYER_NONE;
    app.ppu.layers[0].vram_offset = 0;
    
    
    gen16x_layer_direct& direct_layer = *(gen16x_layer_direct*)(app.ppu.vram + app.ppu.layers[0].vram_offset);
    
    
    app.ppu.layers[0].direct_layer.width = 1152;
    app.ppu.layers[0].direct_layer.height = 80;
    
    for (int i = 0; i < direct_layer.height; i++) {
     for (int j = 0; j < direct_layer.width; j++) {
     direct_layer.map[i * direct_layer.width + j] = ((i / 16) * direct_layer.width / 16) + j / 16;
     
     if (i == j || i == direct_layer.width - j - 1) {
     direct_layer.map[i * direct_layer.width + j] = 200;
     }
     direct_layer.map[i*direct_layer.width + j] = test_background[i*direct_layer.width + j];
     }
     }
    memcpy(direct_layer.map, test_background, sizeof(test_background));
    */

    /*for (int i = 0; i < 256; i++) {
     for (int j = 0; j < 256; j++) {
     direct_layer.map[(i + 16) * direct_layer.width + j + 16] = test_tileset[i*256 + j];
     }
     }*/
    

  



    app.ppu.layers[0].layer_type = GEN16X_LAYER_TILES;
    app.ppu.layers[0].vram_offset = offset;
    app.ppu.layers[3].layer_type = GEN16X_LAYER_TILES;
    app.ppu.layers[3].vram_offset = offset;
    
    unsigned char* tile_layer0 = (app.ppu.vram + app.ppu.layers[0].vram_offset);
    //gen16x_layer_tiles& tile_layer1 = *(gen16x_layer_tiles*)(app.ppu.vram + app.ppu.layers[3].vram_offset);
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            int tile_index = i * 16 + j;
            for (int row = 0; row < 16; row++) {
                for (int col = 0; col < 16; col++) {
                    tile_layer0[(tile_index << 8) | (row << 4) | (col)] = test_tileset_2_tileset[(i * 16 + row) * 256 + j*16 + col];
                    //tile_layer1.tile_set[(tile_index << 8) | (row << 4) | (col)] = test_tileset_2_tileset[(i * 16 + row) * 256 + j*16 + col];
                }
                
            }
        }
    }

    offset += sizeof(test_tileset_2_tileset);

    for (int i = 0; i < sizeof(test_tileset_2_palette)/4; i++) {
        app.ppu.cgram32[i].color_i = test_tileset_2_palette[i];
    }
    
    app.ppu.layers[0].tile_layer.tilemap_vram_offset = offset;
    unsigned char* tilemap_layer0 = (app.ppu.vram + app.ppu.layers[0].tile_layer.tilemap_vram_offset);

    memcpy(tilemap_layer0, test_map_layer0_tilemap, sizeof(test_map_layer0_tilemap));

    offset += sizeof(test_map_layer0_tilemap);

    app.ppu.layers[3].tile_layer.tilemap_vram_offset = offset;
    unsigned char* tilemap_layer1 = (app.ppu.vram + app.ppu.layers[3].tile_layer.tilemap_vram_offset);


    memcpy(tilemap_layer1, test_map_layer1_tilemap, sizeof(test_map_layer1_tilemap));
    offset += sizeof(test_map_layer1_tilemap);


    app.ppu.layers[0].tile_layer.tile_size = GEN16X_TILE16;
    app.ppu.layers[0].tile_layer.flags =  GEN16X_FLAG_CLAMP_X | GEN16X_FLAG_CLAMP_Y;
    app.ppu.layers[0].tile_layer.tilemap_width = 6;
    app.ppu.layers[0].tile_layer.tilemap_height = 6;
    app.ppu.layers[0].tile_layer.transform.a = (1 << gen16x_transform::base);
    app.ppu.layers[0].tile_layer.transform.b = 0;
    app.ppu.layers[0].tile_layer.transform.c = 0;
    app.ppu.layers[0].tile_layer.transform.d = (1 << gen16x_transform::base);

    app.ppu.layers[3].tile_layer.tile_size = GEN16X_TILE16;
    app.ppu.layers[3].tile_layer.flags =  GEN16X_FLAG_CLAMP_X | GEN16X_FLAG_CLAMP_Y;
    app.ppu.layers[3].tile_layer.tilemap_width = 6;
    app.ppu.layers[3].tile_layer.tilemap_height = 6;
    
    app.ppu.layers[3].tile_layer.transform.a = (1 << gen16x_transform::base);
    app.ppu.layers[3].tile_layer.transform.b = 0;
    app.ppu.layers[3].tile_layer.transform.c = 0;
    app.ppu.layers[3].tile_layer.transform.d = (1 << gen16x_transform::base);


    


    app.ppu.layers[1].layer_type = GEN16X_LAYER_SPRITES;
    app.ppu.layers[1].vram_offset = offset;
    app.ppu.layers[1].blend_mode = GEN16X_BLENDMODE_NONE;
    //gen16x_layer_sprites& sprites_layer = *(gen16x_layer_sprites*)(app.ppu.vram + app.ppu.layers[1].vram_offset);
    unsigned char* sprite_tiles = app.ppu.vram + app.ppu.layers[1].vram_offset;

    memcpy(sprite_tiles, test_sprite3_tiles, sizeof(test_sprite3_tiles));
    offset += sizeof(test_sprite3_tiles);
    sprite_tiles += sizeof(test_sprite3_tiles);
    memcpy(sprite_tiles, slash_anim_tiles, sizeof(slash_anim_tiles));
    offset += sizeof(slash_anim_tiles);

    //memcpy(sprite_tiles + sizeof(test_sprite3_tiles), test_sprite2_tiles, sizeof(test_sprite2_tiles));
    //offset += sizeof(test_sprite2_tiles);

    for (int i = 0; i < 15; i++) {
        app.ppu.cgram32[220 + i].color_i = test_sprite3_palette[i];
    }


    for (int i = 0; i < 6; i++) {
        app.ppu.cgram32[220 + 15 + i].color_i = slash_anim_palette[i];
    }

    /*for (int i = 0; i < 11; i++) {
    app.ppu.cgram32[235 + i].color_i = test_sprite2_palette[i];
    }*/

    app.ppu.layers[1].sprite_layer.sprites[0].palette_offset = 220;
    app.ppu.layers[1].sprite_layer.sprites[0].flags = GEN16X_FLAG_SPRITE_ENABLED;
    app.ppu.layers[1].sprite_layer.sprites[0].size = GEN16X_MAKE_SPRITE_SIZE(5, 5);
    app.ppu.layers[1].sprite_layer.sprites[0].tile_index = 0;
    app.ppu.layers[1].sprite_layer.sprites[0].x = app.ppu.screen_width/2 - 16;
    app.ppu.layers[1].sprite_layer.sprites[0].y = 80;
    app.ppu.layers[1].sprite_layer.sprites[0].scale_x = 256;
    app.ppu.layers[1].sprite_layer.sprites[0].scale_y = 256;



    app.ppu.layers[1].sprite_layer.sprites[1].palette_offset = 220 + 15;
    app.ppu.layers[1].sprite_layer.sprites[1].flags = 0;
    app.ppu.layers[1].sprite_layer.sprites[1].size = GEN16X_MAKE_SPRITE_SIZE(6, 6);
    app.ppu.layers[1].sprite_layer.sprites[1].tile_index = sizeof(test_sprite3_tiles)/32;
    app.ppu.layers[1].sprite_layer.sprites[1].x = app.ppu.screen_width/2 - 32;
    app.ppu.layers[1].sprite_layer.sprites[1].y = 80 - 16;
    app.ppu.layers[1].sprite_layer.sprites[1].scale_x = 256;
    app.ppu.layers[1].sprite_layer.sprites[1].scale_y = 256;



    
    app.ppu.layers[4].layer_type = GEN16X_LAYER_TILES;
    app.ppu.layers[4].vram_offset = offset;
    
    unsigned char* tile_layer4 = app.ppu.vram + app.ppu.layers[4].vram_offset;
    /*
    for (int i = 0; i < 128; i++) {
        uint64_t tmp = font_8x8[i];
        uint64_t mask = 0x8000000000000000L;
        for (int col = 0; col < 64; col++) {
            unsigned char b = (tmp & (mask >> col)) ? '8' : 0;
            int index = (i << 6) | (col);
            tile_layer4[index] = b;
        }
    }*/

    memcpy(tile_layer4, test_font_tiles, sizeof(test_font_tiles));
    offset += sizeof(test_font_tiles);

    char test_text[] = "0123456789ABCDEFabcdef";
    app.ppu.layers[4].tile_layer.tilemap_vram_offset = offset;
    unsigned char* tilemap_layer4 = (app.ppu.vram + app.ppu.layers[4].tile_layer.tilemap_vram_offset);

    memcpy(tilemap_layer4, test_text, sizeof(test_text));
    
    offset += 32;

    app.ppu.layers[4].tile_layer.tile_size = GEN16X_TILE8;
    app.ppu.layers[4].tile_layer.flags = 0;
    app.ppu.layers[4].tile_layer.tilemap_width = 5;
    app.ppu.layers[4].tile_layer.tilemap_height = 0;
    app.ppu.layers[4].tile_layer.palette_offset = 220 + 15 + 6;
    app.ppu.layers[4].tile_layer.transform.x = -10;
    app.ppu.layers[4].tile_layer.transform.y = -8;

    memcpy(app.ppu.cgram32 + app.ppu.layers[4].tile_layer.palette_offset, test_font_palette, sizeof(test_font_palette));


    auto row_callback = (gen16x_row_callback_t)[](gen16x_ppu* ppu, unsigned int y) {
        if (y > 16) {
            app.ppu.layers[3].layer_type = GEN16X_LAYER_NONE;
        }
        static int z = 0;
        z += 1;
        
        //gen16x_layer_direct& layer_direct = *(gen16x_layer_direct*)(app.ppu.vram + app.ppu.layers[0].vram_offset);
        
        //gen16x_layer_sprites& sprites_layer = *(gen16x_layer_sprites*)(app.ppu.vram + app.ppu.layers[3].vram_offset);

        app.ppu.layers[1].sprite_layer.sprites[1].x = 100 + sinf(app.current_time*1.31f + y/4.0f)*2;
        app.ppu.layers[1].sprite_layer.sprites[1].y = 100 + cosf(app.current_time*2.2f + 0.25f*sinf(y/4.0f))*10 + y*0.25f;
        //layer_direct.scroll_x = 2*sinf(app.current_time*1.31 + y/4.0f)*2;
        //layer_direct.scroll_y = 10 + sinf(app.current_time*1.3 + y/4.0f);
        int h = y - 80;
        
        if (h > 0) {
            
            app.ppu.layers[0].layer_type = GEN16X_LAYER_NONE;
            app.ppu.layers[1].layer_type = GEN16X_LAYER_TILES;
            float ay = player.height + 16;
            
            float lambda = ay / (float(h + 4.0f));
            
            
            float p_cos = cosf(player.rot*3.1415926f/180.0f);
            float p_sin = sinf(player.rot*3.1415926f/180.0f);
            
            
            float f_x = p_sin;
            float f_y = p_cos;
            
            
            app.ppu.layers[1].tile_layer.transform.a = (int)(((1 << app.ppu.layers[1].tile_layer.transform.base)* 1.0f *  p_cos)*lambda);
            app.ppu.layers[1].tile_layer.transform.b = (int)(((1 << app.ppu.layers[1].tile_layer.transform.base)* 1.0f *  p_sin)*lambda);
            app.ppu.layers[1].tile_layer.transform.c = (int)(((1 << app.ppu.layers[1].tile_layer.transform.base)* 1.0f * -p_sin)*lambda);
            app.ppu.layers[1].tile_layer.transform.d = (int)(((1 << app.ppu.layers[1].tile_layer.transform.base)* 1.0f *  p_cos)*lambda);
            
            app.ppu.layers[1].tile_layer.transform.x = (int)(player.pos.x - 0 * f_x*lambda);
            app.ppu.layers[1].tile_layer.transform.y = (int)(player.pos.y - 0 * f_y*lambda);
            app.ppu.layers[1].tile_layer.transform.cx = (int)(player.pos.x - 0 * f_x*lambda + app.ppu.screen_width / 2);
            app.ppu.layers[1].tile_layer.transform.cy = (int)(player.pos.y - 0*f_y*lambda + app.ppu.screen_height - 16);

            


            
        }
    };
    app.ppu.framebuffer_offset = offset;

    //offset += app.ppu.screen_height*app.ppu.screen_width*4;
    
    printf("Initialized %d bytes of PPU vram\n", (int)sizeof(app.ppu.vram));
    printf("Initialized %d bytes of PPU cgram\n", (int)sizeof(app.ppu.cgram32));
    printf("Initialized %d bytes of PPU layer registers\n", (int)sizeof(app.ppu.layers));
    printf("Utilizing %d bytes of PPU vram\n", offset);
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

void init_spu() {
    memset(&app.spu, 0, sizeof(app.spu));


    app.spu.l_volume = GEN16X_DSP_MAX_VOLUME/4;
    app.spu.r_volume = GEN16X_DSP_MAX_VOLUME/4;
    app.spu.sample_rate = SAMPLE_RATE;

    app.spu.channels[0].enabled = 1;
    app.spu.channels[0].l_volume = GEN16X_DSP_MAX_VOLUME;
    app.spu.channels[0].r_volume = GEN16X_DSP_MAX_VOLUME;

    app.spu.channels[0].pitch = 0;
    app.spu.channels[0].gain_rate = 64;
    app.spu.channels[0].gain_type = GEN16X_DSP_GAIN_NONE;

    app.spu.channels[0].oscillator_type = GEN16X_DSP_OSC_SINE;
    app.spu.channels[0].oscillator_note = 69;
    app.spu.channels[0].oscillator_amplitude = GEN16X_DSP_MAX_VOLUME/4;

    app.spu.channels[0].current_gain_value = 0;
    app.spu.channels[0].voice_offset = 0;
    app.spu.channels[0].voice_loop = 1;
    app.spu.channels[0].voice_samples = SAMPLE_RATE/2;

    app.spu.output_samples = 2048;
    app.spu.output_offset = (sizeof(app.spu.sram)/2) - app.spu.output_samples*2;
    

    short* voice = (short*)app.spu.sram;
    for (int i = 0; i < app.spu.channels[0].voice_samples; i++) {
        float fade_in = i / 8000.0f;
        fade_in = fade_in > 1.0 ? 1.0 : fade_in;

        float fade_out = (app.spu.channels[0].voice_samples - i) / 8000.0f;
        fade_out = fade_out > 1.0 ? 1.0 : fade_in;

        short signal = 0.0*sinf(440.0f*(2*M_PI)*float(i)/SAMPLE_RATE);
        voice[(i * 2)] = signal;
        voice[(i * 2) + 1] = signal;

    }
}


void audio_callback(void *user_data, Uint8 *raw_buffer, int bytes) {
    Sint16 *buffer = (Sint16*)raw_buffer;
    int length = bytes / 2; // 2 bytes per sample for AUDIO_S16SYS
    int &sample_nr(*(int*)user_data);
    short* output = app.spu.sram + app.spu.output_offset;

    for (int i = 0; i < length; i++, sample_nr++) {
        gen16x_spu_tick(&app.spu);
        buffer[i] = (output[0] + output[1]) / 2;
        app.spu.flushed = true;
    }
    //printf("Flushed sound: %d\n", app.spu.channels[0].current_gain_value);

}


bool init_sdl() {
    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return 1;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    
    
    app.window = SDL_CreateWindow("gen16x", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, app.ppu.screen_width, app.ppu.screen_height, SDL_WINDOW_OPENGL|SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_RESIZABLE);
    
    //SDL_SetWindowResizable(g_window, SDL_TRUE);
    app.context = SDL_GL_CreateContext(app.window);
    SDL_GL_SetSwapInterval(0);
    
    //SDL_SetWindowFullscreen(g_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    
    SDL_AddEventWatch(watch, NULL);
    app.surface = SDL_CreateRGBSurfaceWithFormat(0, app.ppu.screen_width, app.ppu.screen_height, 32, SDL_PIXELFORMAT_BGRA32);
    
    
    
    
    
    
    
    
    app.audio_want.freq = SAMPLE_RATE; // number of samples per second
    app.audio_want.format = AUDIO_S16SYS; // sample type (here: signed short i.e. 16 bit)
    app.audio_want.channels = 1; // only one channel
    app.audio_want.samples = SAMPLE_RATE/16; // buffer-size
    app.audio_want.callback = audio_callback; // function SDL calls periodically to refill the buffer
    app.audio_want.userdata = &app.audio_sample_nr; // counter, keeping track of current sample number
    
    if (SDL_OpenAudio(&app.audio_want, NULL) != 0) {
        printf("Failed to open audio: %s", SDL_GetError());
    }
    /*if (app.audio_want.format != app.audio_have.format) {
        printf("Failed to get the desired AudioSpec");
    }*/

    
    return true;
}


void cleanup_sdl() {
    
    SDL_CloseAudio();
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
    
    if (!GLEW_VERSION_3_1) {
        printf("Error: OpenGL 3.1 not supported.\n");
        return false;
    }
    glGetError();
    printf("Status: Using OpenGL %s\n", glGetString(GL_VERSION));
    
    
    
    glGenTextures(1, &app.framebuffer_texture);
    glBindTexture(GL_TEXTURE_2D, app.framebuffer_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, app.ppu.screen_width,
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
        printf("OpenGL Error 1 - %d\n", (int)err);
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
    app.texture_uni = glGetUniformLocation(app.shader_prog, "Texture");
    app.display_size_uni = glGetUniformLocation(app.shader_prog, "display_size");
    app.texture_size_uni = glGetUniformLocation(app.shader_prog, "texture_size");
    
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, app.framebuffer_texture);
    
    err = glGetError();
    if (err) {
        printf("OpenGL Error 2 - %d\n", (int)err);
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


void render_sdl() {

    int d_w, d_h;
    int w_w, w_h;

    SDL_GetWindowSize(app.window, &w_w, &w_h);
    
    SDL_Surface* window_surface = SDL_GetWindowSurface(app.window);
    
    d_w = window_surface->w;
    d_h = window_surface->h;
    
    float scale_w = float(d_w) / float(app.ppu.screen_width);
    float scale_h = float(d_h) / float(app.ppu.screen_height);

    int orig_w = d_w;
    int orig_h = d_h;

    float scale = (scale_w < scale_h) ? scale_w : scale_h;

    d_w = (int)((float)app.ppu.screen_width*scale / 2) * 2;
    d_h = (int)((float)app.ppu.screen_height*scale / 2) * 2;

    if (d_w < app.ppu.screen_width || d_w < app.ppu.screen_height) {
        d_w = (float)app.ppu.screen_width;
        d_h = (float)app.ppu.screen_height;
    }


    int offset_w = (orig_w - d_w) / 2;
    int offset_h = (orig_h - d_h) / 2;
    

    
    float ratio_x = float(app.ppu.screen_width)/float(d_w);
    float ratio_y = float(app.ppu.screen_height)/float(d_h);
    SDL_LockSurface(window_surface);
    int prev_y0 = -1;
    for (int y = 0; y < window_surface->h; ++y) {
        unsigned int* dst_row = (unsigned int*)window_surface->pixels + y * window_surface->pitch / 4;
        
        if (y < offset_h || y - offset_h >= d_h) {
            memset(dst_row, 0, window_surface->pitch);
            continue;
        }
        int y0 = ratio_y * (y - offset_h);
        unsigned int* src_row = (unsigned int*)(app.ppu.vram + app.ppu.framebuffer_offset) + y0 * app.ppu.screen_width;

        if (prev_y0 == y0) {
            memcpy(dst_row, dst_row - window_surface->pitch / 4, window_surface->pitch);
            continue;
        }
        unsigned int prev_pixel = 0;
        prev_y0 = y0;
        int prev_x0 = -1;
        for (int x = 0; x < window_surface->w; ++x) {
            if (x < offset_w || x - offset_w >= d_w) {
                dst_row[x] = 0;
                continue;
            }

            int x0 = ratio_x * (x - offset_w);
            if (prev_x0 == x0) {
                dst_row[x] = prev_pixel;
                continue;
            }
            prev_x0 = x0;
            prev_pixel = src_row[x0];
            dst_row[x] = prev_pixel;
        }
    }
    SDL_UnlockSurface(window_surface);
    SDL_UpdateWindowSurface(app.window);
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
    static bool keydown[256] = {false};
    
    const Uint8 *kbstate = SDL_GetKeyboardState(NULL);
    
    float speed_scale = 20.0f;

    vec2 forward = vec2(-sinf(3.1415926f*player.rot/180.0f), cosf(3.1415926f*player.rot/180.0f));

    if (kbstate[SDL_SCANCODE_W]) {
        
        
        player.pos += speed_scale*10.0f*app.delta_time*forward;
        
    }
    
    if (kbstate[SDL_SCANCODE_S]) {

        
        player.pos += 5.0f*speed_scale*app.delta_time*-forward;
        
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
    
    if (kbstate[SDL_SCANCODE_R]) {
        player.height += 4.0f*speed_scale*app.delta_time;
    }
    if (kbstate[SDL_SCANCODE_N]) {
        if (!keydown[SDL_SCANCODE_N]) {
            keydown[SDL_SCANCODE_N] = true;
            app.spu.channels[0].voice_playing = 1;
            app.spu.channels[0].gain_target = GEN16X_DSP_MAX_VOLUME;
            app.spu.channels[0].gain_rate = 128;
            //app.spu.channels[0].voice_time = 0;
            //app.spu.channels[0].current_gain_value = 0;
            app.spu.channels[0].gain_type = GEN16X_DSP_GAIN_EXPONENTIAL;
            printf("Keydown\n");
        }
    } else if (keydown[SDL_SCANCODE_N]) {
        keydown[SDL_SCANCODE_N] = false;
        app.spu.channels[0].gain_target = 0;
        app.spu.channels[0].gain_rate = 1;
        app.spu.channels[0].gain_type = GEN16X_DSP_GAIN_EXPONENTIAL;
        //app.spu.channels[0].stop = 1;
        printf("Keyup\n");
    }
    
    if (kbstate[SDL_SCANCODE_M]) {
        if (!keydown[SDL_SCANCODE_M]) {
            keydown[SDL_SCANCODE_M] = true;
            app.spu.channels[0].pitch = (((523.25f - 440.0f)*GEN16X_DSP_BASE_PITCH)/440.0f);
            printf("Keydown\n");
        }
    } else if (keydown[SDL_SCANCODE_M]) {
        keydown[SDL_SCANCODE_M] = false;
        app.spu.channels[0].pitch = 0;
        printf("Keyup\n");
    }


    player.moving = false;
    player.move.x = 0;
    player.move.y = 0;
    if (kbstate[SDL_SCANCODE_UP]) {
        //player.pos_y += 5.0f*speed_scale*app.delta_time;
        player.rot = 0;
        player.moving = true;
        player.move.y += -1;
    }
    if (kbstate[SDL_SCANCODE_DOWN]) {
        //player.pos_y -= 5.0f*speed_scale*app.delta_time;
        player.rot = 180;
        player.moving = true;
        player.move.y += 1;
    }

    if (kbstate[SDL_SCANCODE_LEFT]) {
        //player.pos_x -= 5.0f*speed_scale*app.delta_time;
        player.rot = 270;
        player.move.x += -1;
        player.moving = true;
    }
    
    if (kbstate[SDL_SCANCODE_RIGHT]) {
        //player.pos_x += 5.0f*speed_scale*app.delta_time;
        player.move.x += 1;
        player.rot = 90;
        player.moving = true;
    }


    if (kbstate[SDL_SCANCODE_C]) {
        if (!keydown[SDL_SCANCODE_C]) {
            keydown[SDL_SCANCODE_C] = true;
            
            player.state = 2;
            if (player.state) {
                player.moving = false;
            }


        }
        
    } else if (keydown[SDL_SCANCODE_C]) {
        keydown[SDL_SCANCODE_C] = false;
    }

  
}



int main() {

    player.pos = vec2(350.0f);
    player.height = 0;
    player.rot = 0;
    
    init_ppu();
    init_spu();
    
    init_sdl();
    
    
    
    SDL_PauseAudio(0);
    
    app.opengl_enabled = init_opengl();
    
    
    int prev_pos_x = 0;
    int prev_pos_y = 0;
    while (!app.quitting) {
        
        handle_sdl_events();
        handle_sdl_input();
        
        //gen16x_layer_direct& direct_layer = *(gen16x_layer_direct*)(app.ppu.vram + app.ppu.layers[0].vram_offset);
        
        //app.ppu.layers[0].direct_layer.scroll_y = 0;
        //app.ppu.layers[0].direct_layer.scroll_x = (short)((-player.rot/120.0f)*app.ppu.screen_width);
        //app.ppu.layers[0].direct_layer.flags |= GEN16X_FLAG_REPEAT_X;
        
        
        //app.ppu.layers[0].layer_type = GEN16X_LAYER_DIRECT;
        //app.ppu.layers[1].layer_type = GEN16X_LAYER_NONE;
        //app.ppu.layers[3].layer_type = GEN16X_LAYER_TILES;


        //gen16x_layer_sprites& sprites_layer = *(gen16x_layer_sprites*)(app.ppu.vram + app.ppu.layers[3].vram_offset);

        static int frame_no = 0;
        static double attack_time = 0;
        frame_no++;
        BoxCollider player_aabb;

        player_aabb.origin = player.pos - vec2(4);
        player_aabb.size = vec2(8);




        if (player.state != 2) {


            if (player.moving) {
                //float forward_x = -sinf(3.1415926f*player.rot/180.0f);
                //float forward_y = cosf(3.1415926f*player.rot/180.0f);
                //player.forward_x = -0.5;
                //player.forward_y = 0.5;
                
                float m = mag(player.move);
                normalize(player.move);


                if (player.move.y < -0.5) {
                    player.dir = 2;

                }

                if (player.move.y > 0.5) {
                    player.dir = 0;
                }


                if (player.move.x < -0.5) {
                    player.dir = 3;
                }

                if (player.move.x > 0.5) {
                    player.dir = 1;
                }

                for (int c = 0; c < 3; c++) {
                
                    bool any_collision = false;
                    bool was_collision = false;
                
                

                    float fm = mag(player.forward);
                    vec2 mv = player.move;
                    if (fm > 0) {
                        mv += player.forward;
                        normalize(mv);
                    }

                    vec2 player_dxdy = 20*6.0f*(mv)*app.delta_time/3.0f;

                    player_aabb.origin = player.pos - vec2(4) + player_dxdy;
                    player_aabb.size = vec2(8);

                    int n_collisions = 0;
                    vec2 avg = 0;
                    vec2 avg_n = 0;
                    float closest_d = 999999999;
                    int num_polyons = sizeof(test_map_objects2)/sizeof(test_map_objects2[0]);
                    for (int i = 0; i < num_polyons; i++) {
                        ConvexCollider cc;
                        cc.origin.x = test_map_objects2[i][0];
                        cc.origin.y = test_map_objects2[i][1];
                        cc.num_edges = test_map_objects2[i][2];
                        for (int j = 0; j < 12; j++) {
                            cc.edges[j].x = test_map_objects2[i][3 + j*2];
                            cc.edges[j].y = test_map_objects2[i][3 + j*2 + 1];
                        }

                        if (collision_box_convex(player_aabb, cc)) {
                            vec2 c;
                            int ce = closest_point_to_convex(player.pos + player_dxdy, cc, &c);
                            float d = distance(player.pos + player_dxdy, c);
                            if (d < 4) {
                                n_collisions++;
                                avg += c;
                                vec2 n = player.pos - c;// (cc.edges[(ce + 1)%cc.num_edges][1] - cc.edges[ce][1]);
                                normalize(n);
                                avg_n += n;
                                //printf("Y Collision: %f %f!\n", closest_nx, closest_ny);
                                
                                was_collision = true;
                                any_collision = true;

                            }
                        
                        }

                    }
                    if (was_collision) {
                        player_dxdy = vec2(0);
                    }
               
                    player.pos += player_dxdy;
                
                    if (any_collision) {
                        normalize(avg_n);
                        player.forward = avg_n;//(closest_ny*player.move_y + -closest_nx*player.move_x);
                    
                        break;
                    } else {
                        player.forward = vec2(0);
                    }
                


                }
                //printf("Playerpos: %f, %f\n", player.pos_x, player.pos_y);
                player.state = 1;
            } else {
                player.state = 0;

            }

        }



        app.ppu.layers[1].sprite_layer.sprites[0].priority = 0;
        app.ppu.layers[1].sprite_layer.sprites[1].priority = 1;

        player_aabb.origin = player.pos - vec2(8, 16);
        player_aabb.size = vec2(16);

        for (int i = 0; i < sizeof(test_map_objects3)/sizeof(test_map_objects3[0]); i++) {
            ConvexCollider cc;
            cc.origin.x = test_map_objects3[i][0];
            cc.origin.y = test_map_objects3[i][1];
            cc.num_edges = test_map_objects3[i][2];
            for (int j = 0; j < 12; j++) {
                cc.edges[j].x = test_map_objects3[i][3 + j*2];
                cc.edges[j].y = test_map_objects3[i][3 + j*2 + 1];
            }

            if (collision_box_convex(player_aabb, cc)) {
                app.ppu.layers[1].sprite_layer.sprites[0].priority = 1;
                
                break;
            }

        }



        app.ppu.layers[1].sprite_layer.sprites[0].x =  app.ppu.screen_width/2 - 16;

        if (player.state == 0) {
            app.ppu.layers[1].sprite_layer.sprites[0].tile_index = 16*player.dir;
        } else if (player.state == 1) {
            app.ppu.layers[1].sprite_layer.sprites[0].tile_index = 16*(6*(1 + player.dir) + int(app.current_time*12.0)%6);
        } else if (player.state == 2) {
            
            int attack_frame = int(attack_time*20.0);

            if (attack_frame >= 3) {
                player.state = 0;
                attack_time = 0;
                app.ppu.layers[1].sprite_layer.sprites[1].flags = 0;
            } else {
                app.ppu.layers[1].sprite_layer.sprites[0].tile_index = 16*(30 + attack_frame);
                
                int offsets[] = {-7, 6 + 4, 6};

                app.ppu.layers[1].sprite_layer.sprites[0].x = offsets[attack_frame] + app.ppu.screen_width/2 - 16;
                app.ppu.layers[1].sprite_layer.sprites[1].tile_index = 64*(3*player.dir + attack_frame) + sizeof(test_sprite3_tiles)/32;
                app.ppu.layers[1].sprite_layer.sprites[1].flags = GEN16X_FLAG_SPRITE_ENABLED;

                //printf("Attack Frame: %d\n", int(attack_time*15.0));
                attack_time += app.delta_time;
            }
            

        } 
        


        //float ay = player.height + 16;        
        //int sprite_x = 0*sin(frame_no/500.0f)*16.0f;
        //int sprite_y = 0;
        //int sprite_z = 0;
        //float p_cos = cosf(player.rot*3.1415926f/180.0f);
        //float p_sin = sinf(player.rot*3.1415926f/180.0f);
        //float cam_right_x = p_cos;
        //float cam_right_y = p_sin;
        //float cam_right_z = 0;
        //float cam_forward_x = -p_sin;
        //float cam_forward_y = p_cos;
        //float cam_forward_z = 0;
        //float cam_up_x = 0;
        //float cam_up_y = 0;
        //float cam_up_z = 1;
        //float lambda = ay / (float(sprite_z + 4.0f));
        //float sprite_d = (player.pos_x - sprite_x)*(player.pos_x - sprite_x) + (player.pos_y - sprite_y)*(player.pos_y - sprite_y);
        //float sprite_p = (cam_forward_x*(player.pos_x - sprite_x)*(player.pos_x - sprite_x) + cam_forward_y*(player.pos_y-sprite_y)*(player.pos_y-sprite_y));
        //float sprite_s_x =  cam_right_x*(sprite_x - player.pos_x) + cam_forward_x*(sprite_y - player.pos_y) + cam_up_x*(sprite_z - player.height) + player.pos_x;// (cam_right_x*(sprite_x - player.pos_x) + cam_right_y*(sprite_y - player.pos_y) + cam_forward_x*(sprite_x - player.pos_x) + cam_forward_y*(sprite_y - player.pos_y));// +cam_up_x*(sprite_z - player.height);
        //float sprite_s_y =  cam_right_y*(sprite_x - player.pos_x) + cam_forward_y*(sprite_y - player.pos_y) + cam_up_y*(sprite_z - player.height) + player.pos_y;// +cam_up_x*(sprite_z - player.height);
        //float sprite_s_z =  cam_right_x*(sprite_x - player.pos_x) + cam_forward_x*(sprite_y - player.pos_y) + cam_up_x*(sprite_z - player.height);
        //float scale_factor = sprite_p;// 16 + sprite_d;
        //app.ppu.layers[3].sprite_layer.sprites[2].scale_x = app.ppu.layers[3].sprite_layer.sprites[2].scale_y = scale_factor;
        //app.ppu.layers[3].sprite_layer.sprites[2].y = 80 - (16*32)/(scale_factor);// app.ppu.screen_height/2;
        //app.ppu.layers[3].sprite_layer.sprites[2].x = (app.ppu.screen_width/2)*sprite_s_x/(sprite_p/16) + app.ppu.screen_width/2  - (16*256)/(scale_factor);
        

        app.ppu.layers[0].tile_layer.transform.x = (int)player.pos.x - app.ppu.screen_width/2;
        app.ppu.layers[0].tile_layer.transform.y = (int)player.pos.y - app.ppu.screen_height/2;

       
        app.ppu.layers[3].tile_layer.transform.x = app.ppu.layers[0].tile_layer.transform.x;
        app.ppu.layers[3].tile_layer.transform.y = app.ppu.layers[0].tile_layer.transform.y;

        //app.ppu.layers[1].sprite_layer.sprites[1].tile_index = 64*((frame_no/3)%(12)) + sizeof(test_sprite3_tiles)/32;
        //unsigned char sprite_flags = app.ppu.layers[3].sprite_layer.sprites[2].flags;

        //app.ppu.layers[3].sprite_layer.sprites[2].flags = (sprite_p > 0) ? GEN16X_FLAG_SPRITE_ENABLED | sprite_flags : ~GEN16X_FLAG_SPRITE_ENABLED & sprite_flags;
        gen16x_ppu_render(&app.ppu);
       
        if (app.opengl_enabled) {
            render_opengl();
        } else {
            render_sdl();
        }
        
        app.current_time += app.delta_time;

        if (app.timer.elapsed() > 1.0 && app.frame_no > 0.0) {
            char fps_text[32];
            app.delta_time = (float)(app.timer.elapsed() / app.frame_no);
            sprintf(fps_text, "FPS: %0.2f %0.2f ms\n", app.frame_no / app.timer.elapsed(), 1000.0f*app.timer.elapsed()/app.frame_no);
            printf("%s", fps_text);
            unsigned char* tilemap_layer4 = (app.ppu.vram + app.ppu.layers[4].tile_layer.tilemap_vram_offset);
            strncpy((char*)tilemap_layer4, fps_text, 32);
            
            
            app.frame_no = 0.0;
            app.timer.reset();
            
            
        }
        app.frame_no++;
        if (app.opengl_enabled) {
            SDL_GL_SwapWindow(app.window);
        }
        
    }
    
    if (app.opengl_enabled) {
        cleanup_opengl();
    }
    SDL_PauseAudio(1);
    cleanup_sdl();
    
    SDL_Quit();
    
    return 0;
}
