use crate::gpu::{ Gpu, Tile };

pub struct Slider {
    value: i32,
    position: i32
}

impl Slider {
    pub fn set(&mut self, value: i32) {
        self.value = value;
    }

    pub fn get(&self) -> i32 {
        self.value
    }

    pub fn draw(&self, gpu: &mut Gpu) {
        let map_start = 64 * self.position as usize + 1;

        for index in &mut gpu.map[map_start..map_start + self.value as usize + 1] { *index = 2 }
        for index in &mut gpu.map[map_start + self.value as usize + 1..map_start + (33 - self.value as usize)] { *index = 0 }
    }
}

pub struct Engine {
    pub gpu: Gpu,
    pub game: Game
}

pub struct Game {
    pub current_palette_index: usize,
    pub sliders: [Slider; 3]
}

static example_tile_00: Tile = [
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000
];

static example_tile_01: Tile = [
    0b00000000, 0b00000000,
    0b00010101, 0b01010100,
    0b00010101, 0b01010100,
    0b00010101, 0b01010100,
    0b00010101, 0b01010100,
    0b00010101, 0b01010100,
    0b00010101, 0b01010100,
    0b00000000, 0b00000000
];

static example_tile_10: Tile = [
    0b00000000, 0b00000000,
    0b00101010, 0b10101000,
    0b00101010, 0b10101000,
    0b00101010, 0b10101000,
    0b00101010, 0b10101000,
    0b00101010, 0b10101000,
    0b00101010, 0b10101000,
    0b00000000, 0b00000000
];

static example_tile_11: Tile = [
    0b00000000, 0b00000000,
    0b00111111, 0b11111100,
    0b00111111, 0b11111100,
    0b00111111, 0b11111100,
    0b00111111, 0b11111100,
    0b00111111, 0b11111100,
    0b00111111, 0b11111100,
    0b00000000, 0b00000000
];

static pick_tile_01: Tile = [
    0b01010101, 0b01010101,
    0b01010101, 0b01010101,
    0b01010101, 0b01010101,
    0b01010101, 0b01010101,
    0b01010101, 0b01010101,
    0b01010101, 0b01010101,
    0b01010101, 0b01010101,
    0b01010101, 0b01010101
];

static pick_tile_10: Tile = [
    0b10101010, 0b10101010,
    0b10101010, 0b10101010,
    0b10101010, 0b10101010,
    0b10101010, 0b10101010,
    0b10101010, 0b10101010,
    0b10101010, 0b10101010,
    0b10101010, 0b10101010,
    0b10101010, 0b10101010
];

static pick_tile_11: Tile = [
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
];

static mouse_tile: Tile = [
    0b01000000, 0b00000000,
    0b01000000, 0b00000000,
    0b01010000, 0b00000000,
    0b01010000, 0b00000000,
    0b01010100, 0b00000000,
    0b01010100, 0b00000000,
    0b01010101, 0b00000000,
    0b01010101, 0b00000000
];
 
static indicator_tile: Tile = [
    0b01010101, 0b01010101,
    0b01010101, 0b01010101,
    0b01010101, 0b01010101,
    0b01010101, 0b01010101,
    0b01111111, 0b11111101,
    0b01011111, 0b11110101,
    0b01010111, 0b11010101,
    0b01010101, 0b01010101
];

impl Game {
    pub fn new() -> Game {
        Game {
            current_palette_index: 0,
            sliders: [
                Slider { position:  9, value: 0 },
                Slider { position: 11, value: 0 },
                Slider { position: 13, value: 0 }
            ]
        }
    }

    fn get_current_color(&self, gpu: &Gpu) -> u32 {
        if self.current_palette_index == 0 {
            gpu.bg_color
        } else {
            gpu.palette[0][self.current_palette_index - 1]
        }
    }

    fn set_current_color(&mut self, gpu: &mut Gpu, color: u32) {
        if self.current_palette_index == 0 {
            gpu.bg_color = color;
        } else {
            gpu.palette[0][self.current_palette_index - 1] = color;
        }
    }

    fn update_red_value(&mut self, gpu: &mut Gpu, value: i32) {
        self.set_current_color(
            gpu,
            (self.get_current_color(gpu)) |
                (((value << 3) & 0xFF) << 0) as u32
        )
    }

    fn update_green_value(&mut self, gpu: &mut Gpu, value: i32) {
        self.set_current_color(
            gpu,
            (self.get_current_color(gpu)) |
                (((value << 3) & 0xFF) << 8) as u32
        )
    }

    fn update_blue_value(&mut self, gpu: &mut Gpu, value: i32) {
        self.set_current_color(
            gpu,
            (self.get_current_color(gpu)) |
                (((value << 3) & 0xFF) << 16) as u32
        )
    }

    fn set_gpu_tile(gpu: &mut Gpu, index: usize, source_tile: &Tile) {
        for i in 0..source_tile.len() {
            gpu.tile[index][i] = source_tile[i];
        }
    }

    pub fn init(&mut self, gpu: &mut Gpu) {
        Game::set_gpu_tile(gpu, 0, &example_tile_00);
        Game::set_gpu_tile(gpu, 1, &example_tile_01);
        Game::set_gpu_tile(gpu, 2, &example_tile_10);
        Game::set_gpu_tile(gpu, 3, &example_tile_11);
        Game::set_gpu_tile(gpu, 4, &mouse_tile);
        Game::set_gpu_tile(gpu, 7, &pick_tile_01);
        Game::set_gpu_tile(gpu, 8, &pick_tile_10);
        Game::set_gpu_tile(gpu, 9, &pick_tile_11);
        Game::set_gpu_tile(gpu, 10, &indicator_tile);

        for byte in &mut gpu.tile[5] { *byte = 0 }
        for byte in &mut gpu.tile[6] { *byte = 0x55}
        for byte in &mut gpu.attr { *byte = 0x11 }
        for index in &mut gpu.map { *index = 0x06 }

        gpu.map[64 * 7 + 10] = 0x00;
        gpu.attr[32 * 7 + 5] = 0x01;

        gpu.map[64 * 7 + 12] = 0x07;
        gpu.attr[32 * 7 + 6] = 0x01;
    
        gpu.map[64 * 7 + 14] = 0x08;
        gpu.attr[32 * 7 + 7] = 0x01;
    
        gpu.map[64 * 7 + 16] = 0x09;
        gpu.attr[32 * 7 + 8] = 0x01;
    
        gpu.map[64 * 6 + 10] = 0x0A;
        gpu.attr[32 * 6 + 5] = 0x11;
    
        gpu.attr[15] = 0x10;

        for i in 0..8 {
            for index in &mut gpu.map[64 * i..64 * i + 8] { *index = 0 }
            for byte in &mut gpu.attr[32 * i..32 * i + 4] { *byte = 0x00 }
        }

        gpu.bg_color = 0xFFFFFFFF;

        gpu.palette[0][0] = 0xFF0000FF;
        gpu.palette[0][1] = 0xFF00FF00;
        gpu.palette[0][2] = 0xFFFF0000;
    
        gpu.palette[1][0] = 0xFF000000;
        gpu.palette[1][1] = 0xFF000000;
        gpu.palette[1][2] = 0xFF0000FF;
    
        gpu.sprites[0] = 0x01010410;
        gpu.map[31] = 0x05;

        for slider in &mut self.sliders {
            slider.set(3);
        }
        
        self.update_red_value(gpu, self.sliders[0].get());
        self.update_green_value(gpu, self.sliders[1].get());
        self.update_blue_value(gpu, self.sliders[2].get());

        for slider in &mut self.sliders {
            slider.draw(gpu);
        }
    }

}