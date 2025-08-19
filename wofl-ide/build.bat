// ==================== build.bat (FIXED) ====================
@echo off
setlocal enabledelayedexpansion

echo ========================================
echo Building WOFL Ultra-Minimal IDE
echo ========================================
echo.

rem Check for Visual Studio environment
where cl >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: MSVC compiler not found!
    echo Please run this script from "x64 Native Tools Command Prompt for VS"
    exit /b 1
)

rem Configuration
set OUTPUT_DIR=dist
set EXE_NAME=wofl-ide.exe
set OPTIMIZATION=/O2
set WARNINGS=/W4

rem Create output directory
if not exist %OUTPUT_DIR% (
    echo Creating output directory...
    mkdir %OUTPUT_DIR%
)

rem Check which files actually exist
echo Checking for source files...
if not exist src\syntax_asm.c (
    echo WARNING: src\syntax_asm.c not found - skipping
)
if not exist src\theme_system.c (
    echo WARNING: src\theme_system.c not found - skipping
)
if not exist src\plugin_system.c (
    echo WARNING: src\plugin_system.c not found - skipping  
)
if not exist src\plugin_git.c (
    echo WARNING: src\plugin_git.c not found - skipping
)

rem Source files - only include files that exist
set CORE_SOURCES=src\main_win32.c src\editor_buffer.c src\editor_render.c

rem Original syntax files
set SYNTAX_SOURCES=src\syntax_core.c src\syntax_c.c src\syntax_py.c src\syntax_js.c
set SYNTAX_SOURCES=%SYNTAX_SOURCES% src\syntax_html.c src\syntax_css.c 
set SYNTAX_SOURCES=%SYNTAX_SOURCES% src\syntax_md.c src\syntax_go.c src\syntax_rs.c
set SYNTAX_SOURCES=%SYNTAX_SOURCES% src\syntax_sh.c src\syntax_lua.c

rem Check for syntax_json.c vs syntax_jason.c
if exist src\syntax_json.c (
    set SYNTAX_SOURCES=%SYNTAX_SOURCES% src\syntax_json.c
) else if exist src\syntax_jason.c (
    echo NOTE: Using syntax_jason.c
    set SYNTAX_SOURCES=%SYNTAX_SOURCES% src\syntax_jason.c
)

rem New syntax files (if they exist)
if exist src\syntax_cpp.c set SYNTAX_SOURCES=%SYNTAX_SOURCES% src\syntax_cpp.c
if exist src\syntax_csv.c set SYNTAX_SOURCES=%SYNTAX_SOURCES% src\syntax_csv.c

rem Feature files - check existence
set FEATURE_SOURCES=src\find.c src\run_exec_win32.c

rem Check for cmd_palette vs command_palette
if exist src\cmd_palette.c (
    set FEATURE_SOURCES=%FEATURE_SOURCES% src\cmd_palette.c
) else if exist src\command_palette.c (
    echo NOTE: Using command_palette.c
    set FEATURE_SOURCES=%FEATURE_SOURCES% src\command_palette.c
)

if exist src\config.c set FEATURE_SOURCES=%FEATURE_SOURCES% src\config.c

rem Combine all sources
set ALL_SOURCES=%CORE_SOURCES% %SYNTAX_SOURCES% %FEATURE_SOURCES%

rem Compiler flags - don't define WIN32_LEAN_AND_MEAN here since it's in editor.h
set CFLAGS=/nologo %WARNINGS% %OPTIMIZATION% /MT
set DEFINES=/DUNICODE /D_UNICODE
set LIBS=user32.lib gdi32.lib comdlg32.lib shell32.lib ole32.lib

echo.
echo Compiling...
echo.

rem Compile and link
cl %CFLAGS% %DEFINES% %ALL_SOURCES% /link /out:%OUTPUT_DIR%\%EXE_NAME% %LIBS%

if %errorlevel% neq 0 (
    echo.
    echo ========================================
    echo BUILD FAILED
    echo ========================================
    echo.
    echo Please check:
    echo 1. All source files exist in src\ directory
    echo 2. editor.h has matching #ifndef/#endif
    echo 3. File names match (cmd_palette.c vs command_palette.c)
    echo ========================================
    exit /b 1
)

rem Get file size
for %%A in (%OUTPUT_DIR%\%EXE_NAME%) do set SIZE=%%~zA
set /a SIZE_KB=%SIZE% / 1024

echo.
echo ========================================
echo BUILD SUCCESSFUL
echo ========================================
echo Output: %OUTPUT_DIR%\%EXE_NAME%
echo Size: %SIZE_KB% KB
echo.
echo Run with: %OUTPUT_DIR%\%EXE_NAME% [filename]
echo ========================================

rem Clean up .obj files
del *.obj 2>nul

endlocal

// ==================== File Renaming Guide ====================
/*
Based on the errors, it looks like you have some file naming inconsistencies.
Here's what needs to be fixed:

RENAME THESE FILES (if they exist with wrong names):
1. src\command_palette.c → src\cmd_palette.c
2. src\syntax_jason.c → src\syntax_json.c

CREATE THESE NEW FILES (from the artifacts I provided):
1. src\syntax_asm.c (from the syntax_asm.c section)
2. src\syntax_cpp.c (from the syntax_cpp.c section)
3. src\syntax_csv.c (from the syntax_csv.c section)
4. src\theme_system.c (optional - from theme_system.c section)
5. src\theme_system.h (optional - from theme_system.h section)
6. src\plugin_system.c (optional - from plugin_system.c section)
7. src\plugin_system.h (optional - from plugin_system.h section)
8. src\plugin_git.c (optional - from plugin_git.c section)

MINIMAL BUILD (without plugins/themes):
If you just want to get it building first, use this simpler command:

cl /nologo /W4 /O2 /MT /DUNICODE /D_UNICODE ^
  src\main_win32.c src\editor_buffer.c src\editor_render.c ^
  src\syntax_core.c src\syntax_c.c src\syntax_py.c src\syntax_js.c ^
  src\find.c src\run_exec_win32.c ^
  /link /out:dist\wofl-ide.exe user32.lib gdi32.lib comdlg32.lib shell32.lib ole32.lib

Then add more files as you create them.
*/