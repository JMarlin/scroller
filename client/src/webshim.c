#include "webshim.h"
#include "event.h"
#include <emscripten.h>
#include <stdlib.h>

EventDispatchProcedure g_event_proc = 0;

void WS_InitEvents() {

    #warning Not Implemented: WS_InitEvents
}

RenderProcedure g_render_proc = 0;

void do_render_loop_proc() {

    g_render_proc();
}

double get_canvas_width() {
    return EM_ASM_DOUBLE({
        return Module.display_list[0].canvas.offsetWidth;
    }, 0);
}

void do_mouse_event(int x, int y, int buttons) {

	if(!g_event_proc) return;

    double scale = 256.0 / get_canvas_width();
    MouseEvent mouse_event =  {
        .type = MOUSE,
        .x = (int)(scale * x),
        .y = (int)(scale * y),
        .buttons = buttons
    };
 
    g_event_proc(&mouse_event);
}

void do_key_event(int isUp, int code) {

	if(!g_event_proc) return;

    KeyEvent key_event =  {
        .type = KEY,
        .isUp = isUp,
        .code = code
    };
 
    g_event_proc(&key_event);
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
        Module.do_mouse_event = Module.do_mouse_event || Module.cwrap('do_mouse_event', 0, ['number', 'number', 'number']);
        Module.do_key_event = Module.do_key_event || Module.cwrap('do_key_event', 0, ['number', 'number']);
    );
}

WS_Display WS_CreateDisplay(uint32_t w, uint32_t h) {

    WS_Display display;

    display.w = w;
    display.h = h;
    #warning Not error handled: fb malloc
    display.fb = (uint32_t*)malloc(w * h * 4);
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

            var display_ratio = 256.0/240.0;

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
        new_canvas.addEventListener('mousemove', function(e) { Module.do_mouse_event(e.offsetX, e.offsetY, e.buttons) });
        window.addEventListener('keydown', function(e) { Module.do_key_event(0, e.keyCode) } );
        window.addEventListener('keyup', function(e) { Module.do_key_event(1, e.keyCode) } );

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

