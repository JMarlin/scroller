api: main.bc webshim.bc src/index.html
	cp src/index.html ../dist/client/
	emcc -o ../dist/client/client.js -sEXPORTED_FUNCTIONS=['_main','_do_render_loop_proc'] -sEXPORTED_RUNTIME_METHODS=['cwrap'] main.bc webshim.bc

main.bc: src/main.c src/webshim.h
	emcc -c -o main.bc src/main.c -g

webshim.bc: src/webshim.c
	emcc -c -o webshim.bc src/webshim.c -g