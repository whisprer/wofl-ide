// ==================== build_debug.bat ====================
// Debug build script with symbols and no optimization

@echo off
setlocal enabledelayedexpansion

echo ========================================
echo Building WOFL IDE (Debug Mode)
echo ========================================
echo.

rem Check for Visual Studio environment
where cl >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: MSVC compiler not found!
    echo Please run this script from "x64 Native Tools Command Prompt for VS"
    exit /b 1
)

rem Configuration for debug build
set OUTPUT_DIR=dist
set EXE_NAME=wofl-ide-debug.exe
set OPTIMIZATION=/Od
set WARNINGS=/W4
set DEBUG_FLAGS=/Zi /DEBUG

rem Create output directory
if not exist %OUTPUT_DIR% (
    echo Creating output directory...
    mkdir %OUTPUT_DIR%
)

rem Source files
set CORE_SOURCES=src\main_win32.c src\editor_buffer.c src\editor_render.c
set SYNTAX_SOURCES=src\syntax_core.c src\syntax_c.c src\syntax_py.c src\syntax_js.c
set SYNTAX_SOURCES=%SYNTAX_SOURCES% src\syntax_html.c src\syntax_css.c src\syntax_json.c
set SYNTAX_SOURCES=%SYNTAX_SOURCES% src\syntax_md.c src\syntax_go.c src\syntax_rs.c
set SYNTAX_SOURCES=%SYNTAX_SOURCES% src\syntax_sh.c src\syntax_lua.c
set FEATURE_SOURCES=src\find.c src\cmd_palette.c src\config.c src\run_exec_win32.c
set PLUGIN_SOURCES=src\theme_system.c src\plugin_system.c src\syntax_cpp.c src\syntax_asm.c src\syntax_csv.c src\plugin_git.c

set ALL_SOURCES=%CORE_SOURCES% %SYNTAX_SOURCES% %FEATURE_SOURCES%

rem Compiler flags for debug
set CFLAGS=/nologo %WARNINGS% %OPTIMIZATION% /MTd %DEBUG_FLAGS%
set DEFINES=/DUNICODE /D_UNICODE /DWIN32_LEAN_AND_MEAN /D_DEBUG
set LIBS=user32.lib gdi32.lib comdlg32.lib shell32.lib ole32.lib

echo Compiling (Debug)...
echo Optimization: Disabled
echo Debug symbols: Enabled
echo.

rem Compile and link with debug info
cl %CFLAGS% %DEFINES% %ALL_SOURCES% /link /out:%OUTPUT_DIR%\%EXE_NAME% %LIBS% /DEBUG

if %errorlevel% neq 0 (
    echo.
    echo ========================================
    echo DEBUG BUILD FAILED
    echo ========================================
    exit /b 1
)

echo.
echo ========================================
echo DEBUG BUILD SUCCESSFUL
echo ========================================
echo Output: %OUTPUT_DIR%\%EXE_NAME%
echo PDB: %OUTPUT_DIR%\wofl-ide-debug.pdb
echo.
echo Debug with Visual Studio or WinDbg
echo ========================================

rem Clean up .obj files but keep .pdb
del *.obj 2>nul

endlocal
