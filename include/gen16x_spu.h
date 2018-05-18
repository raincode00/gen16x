/*
    gen16x
    Copyright (C) 2018 Jahrain Jackson

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
 */

#pragma once

#include "gen16x_defs.h"


struct gen16x_dsp_channel {

    unsigned char enabled;
    unsigned char mute;

    short l_volume;
    short r_volume;
    short pitch;

    unsigned int time;

    unsigned char reverb_enable;
    short reverb_delay;
    short reverb_count;
    short reverb_l_volume;
    short reverb_r_volume;
    unsigned int reverb_offset;
    unsigned int reverb_size;


    unsigned char gain_type;
    short gain_rate;
    short gain_target;
    short current_gain_value;


    unsigned char voice_loop;
    unsigned char voice_playing;
    unsigned char voice_stop;
    unsigned int voice_offset;
    unsigned int voice_samples;
    unsigned int voice_loop_start;
    unsigned int voice_loop_end;
    

    unsigned char oscillator_type;
    unsigned char oscillator_note;
    short oscillator_amplitude;
    short oscillator_phase;

};

struct gen16x_spu {

    unsigned char flushed;
    unsigned char underrun;
    int l_volume;
    int r_volume;

    unsigned int time_setting;
    unsigned int time_counter;
    unsigned int current_output_position;
    unsigned int output_offset;
    unsigned int output_samples;
    unsigned int sample_rate;
    gen16x_dsp_channel channels[GEN16X_MAX_DSP_CHANNELS];

    short sram[0x40000];
};



void gen16x_spu_tick(gen16x_spu* spu);