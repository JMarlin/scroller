#include "webshim.h"
#include <emscripten.h>
#include <stdlib.h>

EventDispatchProcedure g_event_proc = 0;

int last_mouse_x = 0;
int last_mouse_y = 0;
int last_mouse_buttons = 0;

void WS_InitEvents() {

    #warning Not Implemented: WS_InitEvents
}

RenderProcedure g_render_proc = 0;

void do_render_loop_proc() {

    g_render_proc();
}

void do_event_proc(int x, int y, int buttons) {
	last_mouse_x = x;
	last_mouse_y = y;
	last_mouse_buttons = buttons;
	
	g_event_proc();
}

void WS_GetMouse(int* x, int* y, int* buttons) {
	*x = last_mouse_x;
	*y = last_mouse_y;
	*buttons = last_mouse_buttons;
}

void WS_SetRenderLoopProc(RenderProcedure render_proc) {

    g_render_proc = render_proc;

    EM_ASM_(
        Module.do_render_proc = Module.cwrap('do_render_loop_proc');
    );
}

void WS_StartRenderLoop() {

     EM_ASM_(

         if(!!Module.render_interval) {

             return;
         }

        //Start render loop
        Module.render_interval = setInterval(function() {

            Module.do_render_proc();
        }, 0);
     );
}

void WS_StartEventDispatch(EventDispatchProcedure dispatch_proc) {

    g_event_proc = dispatch_proc;

    EM_ASM_(
        Module.do_event_proc = Module.cwrap('do_event_proc', null, ['number', 'number']);
    );

    //TODO: Dispatch browser events to the dispatch_proc
    //      shutdown application if dispatch_proc returns 1
    #warning Not Implemented: WS_StartEventDispatch
    #warning Not Implemented: Event type needs to be created and passed to WS_StartEventDispatch
}

WS_Display WS_CreateDisplay(uint32_t w, uint32_t h) {

    WS_Display display;

    display.w = w;
    display.h = h;
    #warning Not error handled: fb malloc
    display.fb = (uint8_t*)malloc(w * h * 4);
    display.id = EM_ASM_INT({

        if(!Module.display_list) {

            Module.display_list = [];
        }

        return Module.display_list.length
    }, 0);

    EM_ASM_({

        var new_canvas = document.createElement('canvas');

        document.body.style.margin = '0px';
        document.body.style.backgroundColor = '#000';
        
        var resizer = (e) => {

            var display_ratio = 320.0/240.0;

            var window_ratio = 
                window.innerWidth / window.innerHeight;
            
            if(display_ratio < window_ratio) {
                
                new_canvas.style.top = "0px";

                new_canvas.style.height = 
                    "" + window.innerHeight + "px";

                var canvas_width =
                    (window.innerHeight * display_ratio);

                new_canvas.style.width =
                    "" + canvas_width + "px";

                new_canvas.style.left =
                    "" + ((window.innerWidth - canvas_width) / 2) + "px";
            } else {

                new_canvas.style.left = "0px";

                new_canvas.style.width = 
                    "" + window.innerWidth + "px";

                var canvas_height = 
                    (window.innerWidth / display_ratio);

                new_canvas.style.height =
                    "" + canvas_height + "px";

                new_canvas.style.top =
                    "" + ((window.innerHeight - canvas_height) / 2) + "px";
            }
        };

        window.addEventListener('resize', resizer);
        new_canvas.addEventListener('mousemove', e => {
            const scale = 320 / parseInt(new_canvas.style.width);
            const new_coords = {
                x: e.offsetX * scale,
                y: e.offsetY * scale
            };
            
            Module.do_event_proc(new_coords.x, new_coords.y, e.buttons);
        });

        new_canvas.style.cursor = 'none';
        new_canvas.style.position = 'absolute';
        new_canvas.width = $0;
        new_canvas.height = $1;
        
        document.body.appendChild(new_canvas);

        const display_fb_ptr = $2;
        
        Module.display_list.push({
            canvas: new_canvas,
            framebuf: Module.HEAPU8.subarray(display_fb_ptr, display_fb_ptr + ($0 * $1 * 4)) 
        });

        resizer(null);

    }, display.w, display.h, display.fb);

    return display;
}

void WS_DestroyDisplay(WS_Display display) {

    #warning Not Implemented: WS_DestroyDisplay
}

void WS_ClearDisplay(WS_Display display) {

    #warning Not Implemented: WS_ClearDisplay
}

void WS_StopRenderLoop() {

    EM_ASM_(

        if(!!Module.render_interval) {

            clearInterval(Module.render_interval);
            Module.render_interval = null;
        }
    );
}

void WS_SubmitDisplay(WS_Display display) {

    EM_ASM({

        var display_id = $0;
        var display_w = $1;
        var display_h = $2;

        var disp_data = Module.display_list[display_id];
        var canvas = disp_data.canvas;
        var fb = disp_data.framebuf;
        var ctx = canvas.getContext('2d');
        var canv_data = ctx.getImageData(0, 0, display_w, display_h);

        //Create an unsigned byte subarray  
        canv_data.data.set(fb); 
        ctx.putImageData(canv_data, 0, 0);
    },
        display.id, display.w, display.h
    );
}

