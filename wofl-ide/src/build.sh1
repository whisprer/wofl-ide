@echo off
echo ========================================
echo Building WOFL Ultra-Minimal IDE
echo ========================================
echo Checking for source files...

if not exist src\syntax_asm.c echo WARNING: src\syntax_asm.c not found - skipping

echo Compiling...
cl /W4 /Fe:wofl.exe src\*.c /link user32.lib gdi32.lib
if %ERRORLEVEL% NEQ 0 (
    echo ========================================
    echo BUILD FAILED
    echo ========================================
    echo Please check:
    echo 1. All source files exist in src\ directory
    echo 2. editor.h has matching #ifndef/#endif
    echo 3. File names match (cmd_palette.c vs command_palette.c)
    echo ========================================
    exit /b 1
)
echo ========================================
echo BUILD SUCCEEDED
echo ========================================