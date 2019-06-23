@echo off
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
) else (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)

rem dependencies setup
set imgui_include= /I dependencies/imgui
set imgui_sources= dependencies/imgui/*.cpp
set sfml_include= /I dependencies

rem includes
set includes= /I include /I ../Includes %imgui_include% %sfml_include%

rem libraries
set lib_opencl= /LIBPATH lib/OpenCL.lib
set lib_opengl= /LIBPATH lib/OpenGL32.lib
set lib_sfml= /LIBPATH lib/sfml-graphics.lib /LIBPATH lib/sfml-system.lib /LIBPATH lib/sfml-window.lib
set libraries= %lib_opencl% %lib_opengl% %lib_sfml%

rem defines
set %defines%= /DENABLE_LOG

set compilerflags=/Od /Zi /EHsc /std:c++latest %defines%
set sources= src/*.cpp %imgui_sources%
set linkerflags=/OUT:bin\main.exe
cl.exe %compilerflags% %sources% %includes% /link %linkerflags% %libraries%
del bin\*.ilk *.obj *.pdb