test: test.o gpu.o
	gcc -o test test.o gpu.o -g
	./test
	rm *.o
	rm test

test.o: src/test.c src/gpu.h
	gcc -c -o test.o src/test.c -g

gpu.o: src/gpu.c src/gpu.h
	gcc -c -o gpu.o src/gpu.c -g

client: main.bc gpu.bc webshim.bc src/index.html 
	cp src/index.html ../dist/client/
	emcc -o ../dist/client/client.js -sEXPORTED_FUNCTIONS=['_main','_do_render_loop_proc','_do_event_proc'] -sEXPORTED_RUNTIME_METHODS=['cwrap'] main.bc webshim.bc

main.bc: src/main.c src/webshim.h src/gpu.h
	emcc -c -o main.bc src/main.c -g

gpu.bc: src/gpu.c src/gpu.h
	emcc -c -o gpu.bc src/gpu.c -g

webshim.bc: src/webshim.c
	emcc -c -o webshim.bc src/webshim.c -g