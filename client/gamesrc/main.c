#include "../src/gpu.h"
#include "../src/event.h"
#include <stdio.h>
#include <memory.h>

int x_velo = 0;
int y_velo = 0;
int pos_x = 128;
int pos_y = 100;

uint8_t ExampleTile00[] = {
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000
};

uint8_t ExampleTile01[] = {
    0b00000000, 0b00000000,
    0b00010101, 0b01010100,
    0b00010101, 0b01010100,
    0b00010101, 0b01010100,
    0b00010101, 0b01010100,
    0b00010101, 0b01010100,
    0b00010101, 0b01010100,
    0b00000000, 0b00000000
};

uint8_t ExampleTile10[] = {
    0b00000000, 0b00000000,
    0b00101010, 0b10101000,
    0b00101010, 0b10101000,
    0b00101010, 0b10101000,
    0b00101010, 0b10101000,
    0b00101010, 0b10101000,
    0b00101010, 0b10101000,
    0b00000000, 0b00000000
};

uint8_t ExampleTile11[] = {
    0b00000000, 0b00000000,
    0b00111111, 0b11111100,
    0b00111111, 0b11111100,
    0b00111111, 0b11111100,
    0b00111111, 0b11111100,
    0b00111111, 0b11111100,
    0b00111111, 0b11111100,
    0b00000000, 0b00000000
};

uint8_t PickTile01[] = {
    0b01010101, 0b01010101,
    0b01010101, 0b01010101,
    0b01010101, 0b01010101,
    0b01010101, 0b01010101,
    0b01010101, 0b01010101,
    0b01010101, 0b01010101,
    0b01010101, 0b01010101,
    0b01010101, 0b01010101
};

uint8_t PickTile10[] = {
    0b10101010, 0b10101010,
    0b10101010, 0b10101010,
    0b10101010, 0b10101010,
    0b10101010, 0b10101010,
    0b10101010, 0b10101010,
    0b10101010, 0b10101010,
    0b10101010, 0b10101010,
    0b10101010, 0b10101010
};

uint8_t PickTile11[] = {
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
};

uint8_t MouseTile[] = {
    0b01000000, 0b00000000,
    0b01000000, 0b00000000,
    0b01010000, 0b00000000,
    0b01010000, 0b00000000,
    0b01010100, 0b00000000,
    0b01010100, 0b00000000,
    0b01010101, 0b00000000,
    0b01010101, 0b00000000
};

typedef struct Slider_S {
    int value;
    int position;
} Slider;

Slider sliders[3] = { 
    { .position = 9, .value = 0 },
    { .position = 11, .value = 0 },
    { .position = 13, .value = 0 }
};

void setSlider(GPU* gpu, Slider* slider, int value) {
    slider->value = value;
    memset(&gpu->map[64 * slider->position], 0x02, value + 1);
    memset(&gpu->map[64 * slider->position + value + 1], 0x00, 31 - value);
}

void setGpuTile(GPU* gpu, int index, uint8_t* source) {
    index <<= 4;
    for(int i = 0; i < 16; i++) {
        gpu->tile[index+i] = source[i];
    }
}

void Init(GPU* gpu) {
    setGpuTile(gpu, 0, ExampleTile00);
    setGpuTile(gpu, 1, ExampleTile01);
    setGpuTile(gpu, 2, ExampleTile10);
    setGpuTile(gpu, 3, ExampleTile11);
    setGpuTile(gpu, 4, MouseTile);
    setGpuTile(gpu, 7, PickTile01);
    setGpuTile(gpu, 8, PickTile10);
    setGpuTile(gpu, 9, PickTile11);

    memset(&gpu->tile[5 << 4], 0, 16);
    memset(&gpu->tile[6 << 4], 0x55, 16);
    memset(&gpu->attr, 0x11, 32 * 60);
    memset(&gpu->map, 0x06, 64 * 60);

    gpu->map[64 * 7 + 10] = 0x00;
    gpu->attr[32 * 7 + 5] = 0x01;
    gpu->map[64 * 7 + 12] = 0x07;
    gpu->attr[32 * 7 + 6] = 0x01;
    gpu->map[64 * 7 + 14] = 0x08;
    gpu->attr[32 * 7 + 7] = 0x01;
    gpu->map[64 * 7 + 16] = 0x09;
    gpu->attr[32 * 7 + 8] = 0x01;

    gpu->attr[15] = 0x10;
    
    for(int i = 0; i < 8; i++) {
        memset(&gpu->map[64 * i], 0x00, 8);
        memset(&gpu->attr[32 * i], 0x00, 4);
    }

    gpu->bgColor = 0xFFFFFFFF;

    gpu->palette[0][0] = 0xFF0000FF;
    gpu->palette[0][1] = 0xFF00FF00;
    gpu->palette[0][2] = 0xFFFF0000;

    gpu->palette[1][0] = 0xFF000000;

    gpu->sprites[0] = 0x01010410;
    gpu->map[31] = 0x05;

    for(int i = 0; i < 3; i++)
        setSlider(gpu, &sliders[i], 31);
}

void HandleEvent(GPU* gpu, Event* event) {

    if(event->type == KEY) {
        KeyEvent* keyEvent = (KeyEvent*)event;
        y_velo =
            keyEvent->code == 38
                ? keyEvent->isUp ? 0 : -1
                : keyEvent->code == 40
                    ? keyEvent->isUp
                        ? 0
                        : 1
                    : y_velo;
        x_velo =
            keyEvent->code == 37
                ? keyEvent->isUp ? 0 : -1
                : keyEvent->code == 39
                    ? keyEvent->isUp
                        ? 0
                        : 1
                    : x_velo;
    }

    if(event->type == MOUSE) {
        MouseEvent* mouseEvent = (MouseEvent*)event;
        pos_x = mouseEvent->x;
        pos_y = mouseEvent->y;

        int map_x = pos_x >> 3;
        int map_y = pos_y >> 3;

        if(!mouseEvent->buttons) return;

        if(map_x < 8 && map_y < 8) {

            uint8_t* loc = &gpu->map[map_y * 64 + map_x];
            *loc = (*loc + 1) % 4;

            uint8_t* pSpriteByte = &gpu->tile[
                (5 << 4) +
                (map_y * 2) + 
                (map_x >> 2) ];

            uint8_t shiftAmt = ((0x3 - (map_x & 0x3)) << 1);

            *pSpriteByte = 
                (*pSpriteByte &~(0x3 << shiftAmt)) |
                ((*loc & 0x3) << shiftAmt);
        }

        for(int i = 0; i < 3; i++) if(map_y == sliders[i].position) {
            setSlider(gpu, &sliders[i], map_x);
            break;
        }
    }
}

void Render(GPU* gpu) {
    gpu->sprites[0] = 
        (gpu->sprites[0] & 0x0000FFFF)
        | ((pos_x & 0xFF) << 24)
        | ((pos_y & 0xFF) << 16);
}