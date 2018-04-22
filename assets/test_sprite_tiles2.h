unsigned int test_sprite2_palette[11] = {
    0xFF000000,0xFFFFFFFF,0xFF271920,0x00FF00FF,0xFF754D12,0xFF9F6D20,0xFFF7B145,0xFFEECB97,0xFF362712,0xFFEEDCD4,0xFFFFFFFF,
};
unsigned char test_sprite2_tiles[16][4][32] = {
    {
        0x33,0x33,0x32,0x33,0x33,0x33,0x22,0x32,0x33,0x32,0x42,0x26,0x33,0x24,0x42,0x66,0x33,0x26,0x66,0x62,0x32,0x26,0x22,0x66,0x34,0x62,0x56,0x66,0x32,0x66,0x66,0x67,
        0x23,0x33,0x33,0x33,0x23,0x33,0x33,0x33,0x23,0x33,0x33,0x33,0x23,0x33,0x33,0x33,0x23,0x33,0x33,0x33,0x23,0x33,0x33,0x33,0x23,0x33,0x33,0x33,0x73,0x33,0x33,0x33,
        0x33,0x26,0x66,0x62,0x33,0x32,0x62,0x23,0x33,0x34,0x97,0x62,0x33,0x34,0x76,0x66,0x33,0x32,0x66,0x66,0x33,0x32,0x66,0x64,0x33,0x32,0x46,0x64,0x33,0x33,0x22,0x22,
        0x23,0x32,0x33,0x33,0x33,0x26,0x23,0x33,0x33,0x32,0x42,0x33,0x28,0x86,0x42,0x33,0x48,0x26,0x23,0x33,0x44,0x22,0x33,0x33,0x44,0x23,0x33,0x33,0x22,0x33,0x33,0x33,
    },
    {
        0x33,0x33,0x23,0x32,0x33,0x32,0x23,0x22,0x33,0x24,0x22,0x62,0x32,0x44,0x26,0x62,0x32,0x66,0x66,0x22,0x22,0x62,0x26,0x62,0x46,0x25,0x66,0x62,0x26,0x66,0x66,0x77,
        0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x22,0x33,0x33,0x32,0x66,0x23,0x33,0x33,0x24,0x42,0x33,0x33,0x24,0x42,0x83,0x33,0x24,0x42,
        0x32,0x66,0x66,0x52,0x33,0x22,0x67,0x76,0x32,0x28,0x56,0x66,0x25,0x55,0x86,0x62,0x25,0x22,0x22,0x66,0x32,0x33,0x32,0x56,0x33,0x33,0x32,0x56,0x33,0x33,0x32,0x22,
        0x22,0x22,0x66,0x23,0x64,0x44,0x25,0x23,0x66,0x44,0x42,0x33,0x58,0x64,0x22,0x33,0x22,0x62,0x55,0x23,0x26,0x62,0x25,0x23,0x25,0x23,0x32,0x23,0x32,0x23,0x33,0x33,
    },
    {
        0x33,0x33,0x23,0x32,0x33,0x32,0x23,0x22,0x33,0x24,0x22,0x62,0x32,0x44,0x26,0x62,0x32,0x66,0x66,0x22,0x22,0x62,0x26,0x62,0x46,0x25,0x66,0x62,0x26,0x66,0x66,0x77,
        0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x32,0x22,0x33,0x33,0x26,0x62,0x33,0x33,0x24,0x42,0x33,0x33,0x24,0x23,0x83,0x32,0x44,0x23,
        0x32,0x66,0x66,0x52,0x33,0x22,0x77,0x66,0x33,0x32,0x66,0x66,0x33,0x32,0x66,0x66,0x33,0x33,0x26,0x68,0x33,0x33,0x26,0x52,0x33,0x33,0x26,0x83,0x33,0x33,0x32,0x23,
        0x22,0x22,0x66,0x23,0x64,0x44,0x22,0x33,0x64,0x44,0x52,0x33,0x66,0x44,0x52,0x33,0x58,0x66,0x82,0x33,0x22,0x56,0x83,0x33,0x33,0x26,0x83,0x33,0x33,0x22,0x23,0x33,
    },
    {
        0x33,0x33,0x33,0x33,0x33,0x33,0x23,0x32,0x33,0x32,0x23,0x22,0x33,0x24,0x22,0x62,0x32,0x44,0x26,0x62,0x32,0x66,0x66,0x22,0x22,0x62,0x26,0x62,0x46,0x25,0x66,0x62,
        0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x44,0x23,0x33,0x34,0x46,0x62,0x33,0x34,0x44,0x22,0x33,0x32,0x42,0x33,
        0x26,0x66,0x66,0x77,0x32,0x66,0x65,0x22,0x33,0x28,0x77,0x66,0x32,0x26,0x66,0x66,0x26,0x66,0x62,0x56,0x26,0x22,0x25,0x52,0x22,0x33,0x32,0x52,0x33,0x33,0x32,0x23,
        0x83,0x32,0x62,0x33,0x22,0x25,0x52,0x33,0x44,0x44,0x88,0x33,0x64,0x44,0x42,0x33,0x66,0x66,0x62,0x33,0x22,0x25,0x56,0x23,0x33,0x32,0x26,0x23,0x33,0x33,0x32,0x23,
    },
    {
        0x33,0x33,0x33,0x32,0x33,0x33,0x33,0x22,0x33,0x33,0x32,0x42,0x33,0x33,0x32,0x42,0x33,0x33,0x24,0x66,0x33,0x33,0x26,0x66,0x33,0x33,0x22,0x66,0x33,0x33,0x46,0x26,
        0x33,0x32,0x33,0x33,0x33,0x82,0x33,0x33,0x38,0x62,0x33,0x33,0x26,0x62,0x33,0x33,0x66,0x62,0x33,0x33,0x66,0x62,0x33,0x33,0x66,0x22,0x33,0x33,0x68,0x68,0x33,0x33,
        0x33,0x33,0x85,0x66,0x33,0x33,0x32,0x56,0x33,0x33,0x33,0x26,0x33,0x33,0x33,0x89,0x33,0x33,0x38,0x69,0x33,0x33,0x38,0x66,0x33,0x33,0x32,0x46,0x33,0x33,0x33,0x22,
        0x66,0x58,0x33,0x33,0x65,0x23,0x33,0x33,0x42,0x33,0x33,0x33,0x95,0x23,0x33,0x33,0x66,0x42,0x33,0x33,0x64,0x42,0x33,0x33,0x44,0x42,0x33,0x33,0x22,0x23,0x33,0x33,
    },
    {
        0x33,0x33,0x33,0x32,0x33,0x33,0x33,0x22,0x33,0x33,0x32,0x42,0x33,0x33,0x32,0x42,0x33,0x33,0x24,0x66,0x33,0x33,0x26,0x66,0x33,0x33,0x22,0x66,0x33,0x33,0x46,0x26,
        0x33,0x32,0x33,0x33,0x33,0x82,0x33,0x33,0x38,0x62,0x33,0x33,0x26,0x62,0x33,0x33,0x66,0x62,0x33,0x33,0x66,0x62,0x33,0x33,0x66,0x22,0x33,0x33,0x68,0x68,0x33,0x33,
        0x33,0x33,0x85,0x66,0x33,0x33,0x38,0x56,0x33,0x33,0x32,0x48,0x33,0x33,0x26,0x59,0x33,0x33,0x26,0x25,0x33,0x33,0x25,0x22,0x33,0x33,0x32,0x33,0x33,0x33,0x33,0x33,
        0x66,0x58,0x33,0x33,0x65,0x83,0x33,0x33,0x84,0x23,0x33,0x33,0x96,0x23,0x33,0x33,0x46,0x23,0x33,0x33,0x25,0x83,0x33,0x33,0x26,0x58,0x33,0x33,0x32,0x23,0x33,0x33,
    },
    {
        0x33,0x33,0x33,0x83,0x33,0x33,0x33,0x23,0x33,0x33,0x32,0x42,0x33,0x33,0x32,0x42,0x33,0x33,0x24,0x66,0x33,0x33,0x26,0x66,0x33,0x33,0x22,0x66,0x33,0x33,0x46,0x26,
        0x33,0x38,0x33,0x33,0x22,0x82,0x33,0x33,0x68,0x62,0x33,0x33,0x26,0x62,0x33,0x33,0x66,0x62,0x33,0x33,0x66,0x62,0x33,0x33,0x66,0x22,0x33,0x33,0x68,0x68,0x33,0x33,
        0x33,0x33,0x85,0x66,0x33,0x33,0x32,0x56,0x33,0x33,0x32,0x68,0x33,0x33,0x32,0x59,0x33,0x33,0x33,0x28,0x33,0x33,0x32,0x64,0x33,0x33,0x33,0x22,0x33,0x33,0x33,0x33,
        0x66,0x58,0x33,0x33,0x65,0x23,0x33,0x33,0x86,0x42,0x33,0x33,0x94,0x62,0x33,0x33,0x28,0x75,0x33,0x33,0x23,0x22,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,
    },
    {
        0x33,0x33,0x33,0x83,0x33,0x33,0x33,0x23,0x33,0x33,0x32,0x42,0x33,0x33,0x32,0x42,0x33,0x33,0x24,0x66,0x33,0x33,0x26,0x66,0x33,0x33,0x22,0x66,0x33,0x33,0x46,0x26,
        0x33,0x38,0x33,0x33,0x22,0x82,0x33,0x33,0x68,0x62,0x33,0x33,0x26,0x62,0x33,0x33,0x66,0x62,0x33,0x33,0x66,0x62,0x33,0x33,0x66,0x22,0x33,0x33,0x68,0x68,0x33,0x33,
        0x33,0x33,0x85,0x66,0x33,0x33,0x32,0x56,0x33,0x33,0x24,0x68,0x33,0x33,0x26,0x49,0x33,0x33,0x57,0x82,0x33,0x33,0x22,0x32,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,
        0x66,0x58,0x33,0x33,0x65,0x23,0x33,0x33,0x86,0x23,0x33,0x33,0x95,0x23,0x33,0x33,0x82,0x33,0x33,0x33,0x46,0x23,0x33,0x33,0x22,0x33,0x33,0x33,0x33,0x33,0x33,0x33,
    },
    {
        0x33,0x33,0x33,0x32,0x33,0x33,0x33,0x22,0x33,0x33,0x32,0x42,0x33,0x33,0x32,0x42,0x33,0x33,0x24,0x66,0x33,0x33,0x26,0x66,0x33,0x33,0x22,0x66,0x33,0x33,0x46,0x26,
        0x33,0x32,0x33,0x33,0x33,0x82,0x33,0x33,0x38,0x62,0x33,0x33,0x26,0x62,0x33,0x33,0x66,0x62,0x33,0x33,0x66,0x62,0x33,0x33,0x66,0x22,0x33,0x33,0x68,0x68,0x33,0x33,
        0x33,0x33,0x85,0x66,0x33,0x33,0x38,0x56,0x33,0x33,0x32,0x48,0x33,0x33,0x32,0x69,0x33,0x33,0x32,0x64,0x33,0x33,0x38,0x52,0x33,0x33,0x85,0x62,0x33,0x33,0x32,0x23,
        0x66,0x58,0x33,0x33,0x65,0x83,0x33,0x33,0x84,0x23,0x33,0x33,0x95,0x62,0x33,0x33,0x52,0x62,0x33,0x33,0x22,0x52,0x33,0x33,0x33,0x23,0x33,0x33,0x33,0x33,0x33,0x33,
    },
    {
        0x33,0x33,0x23,0x33,0x33,0x33,0x22,0x33,0x33,0x33,0x24,0x23,0x33,0x33,0x24,0x42,0x33,0x33,0x24,0x44,0x33,0x33,0x24,0x66,0x33,0x33,0x26,0x79,0x33,0x33,0x86,0x25,
        0x23,0x33,0x33,0x33,0x22,0x33,0x33,0x33,0x26,0x23,0x33,0x33,0x26,0x23,0x33,0x33,0x66,0x62,0x33,0x33,0x66,0x62,0x33,0x33,0x97,0x62,0x33,0x33,0x52,0x68,0x33,0x33,
        0x33,0x33,0x25,0x26,0x33,0x33,0x32,0x26,0x33,0x33,0x32,0x46,0x33,0x33,0x32,0x66,0x33,0x33,0x32,0x64,0x33,0x33,0x32,0x64,0x33,0x33,0x32,0x44,0x33,0x33,0x33,0x22,
        0x62,0x52,0x33,0x33,0x62,0x23,0x33,0x33,0x66,0x23,0x33,0x33,0x46,0x62,0x33,0x33,0x42,0x22,0x33,0x33,0x46,0x42,0x33,0x33,0x46,0x62,0x33,0x33,0x22,0x23,0x33,0x33,
    },
    {
        0x33,0x33,0x23,0x33,0x33,0x33,0x22,0x33,0x33,0x33,0x24,0x23,0x33,0x33,0x24,0x42,0x33,0x33,0x24,0x44,0x33,0x33,0x24,0x66,0x33,0x33,0x26,0x79,0x33,0x33,0x86,0x25,
        0x23,0x33,0x33,0x33,0x22,0x33,0x33,0x33,0x26,0x23,0x33,0x33,0x26,0x23,0x33,0x33,0x66,0x62,0x33,0x33,0x66,0x62,0x33,0x33,0x97,0x62,0x33,0x33,0x52,0x68,0x33,0x33,
        0x33,0x32,0x32,0x56,0x33,0x26,0x22,0x46,0x33,0x27,0x64,0x24,0x33,0x32,0x44,0x66,0x33,0x33,0x22,0x22,0x33,0x33,0x33,0x22,0x33,0x33,0x33,0x32,0x33,0x33,0x33,0x32,
        0x65,0x23,0x33,0x33,0x66,0x23,0x33,0x33,0x66,0x48,0x33,0x33,0x66,0x48,0x33,0x33,0x26,0x48,0x33,0x33,0x66,0x23,0x33,0x33,0x62,0x33,0x33,0x33,0x23,0x33,0x33,0x33,
    },
    {
        0x33,0x33,0x23,0x33,0x33,0x33,0x22,0x33,0x33,0x33,0x24,0x23,0x33,0x33,0x24,0x42,0x33,0x33,0x24,0x44,0x33,0x33,0x24,0x66,0x33,0x33,0x26,0x79,0x33,0x33,0x86,0x25,
        0x23,0x33,0x33,0x33,0x22,0x33,0x33,0x33,0x26,0x23,0x33,0x33,0x26,0x23,0x33,0x33,0x66,0x62,0x33,0x33,0x66,0x62,0x33,0x33,0x97,0x62,0x33,0x33,0x52,0x68,0x33,0x33,
        0x33,0x33,0x32,0x56,0x33,0x33,0x26,0x22,0x33,0x33,0x26,0x64,0x33,0x33,0x32,0x24,0x33,0x33,0x32,0x52,0x33,0x33,0x33,0x22,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,
        0x65,0x23,0x33,0x33,0x66,0x58,0x33,0x33,0x26,0x58,0x33,0x33,0x56,0x58,0x33,0x33,0x66,0x23,0x33,0x33,0x62,0x33,0x33,0x33,0x22,0x33,0x33,0x33,0x33,0x33,0x33,0x33,
    },
    {
        0x33,0x33,0x23,0x33,0x33,0x33,0x22,0x33,0x33,0x33,0x24,0x23,0x33,0x33,0x24,0x42,0x33,0x33,0x24,0x44,0x33,0x33,0x24,0x66,0x33,0x33,0x26,0x79,0x33,0x33,0x86,0x25,
        0x23,0x33,0x33,0x33,0x22,0x33,0x33,0x33,0x26,0x23,0x33,0x33,0x26,0x23,0x33,0x33,0x66,0x62,0x33,0x33,0x66,0x62,0x33,0x33,0x97,0x62,0x33,0x33,0x52,0x68,0x33,0x33,
        0x33,0x33,0x32,0x56,0x33,0x33,0x85,0x66,0x33,0x33,0x85,0x62,0x33,0x33,0x85,0x65,0x33,0x33,0x32,0x66,0x33,0x33,0x33,0x26,0x33,0x33,0x33,0x22,0x33,0x33,0x33,0x33,
        0x65,0x23,0x33,0x33,0x22,0x62,0x33,0x33,0x46,0x62,0x33,0x33,0x42,0x23,0x33,0x33,0x25,0x23,0x33,0x33,0x22,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,
    },
    {
        0x33,0x33,0x23,0x33,0x33,0x33,0x22,0x33,0x33,0x33,0x24,0x23,0x33,0x33,0x24,0x42,0x33,0x33,0x24,0x44,0x33,0x33,0x24,0x66,0x33,0x33,0x26,0x79,0x33,0x33,0x86,0x25,
        0x23,0x33,0x33,0x33,0x22,0x33,0x33,0x33,0x26,0x23,0x33,0x33,0x26,0x23,0x33,0x33,0x66,0x62,0x33,0x33,0x66,0x62,0x33,0x33,0x97,0x62,0x33,0x33,0x52,0x68,0x33,0x33,
        0x33,0x33,0x32,0x56,0x33,0x33,0x32,0x66,0x33,0x33,0x84,0x66,0x33,0x33,0x84,0x66,0x33,0x33,0x84,0x62,0x33,0x33,0x32,0x66,0x33,0x33,0x33,0x26,0x33,0x33,0x33,0x32,
        0x65,0x23,0x23,0x33,0x64,0x22,0x62,0x33,0x42,0x46,0x72,0x33,0x66,0x44,0x23,0x33,0x22,0x22,0x33,0x33,0x22,0x33,0x33,0x33,0x23,0x33,0x33,0x33,0x23,0x33,0x33,0x33,
    },
    {
        0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x23,0x33,0x33,0x32,0x23,0x32,0x33,0x32,0x42,0x26,0x33,0x24,0x42,0x65,
        0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x23,0x33,0x33,0x33,0x23,0x22,0x22,0x33,0x22,0x45,0x44,0x23,0x26,0x64,0x44,0x23,
        0x33,0x26,0x66,0x65,0x33,0x26,0x62,0x26,0x32,0x66,0x25,0x66,0x32,0x56,0x66,0x62,0x33,0x25,0x55,0x26,0x33,0x32,0x22,0x22,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,
        0x26,0x64,0x44,0x42,0x26,0x54,0x44,0x42,0x26,0x54,0x44,0x42,0x22,0x22,0x64,0x42,0x64,0x46,0x64,0x88,0x65,0x46,0x52,0x83,0x28,0x82,0x83,0x33,0x33,0x33,0x33,0x33,
    },
    {
        0x33,0x33,0x23,0x32,0x33,0x32,0x23,0x22,0x33,0x24,0x22,0x62,0x32,0x44,0x26,0x62,0x32,0x66,0x66,0x22,0x22,0x62,0x26,0x62,0x46,0x25,0x66,0x62,0x26,0x66,0x66,0x77,
        0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x44,0x23,0x33,0x34,0x46,0x62,0x33,0x34,0x44,0x22,0x33,0x32,0x42,0x33,0x83,0x32,0x62,0x33,
        0x32,0x66,0x65,0x22,0x24,0x28,0x77,0x66,0x22,0x26,0x66,0x66,0x26,0x66,0x62,0x56,0x26,0x22,0x23,0x22,0x22,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,
        0x22,0x25,0x52,0x33,0x44,0x44,0x88,0x33,0x64,0x44,0x42,0x33,0x66,0x66,0x62,0x33,0x22,0x25,0x56,0x23,0x33,0x32,0x26,0x23,0x33,0x33,0x32,0x23,0x33,0x33,0x33,0x33,
    },
};