all:
	mkdir -p ../../tmp.out/gcc/Release/LuaPlugin/
	g++ -g -fpermissive -O3 -std=c++0x -Wall -DNDEBUG -DUNICODE -D_UNICODE -I../../include -I../lua/src -c ../LuaManager.cpp -o ../../tmp.out/gcc/Release/LuaPlugin/LuaManager.o
	g++ -g -fpermissive -O3 -std=c++0x -Wall -DNDEBUG -DUNICODE -D_UNICODE -I../../include -I../lua/src -c ../stdafx.cpp -o ../../tmp.out/gcc/Release/LuaPlugin/stdafx.o
	g++ -g -fpermissive -O3 -std=c++0x -Wall -DNDEBUG -DUNICODE -D_UNICODE -I../../include -I../lua/src -c ../main.cpp -o ../../tmp.out/gcc/Release/LuaPlugin/main.o
	g++ -g -fpermissive -O3 -std=c++0x -Wall -DNDEBUG -DUNICODE -D_UNICODE -I../../include -I../lua/src -c ../Plugin.cpp -o ../../tmp.out/gcc/Release/LuaPlugin/LuaPlugin.o
	g++ -g -fpermissive -O3 -std=c++0x -Wall -DNDEBUG -DUNICODE -D_UNICODE -I../../include -I../lua/src -c ../ScriptInstance.cpp -o ../../tmp.out/gcc/Release/LuaPlugin/ScriptInstance.o
	g++ -g -fpermissive -O3 -std=c++0x -Wall -DNDEBUG -DUNICODE -D_UNICODE -I../../include -I../lua/src -c ../Util.cpp -o ../../tmp.out/gcc/Release/LuaPlugin/Util.o
	mkdir -p ../../compiled/LuaPlugin/
	g++ -fPIC -pedantic -g -shared -dll ../../tmp.out/gcc/Release/LuaPlugin/*.* -o ../../compiled/LuaPlugin/LuaPlugin.so -llua -L../lua/src/ -s
	

