#pragma once;


union PlayerInput {
    struct {
        int move_dir_x : 6;
        int move_dir_y : 6;
        int look_dir_x : 6;
        int look_dir_y : 6;
        unsigned int action0 : 1;
        unsigned int action1 : 1;
        unsigned int action2 : 1;
        unsigned int action3 : 1;
        unsigned int menu : 1;
        unsigned int pause : 1;
        unsigned int trigger0 : 1;
        unsigned int trigger1 : 1;
    };
    unsigned int input;
};