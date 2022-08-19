server: main.o http.o bufloader.o expandobuf.o util.o webobject.o
	gcc -o ../dist/server/server main.o http.o bufloader.o expandobuf.o util.o webobject.o -g

jsonbin: jsonmain.o
	gcc -o bin/jsonbin

main.o: src/main.c src/webobject.h src/router.h src/http.h src/util.h
	gcc -c -o main.o src/main.c -g

http.o: src/http.c src/http.h src/expandobuf.h src/bufloader.h src/util.h
	gcc -c -o http.o src/http.c -g

bufloader.o: src/bufloader.c src/bufloader.h
	gcc -c -o bufloader.o src/bufloader.c -g

expandobuf.o: src/expandobuf.c src/expandobuf.h
	gcc -c -o expandobuf.o src/expandobuf.c -g

util.o: src/util.c src/util.h
	gcc -c -o util.o src/util.c -g

webobject.o: src/webobject.c src/webobject.h src/http.h src/util.h
	gcc -c -o webobject.o src/webobject.c -g