use crate::gpu::{ Gpu, Tile };

pub struct MouseEventData { pub x: u32, pub y: u32, pub buttons: u32 }
pub struct KeyEventData { code: u32, is_up: bool }
pub enum Event {
    MouseEvent(MouseEventData),
    KeyEvent(KeyEventData)
}

pub struct Slider {
    value: i32,
    position: i32
}

fn print_to_console(content: &str) {
    let args = js_sys::Array::new();
    
    args.push(&wasm_bindgen::JsValue::from_str(content));
    web_sys::console::log(&args);
}

impl Slider {
    pub fn set(&mut self, value: i32) {
        print_to_console(&format!("Setting slider value to {value}"));
        self.value = value;
    }

    pub fn get(&self) -> i32 {
        self.value
    }

    pub fn draw(&self, gpu: &mut Gpu) {
        let map_start = 64 * self.position as usize + 1;
        let color_end = map_start + self.value as usize;
        let clear_start = color_end;
        let clear_end = 64 * self.position as usize + 31;

        print_to_console(&format!("Drawing blocks from {map_start} to {color_end}"));
        for index in &mut gpu.map[map_start..color_end] { *index = 2 }

        print_to_console(&format!("Drawing clears from {clear_start} to {clear_end}"));
        for index in &mut gpu.map[clear_start..clear_end] { *index = 0 }
    }
}

pub struct Game {
    pub current_palette_index: usize,
    pub sliders: [Slider; 3],
    pub x_position: u32,
    pub y_position: u32
}

static EXAMPLE_TILE_00: Tile = [
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000
];

static EXAMPLE_TILE_01: Tile = [
    0b00000000, 0b00000000,
    0b00010101, 0b01010100,
    0b00010101, 0b01010100,
    0b00010101, 0b01010100,
    0b00010101, 0b01010100,
    0b00010101, 0b01010100,
    0b00010101, 0b01010100,
    0b00000000, 0b00000000
];

static EXAMPLE_TILE_10: Tile = [
    0b00000000, 0b00000000,
    0b00101010, 0b10101000,
    0b00101010, 0b10101000,
    0b00101010, 0b10101000,
    0b00101010, 0b10101000,
    0b00101010, 0b10101000,
    0b00101010, 0b10101000,
    0b00000000, 0b00000000
];

static EXAMPLE_TILE_11: Tile = [
    0b00000000, 0b00000000,
    0b00111111, 0b11111100,
    0b00111111, 0b11111100,
    0b00111111, 0b11111100,
    0b00111111, 0b11111100,
    0b00111111, 0b11111100,
    0b00111111, 0b11111100,
    0b00000000, 0b00000000
];

static PICK_TILE_01: Tile = [
    0b01010101, 0b01010101,
    0b01010101, 0b01010101,
    0b01010101, 0b01010101,
    0b01010101, 0b01010101,
    0b01010101, 0b01010101,
    0b01010101, 0b01010101,
    0b01010101, 0b01010101,
    0b01010101, 0b01010101
];

static PICK_TILE_10: Tile = [
    0b10101010, 0b10101010,
    0b10101010, 0b10101010,
    0b10101010, 0b10101010,
    0b10101010, 0b10101010,
    0b10101010, 0b10101010,
    0b10101010, 0b10101010,
    0b10101010, 0b10101010,
    0b10101010, 0b10101010
];

static PICK_TILE_11: Tile = [
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
];

static MOUSE_TILE: Tile = [
    0b01000000, 0b00000000,
    0b01000000, 0b00000000,
    0b01010000, 0b00000000,
    0b01010000, 0b00000000,
    0b01010100, 0b00000000,
    0b01010100, 0b00000000,
    0b01010101, 0b00000000,
    0b01010101, 0b00000000
];
 
static INDICATOR_TILE: Tile = [
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
            x_position: 0,
            y_position: 0,
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
            (self.get_current_color(gpu) & !(0xFF << 0)) |
                (((value << 3) & 0xFF) << 0) as u32
        )
    }

    fn update_green_value(&mut self, gpu: &mut Gpu, value: i32) {
        self.set_current_color(
            gpu,
            (self.get_current_color(gpu) & !(0xFF << 8)) |
                (((value << 3) & 0xFF) << 8) as u32
        )
    }

    fn update_blue_value(&mut self, gpu: &mut Gpu, value: i32) {
        self.set_current_color(
            gpu,
            (self.get_current_color(gpu) & !(0xFF << 16)) |
                (((value << 3) & 0xFF) << 16) as u32
        )
    }

    fn set_gpu_tile(gpu: &mut Gpu, index: usize, source_tile: &Tile) {
        for i in 0..source_tile.len() {
            gpu.tile[index][i] = source_tile[i];
        }
    }

    pub fn init(&mut self, gpu: &mut Gpu) {
        Game::set_gpu_tile(gpu, 0, &EXAMPLE_TILE_00);
        Game::set_gpu_tile(gpu, 1, &EXAMPLE_TILE_01);
        Game::set_gpu_tile(gpu, 2, &EXAMPLE_TILE_10);
        Game::set_gpu_tile(gpu, 3, &EXAMPLE_TILE_11);
        Game::set_gpu_tile(gpu, 4, &MOUSE_TILE);
        Game::set_gpu_tile(gpu, 7, &PICK_TILE_01);
        Game::set_gpu_tile(gpu, 8, &PICK_TILE_10);
        Game::set_gpu_tile(gpu, 9, &PICK_TILE_11);
        Game::set_gpu_tile(gpu, 10, &INDICATOR_TILE);

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

    pub fn handle_event(&mut self, gpu: &mut Gpu, event: Event) {
        match event {
            Event::MouseEvent(mouse_data) => {
                self.x_position = mouse_data.x;
                self.y_position = mouse_data.y;

                let map_x = (self.x_position >> 3) as usize;
                let map_y = (self.y_position >> 3) as usize;

                if mouse_data.buttons == 0 { return; }

                if map_x < 8 && map_y < 8 {
                    let loc = &mut gpu.map[map_y * 64 + map_x];
                    *loc = self.current_palette_index;
        
                    let p_sprite_byte = &mut gpu.tile[5][
                        (map_y * 2) + 
                        (map_x >> 2) ];
        
                    let shift_amt = (0x3 - (map_x & 0x3)) << 1;
        
                    *p_sprite_byte = 
                        (*p_sprite_byte & !(0x3 << shift_amt)) |
                        ((*loc as u8 & 0x3) << shift_amt);
                }
        
                if map_x > 17 && map_y < 8 {
                    gpu.map[map_y * 64 + map_x] = 0x05;

                    gpu.attr[map_y * 32 + (map_x >> 1)] &=
                        if (map_x & 1) != 0 {
                            0xF0
                        } else {
                            0x0F
                        };
                }
        
                let mut slider_hit = false;

                for slider in &mut self.sliders {
                    if map_x > 0 &&
                       map_x < 31 &&
                       map_y as i32 == slider.position {
                        slider.set(map_x as i32);
                        slider_hit = true;
                        slider.draw(gpu);

                        break;
                    } 
                }

                if slider_hit {
                    self.update_red_value(gpu, self.sliders[0].get());
                    self.update_green_value(gpu, self.sliders[1].get());
                    self.update_blue_value(gpu, self.sliders[2].get());
                }
       
                for i in 0..4 {
        
                    if map_y == 7 && map_x == (10 + (2 * i)) {
        
                        gpu.map[64 * 6 + 10 + (2 * self.current_palette_index)] = 0x07;
                        gpu.attr[32 * 6 + 5 + self.current_palette_index] = 0x11;
        
                        gpu.map[64 * 6 + 10 + (2 * i)] = 0x0A;
                        gpu.attr[32 * 6 + 5 + i] = 0x11;
        
                        self.current_palette_index = i;
        
                        let color = self.get_current_color(gpu) as i32;

                        let red_percent = ((color >> 0) & 0xFF) as f32 / 255.0;
                        let green_percent = ((color >> 8) & 0xFF) as f32 / 255.0;
                        let blue_percent = ((color >> 16) & 0xFF) as f32 / 255.0;

                        let red_slider_value = (red_percent * 30.0).ceil() as i32;
                        let green_slider_value = (green_percent * 30.0).ceil() as i32;
                        let blue_slider_value = (blue_percent * 30.0).ceil() as i32;
        
                        self.sliders[0].set(red_slider_value);
                        self.sliders[1].set(green_slider_value);
                        self.sliders[2].set(blue_slider_value);

                        self.sliders[0].draw(gpu);
                        self.sliders[1].draw(gpu);
                        self.sliders[2].draw(gpu);

                        break;
                    }
                }
            },
            Event::KeyEvent(key_data) => {
            }
        }
    }

    pub fn step_logic(&mut self, gpu: &mut Gpu) {
        gpu.sprites[0] =
            (gpu.sprites[0] & 0x0000FFFF)
            | ((self.x_position & 0xFF) << 24)
            | ((self.y_position & 0xFF) << 16);
    }

}