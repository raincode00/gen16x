#include <cstdlib>
#include <string.h>
#include <cstdio>
#define _USE_MATH_DEFINES
#include <cmath>
#include <chrono>

#define ENABLE_SDL

#ifdef ENABLE_SDL
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_audio.h>
#include <GL/glew.h>

#endif

#include "gen16x.h"
#include "physics.h"
#include "gamestate.h"
#include "scene.h"

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
    GameState gs;
    gen16x_spu spu;


    Timer timer;

    #ifdef ENABLE_SDL
    SDL_Window* window;
    SDL_Surface* surface;
    SDL_GLContext context;


    SDL_AudioSpec audio_want;
    SDL_AudioSpec audio_have;

    #endif

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

static application_state* g_app;



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
    auto& app = *g_app;
    memset(&app.gs.ppu, 0, sizeof(app.gs.ppu));

    
    app.gs.ppu.screen_height = 216;
    app.gs.ppu.screen_width = 384;
    //app.gs.ppu.framebuffer_offset = sizeof(app.gs.ppu.vram) - (app.gs.ppu.screen_height*app.gs.ppu.screen_width*4);

    printf("Initialized %d bytes of PPU vram\n", (int)sizeof(app.gs.ppu.vram));
    printf("Initialized %d bytes of PPU cgram\n", (int)sizeof(app.gs.ppu.cgram32));
    printf("Initialized %d bytes of PPU layer registers\n", (int)sizeof(app.gs.ppu.layers));
}

#ifdef ENABLE_SDL

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
    auto& app = *g_app;
    if (event->type == SDL_APP_WILLENTERBACKGROUND) {
        app.quitting = true;
    }
    
    return 1;
}
#endif

void init_spu() {
    auto& app = *g_app;
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

#ifdef ENABLE_SDL
void audio_callback(void *user_data, Uint8 *raw_buffer, int bytes) {
    auto& app = *g_app;
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
    auto& app = *g_app;
    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return 1;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    
    
    app.window = SDL_CreateWindow("gen16x", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, app.gs.ppu.screen_width, app.gs.ppu.screen_height, SDL_WINDOW_OPENGL|SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_RESIZABLE);
    
    //SDL_SetWindowResizable(g_window, SDL_TRUE);
    app.context = SDL_GL_CreateContext(app.window);
    SDL_GL_SetSwapInterval(0);
    
    //SDL_SetWindowFullscreen(g_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    
    SDL_AddEventWatch(watch, NULL);
    app.surface = SDL_CreateRGBSurfaceWithFormat(0, app.gs.ppu.screen_width, app.gs.ppu.screen_height, 32, SDL_PIXELFORMAT_BGRA32);
    
    
    
    
    
    
    
    
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
    auto& app = *g_app;
    SDL_CloseAudio();
    SDL_DelEventWatch(watch, NULL);
    SDL_GL_DeleteContext(app.context);
    SDL_DestroyWindow(app.window);
}

bool init_opengl() {
    auto& app = *g_app;
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, app.gs.ppu.screen_width, app.gs.ppu.screen_height,
        0, GL_BGRA, GL_UNSIGNED_BYTE, app.gs.ppu.frambuffer);
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
                       app.gs.ppu.screen_width, app.gs.ppu.screen_height);
    int draw_w;
    int draw_h;
    
    SDL_GL_GetDrawableSize(app.window, &draw_w, &draw_h);
    glProgramUniform2f(app.shader_prog, app.display_size_uni, (float)draw_w, (float)draw_h);
    glViewport(0, 0, draw_w, draw_h);
    return true;
}

void cleanup_opengl() {
    auto& app = *g_app;
    glDeleteTextures(1, &app.framebuffer_texture);
    glDeleteBuffers(1, &app.quad_vbo);
    glDeleteVertexArrays(1, &app.quad_va);
}

void render_opengl() {
    auto& app = *g_app;
    
    
    
    
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, app.gs.ppu.screen_width, app.gs.ppu.screen_height,
                    GL_BGRA, GL_UNSIGNED_BYTE, app.gs.ppu.frambuffer);
    
    
    int d_w, d_h;
    int w_w, w_h;
    
    SDL_GetWindowSize(app.window, &w_w, &w_h);
    SDL_GL_GetDrawableSize(app.window, &d_w, &d_h);
    

    float scale_w = float(d_w) / float(app.gs.ppu.screen_width);
    float scale_h = float(d_h) / float(app.gs.ppu.screen_height);
    
    int orig_w = d_w;
    int orig_h = d_h;
    
    float scale = (scale_w < scale_h) ? scale_w : scale_h;
    
    d_w = (int)((float)app.gs.ppu.screen_width*scale/2)*2;
    d_h = (int)((float)app.gs.ppu.screen_height*scale/2)*2;
    
    if (d_w < app.gs.ppu.screen_width || d_w < app.gs.ppu.screen_height) {
        d_w = (float)app.gs.ppu.screen_width;
        d_h = (float)app.gs.ppu.screen_height;
    }
    
    
    int offset_w = (orig_w - d_w) / 2;
    int offset_h = (orig_h - d_h) / 2;
    
    glProgramUniform2f(app.shader_prog, app.display_size_uni, d_w, d_h);
    glViewport(offset_w, offset_h, d_w, d_h);
    
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}


void render_sdl() {
    auto& app = *g_app;
    int d_w, d_h;
    int w_w, w_h;

    SDL_GetWindowSize(app.window, &w_w, &w_h);
    
    SDL_Surface* window_surface = SDL_GetWindowSurface(app.window);
    
    d_w = window_surface->w;
    d_h = window_surface->h;
    
    float scale_w = float(d_w) / float(app.gs.ppu.screen_width);
    float scale_h = float(d_h) / float(app.gs.ppu.screen_height);

    int orig_w = d_w;
    int orig_h = d_h;

    float scale = (scale_w < scale_h) ? scale_w : scale_h;

    d_w = (int)((float)app.gs.ppu.screen_width*scale / 2) * 2;
    d_h = (int)((float)app.gs.ppu.screen_height*scale / 2) * 2;

    if (d_w < app.gs.ppu.screen_width || d_w < app.gs.ppu.screen_height) {
        d_w = (float)app.gs.ppu.screen_width;
        d_h = (float)app.gs.ppu.screen_height;
    }


    int offset_w = (orig_w - d_w) / 2;
    int offset_h = (orig_h - d_h) / 2;
    

    
    float ratio_x = float(app.gs.ppu.screen_width)/float(d_w);
    float ratio_y = float(app.gs.ppu.screen_height)/float(d_h);
    SDL_LockSurface(window_surface);
    int prev_y0 = -1;
    for (int y = 0; y < window_surface->h; ++y) {
        unsigned int* dst_row = (unsigned int*)window_surface->pixels + y * window_surface->pitch / 4;
        
        if (y < offset_h || y - offset_h >= d_h) {
            memset(dst_row, 0, window_surface->pitch);
            continue;
        }
        int y0 = ratio_y * (y - offset_h);
        unsigned int* src_row = (unsigned int*)(app.gs.ppu.frambuffer) + y0 * app.gs.ppu.screen_width;

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
    auto& app = *g_app;
    
    Uint32 flags = SDL_GetWindowFlags(app.window);
    if (!(flags & (SDL_WINDOW_FULLSCREEN|SDL_WINDOW_MAXIMIZED))) {
        
        // SDL_SetWindowSize(g_window, vp_width, vp_height);
    }
    //printf("Window resized to: %d x %d\n", vp_width, vp_height);
}

void handle_sdl_events() {
    auto& app = *g_app;
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
    auto& app = *g_app;
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
#endif


int main() {
    //g_app = new application_state();
    //auto& app = *g_app;

    static application_state app;
    g_app = &app;

    
    player.pos = vec2(350.0f);
    player.height = 0;
    player.rot = 0;
    
    init_ppu();

    memset(&app.gs.scene, 0, sizeof(app.gs.scene));
    
    app.gs.scene.view_width = app.gs.ppu.screen_width;
    app.gs.scene.view_height = app.gs.ppu.screen_height;


    app.gs.scene.world.tile_layers[1].enabled = true;
    app.gs.scene.world.tile_layers[1].tilemap_width = 6;
    app.gs.scene.world.tile_layers[1].tilemap_height = 6;
    app.gs.scene.world.tile_layers[1].rom_tileset_base = test_tileset_2_tiles;
    app.gs.scene.world.tile_layers[1].rom_tileset_size = sizeof(test_tileset_2_tiles);

    app.gs.scene.world.tile_layers[1].rom_tilemap_base = test_map_layer0_tilemap;
    app.gs.scene.world.tile_layers[1].rom_tilemap_size = sizeof(test_map_layer0_tilemap);

    app.gs.scene.world.tile_layers[1].rom_cgram_base = test_tileset_2_palette;
    app.gs.scene.world.tile_layers[1].rom_cgram_size = sizeof(test_tileset_2_palette);

    app.gs.scene.world.tile_layers[2].enabled = true;
    app.gs.scene.world.tile_layers[2].tilemap_width = 6;
    app.gs.scene.world.tile_layers[2].tilemap_height = 6;

    app.gs.scene.world.tile_layers[2].rom_tileset_base = test_tileset_2_tiles;
    app.gs.scene.world.tile_layers[2].rom_tileset_size = sizeof(test_tileset_2_tiles);

    app.gs.scene.world.tile_layers[2].rom_tilemap_base = test_map_layer1_tilemap;
    app.gs.scene.world.tile_layers[2].rom_tilemap_size = sizeof(test_map_layer1_tilemap);

    app.gs.scene.world.tile_layers[2].rom_cgram_base = test_tileset_2_palette;
    app.gs.scene.world.tile_layers[2].rom_cgram_size = sizeof(test_tileset_2_palette);



    char test_text[32] = {0};

    app.gs.scene.world.tile_layers[4].enabled = true;
    app.gs.scene.world.tile_layers[4].tilemap_width = 5;
    app.gs.scene.world.tile_layers[4].tilemap_height = 0;
    app.gs.scene.world.tile_layers[4].position.x = 4;
    app.gs.scene.world.tile_layers[4].position.y = 4;
    app.gs.scene.world.tile_layers[4].rom_tileset_base = test_font_tiles;
    app.gs.scene.world.tile_layers[4].rom_tileset_size = sizeof(test_font_tiles);
    app.gs.scene.world.tile_layers[4].rom_tilemap_base = test_text;
    app.gs.scene.world.tile_layers[4].rom_tilemap_size = sizeof(test_text);
    app.gs.scene.world.tile_layers[4].rom_cgram_base = test_font_palette;
    app.gs.scene.world.tile_layers[4].rom_cgram_size = sizeof(test_font_palette);




    app.gs.scene.num_sprites = 1;
    app.gs.scene.sprites[0].visible = true;
    app.gs.scene.sprites[0].current_frame = 0;
    app.gs.scene.sprites[0].rom_cgram_base = test_sprite3_palette;
    app.gs.scene.sprites[0].rom_cgram_size = sizeof(test_sprite3_palette);
    app.gs.scene.sprites[0].rom_sprite_base = test_sprite3_tiles;
    app.gs.scene.sprites[0].rom_sprite_size = sizeof(test_sprite3_tiles);
    app.gs.scene.sprites[0].size_w = 5;
    app.gs.scene.sprites[0].size_h = 5;
    app.gs.scene.sprites[0].scale = vec2(1,1);
    
    WorldEntity sprite_ent;
    sprite_ent.layer = 0;
    sprite_ent.id = 0;
    sprite_ent.type = WORLD_ENTITY_TYPE_SPRITE;

    
    int used_vram = 0;
    int used_cgram = 0;

    scene_load_ppu(&app.gs.scene, &app.gs.ppu, &used_vram, &used_cgram);
    

    printf("Initialized Scene utilizing %d bytes of PPU vram and %d bytes of cgram\n", used_vram, used_cgram);

    init_spu();
    
    app.gs.scene.world.grid_node_size = 4;
    app.gs.scene.world.grid_width = 64;
    app.gs.scene.world.grid_height = 64;
    
    

    #ifdef ENABLE_SDL
        init_sdl();
        app.opengl_enabled = init_opengl();


        SDL_PauseAudio(0);

    #endif
    
   


    
    int prev_pos_x = 0;
    int prev_pos_y = 0;
    while (!app.quitting) {
        
        #ifdef ENABLE_SDL
            handle_sdl_events();
            handle_sdl_input();
        #endif

        
        
        static int frame_no = 0;
        static double attack_time = 0;
        frame_no++;

        BoxCollider player_aabb;

        player_aabb.origin = player.pos - vec2(4);
        player_aabb.size = vec2(8);

        app.gs.scene.sprites[0].origin = vec2(16, 30);


        if (player.state != 2) {


            if (player.moving) {

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


        app.gs.scene.sprites[0].priority = 0;
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
                app.gs.scene.sprites[0].priority = 1;
                
                break;
            }

        }


        if (player.state == 0) {
            app.gs.scene.sprites[0].current_frame = player.dir;
        } else if (player.state == 1) {
            app.gs.scene.sprites[0].current_frame = (6*(1 + player.dir) + int(app.current_time*12.0)%6);

        } else if (player.state == 2) {
            
            int attack_frame = int(attack_time*10.0);

            if (attack_frame >= 3) {
                player.state = 0;
                attack_time = 0;
                //app.gs.ppu.layers[1].sprite_layer.sprites[1].flags = 0;
            } else {
                //app.gs.ppu.layers[1].sprite_layer.sprites[0].tile_index = 16*(30 + attack_frame);
                
                int offsets[] = {-7, 10, 6};
                app.gs.scene.sprites[0].current_frame = 30 + attack_frame;
                app.gs.scene.sprites[0].origin.x = 16 - offsets[attack_frame];
                //app.gs.ppu.layers[1].sprite_layer.sprites[0].x = offsets[attack_frame] + app.gs.ppu.screen_width/2 - 16;
                //app.gs.ppu.layers[1].sprite_layer.sprites[1].tile_index = 64*(3*player.dir + attack_frame) + sizeof(test_sprite3_tiles)/32;
                //app.gs.ppu.layers[1].sprite_layer.sprites[1].flags = GEN16X_FLAG_SPRITE_ENABLED;

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
        //app.gs.ppu.layers[3].sprite_layer.sprites[2].scale_x = app.gs.ppu.layers[3].sprite_layer.sprites[2].scale_y = scale_factor;
        //app.gs.ppu.layers[3].sprite_layer.sprites[2].y = 80 - (16*32)/(scale_factor);// app.gs.ppu.screen_height/2;
        //app.gs.ppu.layers[3].sprite_layer.sprites[2].x = (app.gs.ppu.screen_width/2)*sprite_s_x/(sprite_p/16) + app.gs.ppu.screen_width/2  - (16*256)/(scale_factor);
        


        //app.gs.ppu.layers[1].sprite_layer.sprites[1].tile_index = 64*((frame_no/3)%(12)) + sizeof(test_sprite3_tiles)/32;
        //unsigned char sprite_flags = app.gs.ppu.layers[3].sprite_layer.sprites[2].flags;

        //app.gs.ppu.layers[3].sprite_layer.sprites[2].flags = (sprite_p > 0) ? GEN16X_FLAG_SPRITE_ENABLED | sprite_flags : ~GEN16X_FLAG_SPRITE_ENABLED & sprite_flags;


        app.gs.scene.camera_pos = player.pos;
        if (app.gs.scene.camera_pos.x < app.gs.scene.view_width/2) {
            app.gs.scene.camera_pos.x = app.gs.scene.view_width/2;
        }

        if (app.gs.scene.camera_pos.y < app.gs.scene.view_height/2) {
            app.gs.scene.camera_pos.y = app.gs.scene.view_height/2;
        }

        if (app.gs.scene.camera_pos.x > (1<<app.gs.scene.world.grid_node_size)*app.gs.scene.world.grid_width - app.gs.scene.view_width/2) {
            app.gs.scene.camera_pos.x = (1<<app.gs.scene.world.grid_node_size)*app.gs.scene.world.grid_width - app.gs.scene.view_width/2;
        }

        if (app.gs.scene.camera_pos.y > (1<<app.gs.scene.world.grid_node_size)*app.gs.scene.world.grid_height - app.gs.scene.view_height/2) {
            app.gs.scene.camera_pos.y = (1<<app.gs.scene.world.grid_node_size)*app.gs.scene.world.grid_height - app.gs.scene.view_height/2;
        }

        vec2 viewport_min = app.gs.scene.camera_pos - vec2(app.gs.scene.view_width, app.gs.scene.view_width)*0.5f;
        vec2 viewport_max = app.gs.scene.camera_pos + vec2(app.gs.scene.view_width, app.gs.scene.view_width)*0.5f;

        world_clear_entities(&app.gs.scene.world, viewport_min, viewport_max);

        app.gs.scene.sprites[0].pos = player.pos;
        
        world_insert_entity(&app.gs.scene.world, sprite_ent, player.pos - player_aabb.size/2.0f, player.pos + player_aabb.size/2.0f);

        
        
        
        gamestate_tick(&app.gs, app.delta_time);

        gen16x_ppu_render(&app.gs.ppu);
        #ifdef ENABLE_SDL
            if (app.opengl_enabled) {
                render_opengl();
            } else {
                render_sdl();
            }
        #endif
        
        
        app.current_time += app.delta_time;

        if (app.timer.elapsed() > 1.0 && app.frame_no > 0.0) {
            char fps_text[32];
            app.delta_time = (float)(app.timer.elapsed() / app.frame_no);
            sprintf(fps_text, "FPS: %0.2f %0.2f ms\n", app.frame_no / app.timer.elapsed(), 1000.0f*app.timer.elapsed()/app.frame_no);
            printf("%s", fps_text);
            unsigned char* tilemap_layer4 = (app.gs.ppu.vram + app.gs.ppu.layers[app.gs.scene.world.tile_layers[4].ppu_layer].tile_layer.tilemap_vram_offset);
            strncpy((char*)tilemap_layer4, fps_text, 32);
            
            app.frame_no = 0.0;
            app.timer.reset();
        }
        app.frame_no++;
        #ifdef ENABLE_SDL
        if (app.opengl_enabled) {
            SDL_GL_SwapWindow(app.window);
        }
        #endif
    }
    #ifdef ENABLE_SDL
        if (app.opengl_enabled) {
            cleanup_opengl();
        }
        SDL_PauseAudio(1);
        cleanup_sdl();

        SDL_Quit();
    #endif
    return 0;
}
