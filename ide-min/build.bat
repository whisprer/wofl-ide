@echo off
setlocal
if not exist dist mkdir dist

rem Use from a "x64 Native Tools Command Prompt for VS" (MSVC)
cl /nologo /W4 /O2 /MT /DUNICODE /D_UNICODE ^
  src\main_win32.c src\editor_buffer.c src\editor_render.c src\syntax_core.c ^
  src\syntax_c.c src\syntax_py.c src\syntax_js.c src\syntax_html.c src\syntax_css.c ^
  src\syntax_json.c src\syntax_md.c src\syntax_go.c src\syntax_rs.c src\syntax_sh.c src\syntax_lua.c ^
  src\run_exec_win32.c src\find.c src\cmd_palette.c src\config.c ^
  /link /out:dist\wofl-ide.exe user32.lib gdi32.lib comdlg32.lib shell32.lib ole32.lib

if errorlevel 1 (
  echo Build failed.
  exit /b 1
) else (
  echo Built dist\wofl-ide.exe
)
