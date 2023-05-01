pub type Sprite = u32;
pub type AttributePair = u8;
pub type Attribute = u8;
pub type VideoAddress = u16;
pub type Tile = [u8; 16];

pub struct Gpu {
    pub map: [usize; 64 * 60],
    pub attr: [AttributePair; 32 * 60],
    pub tile: [Tile; 256],
    pub palette: [[u32; 4]; 3],
    pub sprites: [Sprite; 32],
    pub bg_color: u32
}

trait AttributeExt {
    fn get_palette(self) -> usize;
    fn is_h_mirrored(self) -> bool;
    fn is_v_mirrored(self) -> bool;
}

impl AttributeExt for Attribute {
    fn get_palette(self) -> usize {
        (self & 0x03) as usize
    }

    fn is_h_mirrored(self) -> bool {
        (self & 0x04) != 0
    }

    fn is_v_mirrored(self) -> bool {
        (self & 0x08) != 0
    }
}

trait SpriteExt {
    fn get_x(self) -> u8;
    fn get_y(self) -> u8;
    fn is_active(self) -> bool;
    fn get_tile_index(self) -> usize;
    fn get_attribute(self) -> Attribute;
}

impl SpriteExt for Sprite {
    fn get_x(self) -> u8 {
        ((self >> 24) & 0xFF) as u8
    }

    fn get_y(self) -> u8 {
        ((self >> 16) & 0xFF) as u8
    }

    fn is_active(self) -> bool {
        (self & 0x10) != 0
    }

    fn get_tile_index(self) -> usize {
        ((self >> 8) & 0xFF) as usize
    }

    fn get_attribute(self) -> Attribute {
        (self & 0x0F) as Attribute
    }
}

trait VideoAddressExt {
    fn get_xpos(self) -> u8;
    fn get_ypos(self) -> u8;
    fn to_map_index(self) -> usize;
}

impl VideoAddressExt for VideoAddress {
    fn get_xpos(self) -> u8 {
        (self & 0xFF) as u8
    }

    fn get_ypos(self) -> u8 {
        ((self >> 8) & 0xFF) as u8
    }

    //TODO: Add x and y scrolling
    fn to_map_index(self) -> usize {
        (
            ((self >> 5) & 0xFFC0) |
            ((self >> 3) & 0x001F)
        ) as usize
    }
}

impl Gpu {
    pub fn new() -> Gpu {
        Gpu {
            map: [0; 64 * 60],
            attr: [0; 32 * 60],
            tile: [[0; 16]; 256],
            palette: [[0xFF000000; 4]; 3],
            sprites: [0; 32],
            bg_color: 0xFF000000
        }
    }

    fn video_address_to_sprite_index(&self, sprite_found: &mut bool, addr: VideoAddress) -> usize {
        *sprite_found = true;

        let xpos = addr.get_xpos();
        let ypos = addr.get_ypos();

        //TODO: Support multi-tile sprites
        for i in 0..self.sprites.len() {
            let sprite = self.sprites.get(i).unwrap();
            
            if sprite.is_active() &&
               xpos >= sprite.get_x() &&
               xpos < (sprite.get_x() + 8) &&
               ypos >= sprite.get_y() &&
               ypos < (sprite.get_y() + 8) {
            
                return i;
            }
        }

        *sprite_found = false;

        0
    }

    //TODO: Add x and y scrolling
    fn tile_byte_offset_from_coordinates_and_attribute(x: u8, y: u8, attr: Attribute) -> usize {
        let tile_x_half = ((x & 0x04) >> 2) ^ if attr.is_h_mirrored() { 0x01 } else { 0x00 };
        let tile_y_offset = ((y & 0x07) ^ if attr.is_v_mirrored() { 0x07 } else { 0x00 }) << 1;

        (tile_y_offset | tile_x_half) as usize
    }

    fn tile_byte_from_tile_map_index_coordinates_and_attribute(&self, tile_index: usize, x: u8, y: u8, attr: Attribute) -> u8 {
        self.tile[tile_index][Gpu::tile_byte_offset_from_coordinates_and_attribute(x, y, attr)]
    }

    fn tile_pixel_palette_index_from_tile_byte_coordinates_and_attribute(tile_byte: u8, x: u8, y: u8, attr: Attribute) -> usize {
        let tile_x_subaddr = ((x & 0x03) ^ if attr.is_h_mirrored() { 0x03 } else { 0x00 }) << 1;

        ((tile_byte >> (6 - tile_x_subaddr)) & 0x03) as usize
    }

    fn sprite_pixel_index_from_index_and_video_address(&self, sprite_index: usize, addr: VideoAddress) -> usize {
        let sprite = self.sprites[sprite_index];
        let virtual_x = addr.get_xpos() - sprite.get_x();
        let virtual_y = addr.get_ypos() - sprite.get_y();

        let tile_byte = self.tile_byte_from_tile_map_index_coordinates_and_attribute(
            sprite.get_tile_index(),
            virtual_x,
            virtual_y,
            sprite.get_attribute()
        );

        Gpu::tile_pixel_palette_index_from_tile_byte_coordinates_and_attribute(tile_byte, virtual_x, virtual_y, sprite.get_attribute())
    }

    fn attribute_from_tile_map_index(&self, tile_map_index: usize) -> Attribute {
        (self.attr[tile_map_index >> 1] >> ((1 - (tile_map_index & 0x01)) << 2)) & 0x0F
    }

    fn tile_pixel_index_from_tile_and_video_address(&self, tile_map_index: usize, addr: VideoAddress) -> usize {
        let attribute = self.attribute_from_tile_map_index(tile_map_index);
        let tile_byte = self.tile_byte_from_tile_map_index_coordinates_and_attribute(self.map[tile_map_index], addr.get_xpos(), addr.get_ypos(), attribute);

        Gpu::tile_pixel_palette_index_from_tile_byte_coordinates_and_attribute(tile_byte, addr.get_xpos(), addr.get_ypos(), attribute)
    }

    fn pixel_color_from_palette_index_and_attribute(&self, tile_pixel_palette_index: usize, attr: Attribute) -> u32 {
        if tile_pixel_palette_index == 0 {
            self.bg_color
        } else {
            self.palette[attr.get_palette()][tile_pixel_palette_index - 1]
        }
    }

    fn look_up_pixel(&self, addr: VideoAddress) -> u32 {
        let mut sprite_found = false;
        let sprite_index = self.video_address_to_sprite_index(&mut sprite_found, addr);

        let pixel_index = if sprite_found {
            self.sprite_pixel_index_from_index_and_video_address(sprite_index, addr)
        } else {
            0
        };

        let is_sprite_pixel = pixel_index != 0;

        let tile_map_index = if is_sprite_pixel {
            0
        } else {
            addr.to_map_index()
        };

        let pixel_index = if pixel_index != 0 {
            pixel_index
        } else {
            self.tile_pixel_index_from_tile_and_video_address(tile_map_index, addr)
        };

        self.pixel_color_from_palette_index_and_attribute(
            pixel_index,
            if is_sprite_pixel {
                self.sprites[sprite_index].get_attribute()
            } else {
                self.attribute_from_tile_map_index(tile_map_index)
            }
        )
    }

    pub fn render_to(&self, fb: &mut [u8]) {
        let mut addr = 0u32;
        let mut u32_pixel_val = 0u32;

        for pixel in fb.iter_mut() {

            let sub_addr = addr & 0x3;

            if sub_addr == 0 {
                 u32_pixel_val = self.look_up_pixel((addr >> 2) as VideoAddress);
            }

            *pixel = ((u32_pixel_val >> (8 * sub_addr)) & 0xFF) as u8;
            addr = addr + 1;
        }
    }
}