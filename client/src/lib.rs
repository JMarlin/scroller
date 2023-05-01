use wasm_bindgen::prelude::*;

mod game;
mod gpu;

use game::Game;
use gpu::Gpu;

// When the `wee_alloc` feature is enabled, use `wee_alloc` as the global
// allocator.
#[cfg(feature = "wee_alloc")]
#[global_allocator]
static ALLOC: wee_alloc::WeeAlloc = wee_alloc::WeeAlloc::INIT;

pub fn set_panic_hook() {
    // When the `console_error_panic_hook` feature is enabled, we can call the
    // `set_panic_hook` function at least once during initialization, and then
    // we will get better error messages if our code ever panics.
    //
    // For more details see
    // https://github.com/rustwasm/console_error_panic_hook#readme
    #[cfg(feature = "console_error_panic_hook")]
    console_error_panic_hook::set_once();
}

fn next_frame(f: &Closure<dyn FnMut()>) {
    web_sys::window().unwrap().set_timeout_with_callback(f.as_ref().unchecked_ref())
        .expect("Failed to request a frame");
}

#[wasm_bindgen(start)]
fn run() {

    let mut fb = [0xFFu8; 256 * 4 * 240];
    let document = web_sys::window().unwrap().document().unwrap();

    let canvas = document
        .create_element("canvas")
        .expect("Couldn't create canvas")
        .dyn_into::<web_sys::HtmlCanvasElement>()
        .unwrap();

    document.body().unwrap().append_child(&canvas)
        .expect("Couldn't append canvas");

    canvas.set_width(256);
    canvas.set_height(240);

    let f = std::rc::Rc::new(std::cell::RefCell::new(None));
    let g = f.clone();

    let context = canvas
        .get_context("2d")
        .expect("Couldn't get canvas context")
        .unwrap()
        .dyn_into::<web_sys::CanvasRenderingContext2d>()
        .unwrap();

    let mut gpu = Gpu::new();
    let mut game = Game::new();

    game.init(&mut gpu);

    *g.borrow_mut() = Some(Closure::new(move || {

        //game.step_logic(&mut gpu);
        gpu.render_to(&mut fb);

        let canv_data = web_sys::ImageData::new_with_u8_clamped_array(wasm_bindgen::Clamped(&fb), 256).unwrap();

        /*
        let args = js_sys::Array::new();
        
        args.push(&wasm_bindgen::JsValue::from_str(&format!("Value is: {:?}", fb[3])));
        web_sys::console::log(&args);
        */

        let _ = context.put_image_data(&canv_data, 0.0, 0.0);

        next_frame(f.borrow().as_ref().unwrap());
    }));

    next_frame(g.borrow().as_ref().unwrap());
}
