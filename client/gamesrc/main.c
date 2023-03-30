#include "../src/gpu.h"
#include "../src/event.h"
#include "../src/engine.h"
#include <memory.h>
#include <stdlib.h>

typedef void (*SliderChangeHandler)(Engine* engine, struct Slider_S*);

typedef struct Slider_S {
    int value;
    int position;
    SliderChangeHandler onchange;
} Slider;

typedef struct GameState_S {
    int xVelocity;
    int yVelocity;
    int xPosition;
    int yPosition;
    Slider sliders[3];
    int currentPaletteIndex;
} GameState;

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

uint8_t IndicatorTile[] = {
    0b01010101, 0b01010101,
    0b01010101, 0b01010101,
    0b01010101, 0b01010101,
    0b01010101, 0b01010101,
    0b01111111, 0b11111101,
    0b01011111, 0b11110101,
    0b01010111, 0b11010101,
    0b01010101, 0b01010101
};

void setSlider(Engine* engine, Slider* slider, int value) {

    slider->value = value;
    memset(&engine->gpu->map[64 * slider->position], 0x02, value + 1);
    memset(&engine->gpu->map[64 * slider->position + value + 1], 0x00, 31 - value);

    slider->onchange(engine, slider);
}

void setGpuTile(GPU* gpu, int index, uint8_t* source) {

    index <<= 4;

    for(int i = 0; i < 16; i++) 
        gpu->tile[index+i] = source[i];
}

uint32_t getCurrentColor(Engine* engine) {

    GameState* state = (GameState*)engine->state;

    if(state->currentPaletteIndex == 0)
        return engine->gpu->bgColor;

    return engine->gpu->palette[0][state->currentPaletteIndex - 1];
}

void setCurrentColor(Engine* engine, uint32_t color) {

    GameState* state = (GameState*)engine->state;

    if(state->currentPaletteIndex == 0) {

        engine->gpu->bgColor = color;

        return;
    }

    engine->gpu->palette[0][state->currentPaletteIndex - 1] = color;
}

void updateRedValue(Engine* engine, Slider* slider) {

    setCurrentColor(
        engine,
        (getCurrentColor(engine) & 0xFFFFFF00) |
            (((slider->value << 3) & 0xFF) << 0) );
}

void updateGreenValue(Engine* engine, Slider* slider) {

    setCurrentColor(
        engine,
        (getCurrentColor(engine) & 0xFFFF00FF) |
            (((slider->value << 3) & 0xFF) << 8) );
}

void updateBlueValue(Engine* engine, Slider* slider) {

    setCurrentColor(
        engine,
        (getCurrentColor(engine) & 0xFF00FFFF) |
            (((slider->value << 3) & 0xFF) << 16) );
}

GameState* createInitialGameState() {

    GameState* state = (void*)malloc(sizeof(GameState));

    *state = (GameState) {
        .xVelocity = 0,
        .xPosition = 0,
        .yVelocity = 0,
        .yPosition = 0,
        .currentPaletteIndex = 0,
        .sliders = { 
            { .position =  9, .value = 0, .onchange = (SliderChangeHandler)&updateRedValue },
            { .position = 11, .value = 0,  .onchange = (SliderChangeHandler)&updateGreenValue },
            { .position = 13, .value = 0, .onchange = (SliderChangeHandler)&updateBlueValue }
        }
    };

    return state;
}

void Init(Engine* engine) {

    GPU* gpu = engine->gpu;
    GameState* state = (void*)createInitialGameState(); 

    engine->state = (void*)state;

    setGpuTile(gpu, 0, ExampleTile00);
    setGpuTile(gpu, 1, ExampleTile01);
    setGpuTile(gpu, 2, ExampleTile10);
    setGpuTile(gpu, 3, ExampleTile11);
    setGpuTile(gpu, 4, MouseTile);
    setGpuTile(gpu, 7, PickTile01);
    setGpuTile(gpu, 8, PickTile10);
    setGpuTile(gpu, 9, PickTile11);
    setGpuTile(gpu, 10, IndicatorTile);

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

    gpu->map[64 * 6 + 10] = 0x0A;
    gpu->attr[32 * 6 + 5] = 0x11;

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
    gpu->palette[1][1] = 0xFF000000;
    gpu->palette[1][2] = 0xFF0000FF;

    gpu->sprites[0] = 0x01010410;
    gpu->map[31] = 0x05;

    for(int i = 0; i < 3; i++)
        setSlider(engine, &state->sliders[i], 31);
}

void HandleEvent(Engine* engine, Event* event) {

    GameState* state = (GameState*)engine->state;
    GPU* gpu = engine->gpu;

    if(event->type == KEY) {
        KeyEvent* keyEvent = (KeyEvent*)event;
        state->yVelocity =
            keyEvent->code == 38
                ? keyEvent->isUp ? 0 : -1
                : keyEvent->code == 40
                    ? keyEvent->isUp
                        ? 0
                        : 1
                    : state->yVelocity;
        state->xVelocity =
            keyEvent->code == 37
                ? keyEvent->isUp ? 0 : -1
                : keyEvent->code == 39
                    ? keyEvent->isUp
                        ? 0
                        : 1
                    : state->xVelocity;
    }

    if(event->type == MOUSE) {
        MouseEvent* mouseEvent = (MouseEvent*)event;
        state->xPosition = mouseEvent->x;
        state->yPosition = mouseEvent->y;

        int map_x = state->xPosition >> 3;
        int map_y = state->yPosition >> 3;

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

        for(int i = 0; i < 3; i++) {

            if(map_y == state->sliders[i].position) {

                setSlider(engine, &state->sliders[i], map_x);

                break;
            }
        }

        for(int i = 0; i < 4; i++) {

            if(map_y == 7 && map_x == (10 + (2 * i))) {

                gpu->map[64 * 6 + 10 + (2 * state->currentPaletteIndex)] = 0x07;
                gpu->attr[32 * 6 + 5 + state->currentPaletteIndex] = 0x11;

                gpu->map[64 * 6 + 10 + (2 * i)] = 0x0A;
                gpu->attr[32 * 6 + 5 + i] = 0x11;

                state->currentPaletteIndex = i;

                int32_t color = getCurrentColor(engine);

                setSlider(engine, &state->sliders[0], ((color >>  0) & 0xFF) >> 3);
                setSlider(engine, &state->sliders[1], ((color >>  8) & 0xFF) >> 3);
                setSlider(engine, &state->sliders[2], ((color >> 16) & 0xFF) >> 3);

                break;
            }
        }
    }
}

void Render(Engine* engine) {

    GameState* state = (GameState*)engine->state;

    engine->gpu->sprites[0] = 
        (engine->gpu->sprites[0] & 0x0000FFFF)
        | ((state->xPosition & 0xFF) << 24)
        | ((state->yPosition & 0xFF) << 16);
}