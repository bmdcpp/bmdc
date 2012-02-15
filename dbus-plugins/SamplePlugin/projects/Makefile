all:
	mkdir -p ../../tmp.out/gcc/Release/SamplePlugin/
	g++ -O3 -Wall -DNDEBUG -DUNICODE -D_UNICODE `pkg-config --libs --cflags gtk+-2.0` -I../../include -c ../main.c -o ../../tmp.out/gcc/Release/SamplePlugin/main.o
	g++ -O3 -Wall -DNDEBUG -DUNICODE -D_UNICODE `pkg-config --libs --cflags gtk+-2.0` -I../../include -c ../stdafx.c -o ../../tmp.out/gcc/Release/SamplePlugin/stdafx.o
	g++ -O3 -Wall -DNDEBUG -DUNICODE -D_UNICODE `pkg-config --libs --cflags gtk+-2.0` -I../../include -c ../Plugin.c -o ../../tmp.out/gcc/Release/SamplePlugin/Plugin.o
	g++ -shared -dll `pkg-config --libs --cflags gtk+-2.0` ../../tmp.out/gcc/Release/SamplePlugin/*.* -o ../../compiled/SamplePlugin/SamplePlugin.so -s

Debug:
	mkdir -p ../../tmp.out/gcc/Debug/SamplePlugin/
	g++ -O3 -Wall -DDEBUG -DUNICODE -D_UNICODE `pkg-config --libs --cflags gtk+-2.0` -I../../include -c ../main.c -o ../../tmp.out/gcc/Debug/SamplePlugin/main.o
	g++ -O3 -Wall -DDEBUG -DUNICODE -D_UNICODE `pkg-config --libs --cflags gtk+-2.0` -I../../include -c ../stdafx.c -o ../../tmp.out/gcc/Debug/SamplePlugin/stdafx.o
	g++ -O3 -Wall -DDEBUG -DUNICODE -D_UNICODE `pkg-config --libs --cflags gtk+-2.0` -I../../include -c ../Plugin.c -o ../../tmp.out/gcc/Debug/SamplePlugin/Plugin.o
	g++ -shared -dll `pkg-config --libs --cflags gtk+-2.0` ../../tmp.out/gcc/Release/SamplePlugin/*.* -o ../../compiled/SamplePlugin/SamplePlugin.so -s	
