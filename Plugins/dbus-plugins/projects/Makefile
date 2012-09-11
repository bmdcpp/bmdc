all:
	mkdir -p ../tmp.out/gcc/Release/SamplePlugin/
	mkdir -p ../compiled/SamplePlugin/
	gcc -fPIC -O3 -Wall -DNDEBUG -DUNICODE -D_UNICODE `pkg-config --libs --cflags gtk+-2.0` -I../include -c ../fsusage.c -o ../tmp.out/gcc/Release/SamplePlugin/fsusage.o
	g++ -fPIC -O3 -Wall -DNDEBUG -DUNICODE -D_UNICODE `pkg-config --libs --cflags gtk+-2.0` -I../include -c ../freespace.cpp -o ../tmp.out/gcc/Release/SamplePlugin/freespace.o
	g++ -fPIC -O3 -Wall -DNDEBUG -DUNICODE -D_UNICODE `pkg-config --libs --cflags gtk+-2.0` -I../include -c ../main.c -o ../tmp.out/gcc/Release/SamplePlugin/main.o
	g++ -fPIC -O3 -Wall -DNDEBUG -DUNICODE -D_UNICODE `pkg-config --libs --cflags gtk+-2.0` -I../include -c ../stdafx.c -o ../tmp.out/gcc/Release/SamplePlugin/stdafx.o
	g++ -fPIC -O3 -Wall -DNDEBUG -DUNICODE -D_UNICODE `pkg-config --libs --cflags gtk+-2.0` -I../include -c ../Plugin.c -o ../tmp.out/gcc/Release/SamplePlugin/Plugin.o
	g++ -fPIC -shared -dll `pkg-config --libs --cflags gtk+-2.0` ../tmp.out/gcc/Release/SamplePlugin/*.* -o ../compiled/SamplePlugin/SamplePlugin.so -s
Debug:
	mkdir -p ../tmp.out/gcc/Debug/SamplePlugin/
	mkdir -p ../compiled/SamplePlugin/
	g++ -fPIC -pedantic -Wall -g -O0 -D_DEBUG -DUNICODE -D_UNICODE -I../include -c ../main.c -o ../tmp.out/Win32/MinGW/Debug/SamplePlugin/main.o
	g++ -fPIC -pedantic -Wall -g -O0 -D_DEBUG -DUNICODE -D_UNICODE -I../include -c ../stdafx.c -o ../tmp.out/Win32/MinGW/Debug/SamplePlugin/stdafx.o
	g++ -fPIC -pedantic -Wall -g -O0 -D_DEBUG -DUNICODE -D_UNICODE -I../include -c ../Plugin.c -o ../tmp.out/Win32/MinGW/Debug/SamplePlugin/Plugin.o
	g++ -fPIC -shared -dll  ../../tmp.out/gcc/Debug/SamplePlugin/*.* -o ../compiled/SamplePlugin/SamplePlugin.so -s
