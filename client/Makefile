test: test.o gpu.o
	gcc -o test test.o gpu.o -g
	./test
	rm *.o
	rm test

test.o: src/test.c src/gpu.h
	gcc -c -o test.o src/test.c -g -fPIC

gpu.o: src/gpu.c src/gpu.h
	gcc -c -o gpu.o src/gpu.c -g -fPIC

client: game.so main.bc gpu.bc webshim.bc module.bc src/index.html 
	cat src/index.html | envsubst > ../dist/client/index.html
	emcc -o ../dist/client/client.js -sMAIN_MODULE -sASYNCIFY=1 -sEXPORTED_RUNTIME_METHODS=['cwrap'] main.bc webshim.bc gpu.bc module.bc --embed-file game.so

game.so: gamesrc/main.c src/gpu.h
	emcc -c -o game.bc gamesrc/main.c -g -fPIC
	emcc -o game.so -sSIDE_MODULE game.bc

main.bc: src/main.c src/webshim.h src/gpu.h src/event.h
	emcc -c -o main.bc src/main.c -g -fPIC

gpu.bc: src/gpu.c src/gpu.h
	emcc -c -o gpu.bc src/gpu.c -g -fPIC

webshim.bc: src/webshim.c src/event.h
	emcc -c -o webshim.bc src/webshim.c -g -fPIC

module.bc: src/module.c src/module.h
	emcc -c -o module.bc src/module.c -g -fPIC
