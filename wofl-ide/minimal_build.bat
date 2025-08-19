// ==================== Minimal build.bat ====================
@echo off
echo Building WOFL IDE (Minimal)...

cl /nologo /W3 /O2 /MT /DUNICODE /D_UNICODE ^
  src\main_win32.c src\editor_buffer.c src\editor_render.c ^
  src\syntax_core.c src\syntax_c.c src\syntax_py.c src\syntax_js.c ^
  src\find.c src\run_exec_win32.c src\cmd_palette.c src\config.c ^
  /link /out:dist\wofl-ide.exe user32.lib gdi32.lib comdlg32.lib shell32.lib ole32.lib

if %errorlevel% neq 0 (
    echo BUILD FAILED
    exit /b 1
)

echo BUILD SUCCESSFUL
echo Output: dist\wofl-ide.exe