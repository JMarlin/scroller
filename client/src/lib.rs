use wasm_bindgen::prelude::*;
use web_sys::HtmlCanvasElement;
use std::rc::Rc;
use std::cell::RefCell;

mod game;
mod gpu;

use game::{Game, Event, MouseEventData};
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

fn register_mouse(f: &JsValue) {
    web_sys::window().unwrap().add_event_listener_with_callback("mousemove", f.as_ref().unchecked_ref())
        .expect("Failed to register mouse handler");

    web_sys::window().unwrap().add_event_listener_with_callback("mousedown", f.as_ref().unchecked_ref())
        .expect("Failed to register mouse handler");
}

fn register_resize(f: &JsValue) {
    web_sys::window().unwrap().add_event_listener_with_callback("resize", f.as_ref().unchecked_ref())
        .expect("Failed to register resize handler");
}

fn print_to_console(content: &str) {
    let args = js_sys::Array::new();
    
    args.push(&wasm_bindgen::JsValue::from_str(content));
    web_sys::console::log(&args);
}

fn fit_canvas_to_window(canvas: &HtmlCanvasElement) {
    let window = web_sys::window().unwrap();
    let inner_height = window.inner_height().unwrap().as_f64().unwrap();
    let inner_width = window.inner_width().unwrap().as_f64().unwrap();
    let display_ratio = 256.0/240.0;
    let window_ratio = inner_width / inner_height;
        
    if display_ratio < window_ratio {
        
        canvas.style().set_property("top", "0px")
            .expect("Unable to set 'top' canvas property");

        canvas.style()
            .set_property(
                "height",
                &format!("{inner_height}px") )
            .expect("Unable to set 'top' canvas property");

        let canvas_width = inner_height * display_ratio;

        canvas.style()
            .set_property("width",
                &format!("{canvas_width}px") )
            .expect("Unable to set 'width' canvas property");

        let left_offset = (inner_width - canvas_width) / 2.0;

        canvas.style()
            .set_property(
                "left",
                &format!("{left_offset}px") )
            .expect("Unable to set 'left' canvas property");
    } else {

        canvas.style().set_property("left", "0px")
            .expect("Unable to set 'left' canvas property");

        canvas.style()
            .set_property(
                "width",
                &format!("{inner_width}px") )
            .expect("Unable to set 'width' canvas property");

        let canvas_height = inner_width / display_ratio;

        canvas.style()
            .set_property(
                "height",
                &format!("{canvas_height}px") )
            .expect("Unable to set 'height' canvas property");

        let top_offset = (inner_height - canvas_height) / 2.0;

        canvas.style()
            .set_property(
                "top", 
                &format!("{top_offset}px") )
            .expect("Unable to set 'top' canvas property");
    }
}

fn mouse_handler(event: web_sys::MouseEvent, game: &mut Game, gpu: &mut Gpu, canvas: &HtmlCanvasElement) {

    let canvas_height = canvas.offset_height() as f32;
    let canvas_top = canvas.offset_top() as f32;
    let canvas_width = canvas.offset_width() as f32;
    let canvas_left = canvas.offset_left() as f32;
    let scaled_position_x = (((event.x() as f32) - canvas_left) / canvas_width) * 256.0;
    let scaled_position_y = (((event.y() as f32) - canvas_top) / canvas_height) * 240.0;

    let buttons = event.buttons() as u32;

    game.handle_event(
        gpu,
        Event::MouseEvent(MouseEventData {
            x: scaled_position_x as u32,
            y: scaled_position_y as u32,
            buttons: buttons } ) );
}

#[wasm_bindgen(start)]
fn run() {

    set_panic_hook();

    let mut fb = [0xFFu8; 256 * 4 * 240];
    let document = web_sys::window().unwrap().document().unwrap();

    let canvas = document
        .create_element("canvas")
        .expect("Couldn't create canvas")
        .dyn_into::<web_sys::HtmlCanvasElement>()
        .unwrap();

    let body = document.body().unwrap();

    body.append_child(&canvas)
        .expect("Couldn't append canvas");

    canvas.set_width(256);
    canvas.set_height(240);

    canvas.style().set_property("position", "absolute")
        .expect("Unable to set canvas 'position' property");

    body.style()
        .set_property("background-color", "black")
        .expect("Unable to set body 'background-color' property");

    canvas.style()
        .set_property("cursor", "none")
        .expect("Unable to set canvas 'cursor' property");

    fit_canvas_to_window(&canvas);

    let render_callback_ref_a = Rc::new(RefCell::new(None));
    let render_callback_ref_b = render_callback_ref_a.clone();

    let context = canvas
        .get_context("2d")
        .expect("Couldn't get canvas context")
        .unwrap()
        .dyn_into::<web_sys::CanvasRenderingContext2d>()
        .unwrap();

    let mut gpu = Gpu::new();
    let mut game = Game::new();

    game.init(&mut gpu);

    let gpu_ref_a = Rc::new(RefCell::new(gpu));
    let gpu_ref_b = gpu_ref_a.clone();
    let game_ref_a = Rc::new(RefCell::new(game));
    let game_ref_b = game_ref_a.clone();
    let canvas_ref_a = Rc::new(RefCell::new(canvas));
    let canvas_ref_b = canvas_ref_a.clone();

    *render_callback_ref_b.borrow_mut() = Some(Closure::new(move || {

        (*game_ref_a).borrow_mut().step_logic(&mut *(*gpu_ref_a).borrow_mut());
        (*gpu_ref_a).borrow().render_to(&mut fb);

        let canv_data = web_sys::ImageData::new_with_u8_clamped_array(wasm_bindgen::Clamped(&fb), 256).unwrap();
        let _ = context.put_image_data(&canv_data, 0.0, 0.0);

        next_frame(render_callback_ref_a.borrow().as_ref().unwrap());
    }));

    let mouse_callback = Closure::<dyn FnMut(web_sys::MouseEvent)>::new(move |e: web_sys::MouseEvent| {
        mouse_handler(e, &mut *game_ref_b.borrow_mut(), &mut *gpu_ref_b.borrow_mut(), &*canvas_ref_a.borrow());
    }).into_js_value();

    let mouse_callback_ref = Rc::new(RefCell::new(Some(mouse_callback)));

    let resize_callback = Closure::<dyn FnMut(web_sys::UiEvent)>::new(move |_e: web_sys::UiEvent| {
        fit_canvas_to_window(&*canvas_ref_b.borrow());
    }).into_js_value();

    let resize_callback_ref = Rc::new(RefCell::new(Some(resize_callback)));

    register_resize(resize_callback_ref.borrow().as_ref().unwrap());
    register_mouse(mouse_callback_ref.borrow().as_ref().unwrap());
    next_frame(render_callback_ref_b.borrow().as_ref().unwrap());
}
