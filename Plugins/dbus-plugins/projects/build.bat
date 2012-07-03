:: This is just something I made out of boredom ;)
:: Quick usage "build <mode>" from command line

@echo off
mkdir "../../compiled/SamplePlugin/" 2> nul

if "%1"=="release" goto mingw_release
if "%1"=="debug" goto mingw_debug

:interactive
cls
echo  Simple SamplePlugin mingw builder 
echo -----------------------------------
echo.
echo Choose build type:
echo   1. mingw-release
echo   2. mingw-debug
echo   3. quit
echo.
set /p userinp=Choice:
set userinp=%userinp:~0,13%
if "%userinp%"=="1" goto mingw_release
if "%userinp%"=="2" goto mingw_debug
if "%userinp%"=="3" goto quit
if "%userinp%"=="mingw-release" goto mingw_release
if "%userinp%"=="mingw-debug" goto mingw_debug
if "%userinp%"=="quit" goto quit
goto interactive

:mingw_release
mkdir "../../tmp.out/Win32/MinGW/Release/SamplePlugin/" 2> nul
windres -i ../SamplePlugin.rc -J rc -o ../../tmp.out/Win32/MinGW/Release/SamplePlugin/SamplePlugin.res -O coff 
gcc -O3 -Wall -DWIN32 -DNDEBUG -DUNICODE -D_UNICODE -I../../include -c ../main.c -o ../../tmp.out/Win32/MinGW/Release/SamplePlugin/main.o
gcc -O3 -Wall -DWIN32 -DNDEBUG -DUNICODE -D_UNICODE -I../../include -c ../stdafx.c -o ../../tmp.out/Win32/MinGW/Release/SamplePlugin/stdafx.o
gcc -O3 -Wall -DWIN32 -DNDEBUG -DUNICODE -D_UNICODE -I../../include -c ../Plugin.c -o ../../tmp.out/Win32/MinGW/Release/SamplePlugin/Plugin.o
g++ -shared -Wl,--output-def=../../compiled/SamplePlugin/libSamplePlugin.def -Wl,--dll ../../tmp.out/Win32/MinGW/Release/SamplePlugin/*.* -o ../../compiled/SamplePlugin\SamplePlugin.dll -s -Wl,--add-stdcall-alias
goto end

:mingw_debug
mkdir "../../tmp.out/Win32/MinGW/Debug/SamplePlugin/" 2> nul
windres -i ../SamplePlugin.rc -J rc -o ../../tmp.out/Win32/MinGW/Debug/SamplePlugin/SamplePlugin.res -O coff 
gcc -pedantic -Wall -g -O0 -DWIN32 -D_DEBUG -DUNICODE -D_UNICODE -I../../include -c ../main.c -o ../../tmp.out/Win32/MinGW/Debug/SamplePlugin/main.o
gcc -pedantic -Wall -g -O0 -DWIN32 -D_DEBUG -DUNICODE -D_UNICODE -I../../include -c ../stdafx.c -o ../../tmp.out/Win32/MinGW/Debug/SamplePlugin/stdafx.o
gcc -pedantic -Wall -g -O0 -DWIN32 -D_DEBUG -DUNICODE -D_UNICODE -I../../include -c ../Plugin.c -o ../../tmp.out/Win32/MinGW/Debug/SamplePlugin/Plugin.o
g++ -shared -Wl,--output-def=../../compiled/SamplePlugin/libSamplePlugin.def -Wl,--dll  ../../tmp.out/Win32/MinGW/Debug/SamplePlugin/*.* -o ../../compiled/SamplePlugin/SamplePlugin.dll -Wl,--add-stdcall-alias
goto end

:end
echo.
set /p userinp="Operation finished! - Do you wish to quit? <y/n>:"
set userinp=%userinp:~0,1%
if "%userinp%"=="n" goto interactive
if "%userinp%"=="y" goto quit

:quit
