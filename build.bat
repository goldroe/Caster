@echo off

set MODE=%1
if [%1]==[] (
  set MODE=debug
)

if not exist build mkdir build
set WARNING_FLAGS=/W4 /wd4100 /wd4101 /wd4189 /wd4201 /wd4456 /wd4505 /wd4706 /wd4127
set INCLUDES=/Isrc\ /Iext\ /Iext\freetype\include\
set COMMON_COMPILER_FLAGS=/nologo /FC /EHsc /Fdbuild\ /Fobuild\ %WARNING_FLAGS% %INCLUDES% /std:c++17

set COMPILER_FLAGS=%COMMON_COMPILER_FLAGS%
if %MODE%==release (
  set COMPILER_FLAGS=/O2 /D /MD %COMPILER_FLAGS%
) else if %mode%==debug (
  set COMPILER_FLAGS=/Zi /Od /MDd %COMPILER_FLAGS%
) else (
  echo Unkown build mode
  exit /B 2
)

set LIBS=/LIBPATH:ext\freetype\libs\x64\Release\ user32.lib shell32.lib kernel32.lib winmm.lib shlwapi.lib freetype.lib
set COMMON_LINKER_FLAGS=/INCREMENTAL:NO /OPT:REF
set LINKER_FLAGS=%COMMON_LINKER_FLAGS% %LIBS%

:: Build Project
CL %COMPILER_FLAGS% src\caster\caster_main.cpp /FeCaster.exe /link %LINKER_FLAGS% || exit /b 1
