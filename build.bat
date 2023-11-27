@echo off
if not exist ".\build\" mkdir "build"
call vcvars.bat
set LIBRARIES=glfw3.lib opengl32.lib user32.lib gdi32.lib shell32.lib
pushd .\build
cl /MT /Zi /Od /EHsc -nologo ../code/main.cpp ../Include/glad/glad.c /I ..\Include /link /ENTRY:wmainCRTStartup /SUBSYSTEM:CONSOLE /LIBPATH:..\Libraries\ %LIBRARIES%
popd
