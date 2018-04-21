#pragma once
#define SHADER_TEXT(a) #a
const char * g_quad_vsh = R"(
    #version 330 core

    in vec2 position;
    out vec2 texcoord;
    uniform vec2 display_size;
    uniform vec2 texture_size;

    void main() {
        gl_Position = vec4(position, 0.0, 1.0);
        vec2 pos2 = position;

        texcoord = vec2(pos2.x* 0.5 + 0.5, 0.5 - 0.5*pos2.y);
    }

)";


const char * g_quad_psh = R"(
    #version 330 core
    uniform sampler2D texture2;
    uniform sampler2D texture;
    uniform vec2 display_size;
    uniform vec2 texture_size;

    in  vec4 gl_FragCoord;
    in  vec2 texcoord;
    out vec4 frag_color;

    void main() {

        vec2 vuv = texcoord * texture_size;
        vec2 alpha = 0.7*texture_size/display_size;
        vec2 x = fract(vuv);
        vec2 x_ = clamp((0.5/alpha) * x, 0, 0.5) + clamp((0.5 / alpha) * (x - 1.0) + 0.5, 0, 0.5);
        
        vec2 texcoord2 = (floor(vuv) + x_) / texture_size;
        

        vec4 color = textureLod(texture, texcoord2, 0);
        

        frag_color = vec4(color.rgb, 1.0);
    }

)";
