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

#include "gen16x_spu.h"

static const unsigned char osc_wave[4][256] = {
    {128,131,134,137,140,143,146,149,152,156,159,162,165,168,171,174,
     176,179,182,185,188,191,193,196,199,201,204,206,209,211,213,216,
     218,220,222,224,226,228,230,232,234,236,237,239,240,242,243,245,
     246,247,248,249,250,251,252,252,253,254,254,255,255,255,255,255,
     255,255,255,255,255,255,254,254,253,252,252,251,250,249,248,247,
     246,245,243,242,240,239,237,236,234,232,230,228,226,224,222,220,
     218,216,213,211,209,206,204,201,199,196,193,191,188,185,182,179,
     176,174,171,168,165,162,159,156,152,149,146,143,140,137,134,131,
     128,124,121,118,115,112,109,106,103,99, 96, 93, 90, 87, 84, 81,
     79, 76, 73, 70, 67, 64, 62, 59, 56, 54, 51, 49, 46, 44, 42, 39,
     37, 35, 33, 31, 29, 27, 25, 23, 21, 19, 18, 16, 15, 13, 12, 10,
     9, 8, 7, 6, 5, 4, 3, 3, 2, 1, 1, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 3, 4, 5, 6, 7, 8,
     9, 10, 12, 13, 15, 16, 18, 19, 21, 23, 25, 27, 29, 31, 33, 35,
     37, 39, 42, 44, 46, 49, 51, 54, 56, 59, 62, 64, 67, 70, 73, 76,
     79, 81, 84, 87, 90, 93, 96, 99, 103,106,109,112,115,118,121,124},
    {128, 130, 132, 134, 136, 138, 140, 142, 144, 146, 148, 150, 152, 154, 156, 158,
     160, 162, 164, 166, 168, 170, 172, 174, 176, 178, 180, 182, 184, 186, 188, 190,
     192, 194, 196, 198, 200, 202, 204, 206, 208, 210, 212, 214, 216, 218, 220, 222, 
     224, 226, 228, 230, 232, 234, 236, 238, 240, 242, 244, 246, 248, 250, 252, 254,
     255, 253, 251, 249, 247, 245, 243, 241, 239, 237, 235, 233, 231, 229, 227, 225,
     223, 221, 219, 217, 215, 213, 211, 209, 207, 205, 203, 201, 199, 197, 195, 193,
     191, 189, 187, 185, 183, 181, 179, 177, 175, 173, 171, 169, 167, 165, 163, 161,
     159, 157, 155, 153, 151, 149, 147, 145, 143, 141, 139, 137, 135, 133, 131, 129,
     127, 125, 123, 121, 119, 117, 115, 113, 111, 109, 107, 105, 103, 101,  99,  97, 
      95,  93,  91,  89,  87,  85,  83,  81,  79,  77,  75,  73,  71,  69,  67,  65,
      63,  61,  59,  57,  55,  53,  51,  49,  47,  45,  43,  41,  39,  37,  35,  33,
      31,  29,  27,  25,  23,  21,  19,  17,  15,  13,  11,  9,    7,   5,   3,   1,
       0,   2,   4,   6,   8,  10,  12,  14,  16,  18,  20, 22,   24,  26,  28,  30,
      32,  34,  36,  38,  40,  42,  44,  46,  48,  50,  52, 54,   56,  58,  60,  62,
      64,  66,  68,  70,  72,  74,  76,  78,  80,  82,  84, 86,   88,  90,  92,  94,
      96,  98, 100, 102, 104, 106, 108, 110, 112, 114, 116, 118, 120, 122, 124, 126},
    {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 239, 223, 207, 191, 175, 159, 143, 127, 111, 95, 79, 63, 47, 31, 15,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240}, 
    {225, 203, 49, 189, 51, 58, 217, 10, 159, 92, 76, 36, 129, 81, 195, 135, 157, 24, 
    188, 156, 249, 233, 78, 199, 142, 48, 16, 105, 71, 97, 186, 151, 151, 253, 4, 24,
    162, 232, 254, 67, 173, 91, 248, 155, 94, 225, 201, 144, 43, 19, 28, 55, 120, 58,
    226, 193, 160, 5, 133, 178, 188, 96, 72, 38, 101, 37, 15, 204, 154, 221, 65, 83,
    215, 223, 209, 162, 96, 168, 132, 35, 203, 138, 135, 165, 237, 174, 173, 211, 103,
    226, 42, 219, 48, 106, 159, 15, 165, 144, 193, 62, 140, 73, 72, 86, 75, 71, 12, 82,
    170, 240, 8, 198, 182, 11, 89, 42, 4, 109, 103, 126, 153, 246, 160, 25, 33, 146, 45,
    194, 242, 251, 114, 51, 21, 178, 75, 135, 29, 103, 211, 110, 182, 198, 213, 6, 185,
    160, 195, 48, 163, 145, 70, 170, 219, 186, 141, 156, 238, 225, 208, 64, 164, 252, 34,
    205, 206, 138, 104, 159, 190, 71, 118, 76, 3, 117, 218, 122, 203, 52, 56, 200, 191,
    254, 173, 25, 157, 176, 21, 37, 113, 229, 161, 206, 8, 213, 216, 189, 204, 49, 212,
    27, 118, 124, 75, 78, 12, 13, 63, 199, 48, 67, 189, 40, 215, 89, 96, 1, 175, 232,
    65, 167, 121, 195, 44, 154, 161, 39, 183, 75, 91, 22, 16, 203, 147, 39, 234, 194, 58, 234, 
    158, 54, 88, 209, 203, 38, 226, 121, 157, 54, 242, 180, 119, 170, 14, 41, 232, 54,}};



inline int fast_abs(int x) {
    return (x >= 0) ? x : -x;
}

inline int mod_wrap(int a, int b) {
    return (b + (a%b)) % b;
}
int midi_note_to_freq(int x, int a) {
    // converts a midi note x tuned to frequency A into a frequency integer
    //int a = 440;
    int b = 69;
    int c = 16;
    int d = 12;
    
    int hd = d/2;
    int k = 64;

    int x0 = x;
    if (x - b < 0) {
        x0 -= d;
    }

    int h = (x0-b)/d;
    
    int g = h >= 0 ? a*(1<<h) : (a/(1<<(-h)));
    int f = ((g*((d*(x-b))/d))/(d)) - g*(h-1);
    int p = hd - fast_abs(hd - mod_wrap(x-b,d));
    int q = (k - ((k*p)/hd));
    int u = k*(k - ((q*q)/k));
    return f -(f*u)/(c*k*k); //f-fuckk!!
}

static int midi_table_440x32[128] = {
    262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494, 523, 554, 587, 622, 659, 
    698, 740, 784, 831, 880, 932, 988, 1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 
    1661, 1760, 1865, 1976, 2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520,
    3729, 3951, 4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6645, 7040, 7459, 7902, 
    8372, 8870, 9397, 9956, 10548, 11175, 11840, 12544, 13290, 14080, 14917, 15804, 16744, 
    17740, 18795, 19912, 21096, 22351, 23680, 25088, 26580, 28160, 29834, 31609, 33488, 
    35479, 37589, 39824, 42192, 44701, 47359, 50175, 53159, 56320, 59669, 63217, 66976, 
    70959, 75178, 79649, 84385, 89402, 94719, 100351, 106318, 112640, 119338, 126434, 
    133952, 141918, 150356, 159297, 168769, 178805, 189437, 200702, 212636, 225280, 
    238676, 252868, 267905, 283835, 300713, 318594, 337539, 357610, 378874, 401403,
};

int midi_note_to_freq_table(int x) {
    return midi_table_440x32[x&0x7F];
}

void gen16x_spu_tick(gen16x_spu* spu) {
    
    int r_acc_signal = 0;
    int l_acc_signal = 0;
    short* output =  (spu->sram + spu->output_offset);
    if (spu->flushed) {
        spu->current_output_position = 0;
        spu->flushed = 0;
    }
    
    for (int i = 0; i < GEN16X_MAX_DSP_CHANNELS; i++) {
        gen16x_dsp_channel &dsp = spu->channels[i];
        
        if (!dsp.enabled) {
            continue;
        }
        short* voice = (spu->sram + dsp.voice_offset);
        
        
        int voice_time = int(dsp.time)/GEN16X_DSP_BASE_PITCH;
        
        if (dsp.voice_stop) {
            if (dsp.voice_playing) {
                dsp.voice_playing = 0;
            }
            dsp.voice_stop = 0;
        }
        if (dsp.voice_playing && (voice_time + 1) >= dsp.voice_samples/2) {
            if (dsp.voice_loop) {
                voice_time %= dsp.voice_samples/2;
            } else {
                dsp.voice_playing = 0;
                voice_time = 0;
            }
            
        }
        if ((spu->time_counter & 0x1) == 0) {
            switch (dsp.gain_type) {
                case GEN16X_DSP_GAIN_DIRECT:
                    dsp.current_gain_value = dsp.gain_target;
                    break;
                case GEN16X_DSP_GAIN_LINEAR:
                {
                    int dir = (dsp.gain_target - dsp.current_gain_value);
                    
                    dir = (dir < 0) ? -1 : ((dir > 0) ? 1 : 0);
                    
                    int new_gain_value = dir*(int)dsp.gain_rate + (int)dsp.current_gain_value;
                    
                    if ((dir < 0 && new_gain_value < dsp.gain_target)
                     || (dir > 0 && new_gain_value > dsp.gain_target)) {
                        dsp.current_gain_value = dsp.gain_target;
                    } else {
                        dsp.current_gain_value = new_gain_value;
                    }
                    break;
                }
                case GEN16X_DSP_GAIN_EXPONENTIAL:
                {
                    dsp.current_gain_value = (((int)dsp.gain_target*dsp.gain_rate) + ((int)dsp.current_gain_value*(GEN16X_DSP_MAX_VOLUME - (int)dsp.gain_rate)))/(GEN16X_DSP_MAX_VOLUME);
                    break;
                }
            }
        }

        
        //if (dsp.voice_playing) {

        int lerp_value = (dsp.time % GEN16X_DSP_BASE_PITCH);

        int l_input_signal0 = 0;
        int l_input_signal1 = 0;
        int r_input_signal0 = 0;
        int r_input_signal1 = 0;

        if (dsp.voice_playing) {
            l_input_signal0 = voice[(voice_time*2)];
            l_input_signal1 = voice[((voice_time+1)* 2)];
            r_input_signal0 = voice[((voice_time)* 2) + 1];
            r_input_signal1 = voice[((voice_time+1)* 2) + 1];
        }
        
        
        
        int l_input_signal = (l_input_signal0*(GEN16X_DSP_BASE_PITCH - lerp_value) + l_input_signal1*(lerp_value))/GEN16X_DSP_BASE_PITCH;

        
        int r_input_signal = (r_input_signal0*(GEN16X_DSP_BASE_PITCH - lerp_value) + r_input_signal1*(lerp_value))/GEN16X_DSP_BASE_PITCH;



        if (dsp.oscillator_type != GEN16X_DSP_OSC_NONE) {

            

            


            int osc_value = 0;
            unsigned int fixed_time = dsp.time/GEN16X_DSP_BASE_PITCH;

            unsigned int fixed_time_sub = (dsp.time%GEN16X_DSP_BASE_PITCH);

            int osc_time0 = (midi_note_to_freq_table(dsp.oscillator_note)*fixed_time)/(spu->sample_rate/8);

            int osc_value0 = 0;

            osc_value0 = (((int)osc_wave[(dsp.oscillator_type - 1)&0x3][(osc_time0 + dsp.oscillator_phase) & 0xFF]) - 127)*256;
            
            //= (((int)osc_wave[(dsp.oscillator_type - 1)&0x3][(osc_time0 + dsp.oscillator_phase) & 0xFF]) - 127)*128;
            
            osc_value = osc_value0;


            l_input_signal += osc_value*dsp.oscillator_amplitude/(GEN16X_DSP_MAX_VOLUME);
            r_input_signal += osc_value*dsp.oscillator_amplitude/(GEN16X_DSP_MAX_VOLUME);
        }

        l_input_signal = l_input_signal*dsp.l_volume/GEN16X_DSP_MAX_VOLUME;
        r_input_signal = r_input_signal*dsp.r_volume/GEN16X_DSP_MAX_VOLUME;

        l_acc_signal += ((l_input_signal)*((int)dsp.current_gain_value))/GEN16X_DSP_MAX_VOLUME;
        r_acc_signal += ((r_input_signal)*((int)dsp.current_gain_value))/GEN16X_DSP_MAX_VOLUME;

        dsp.time += (GEN16X_DSP_BASE_PITCH + dsp.pitch);

        //}
        
    }
    int output_pos = spu->current_output_position++;
    if (output_pos >= spu->output_samples/2) {
        output_pos = (spu->output_samples/2) - 1;
        spu->underrun = 1;
    } else {
        spu->underrun = 0;
    }
    int l_result = (l_acc_signal*spu->l_volume)/(GEN16X_DSP_MAX_VOLUME);
    int r_result = (r_acc_signal*spu->r_volume)/(GEN16X_DSP_MAX_VOLUME);
    if (l_result <= -(65534/2)) {
        l_result = -(65534/2);
    }
    if (l_result >= (65534/2)) {
        l_result = (65534/2);
    }
    if (r_result <= -(65534/2)) {
        r_result = -(65534/2);
    }
    if (r_result >= (65534/2)) {
        r_result = (65534/2);
    }
    output[(output_pos*2)] = (l_result);
    output[(output_pos*2) + 1] = (r_result);
    spu->time_counter++;
}