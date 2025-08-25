@echo off
REM ---- WOFL IDE: DEBUG BUILD (MSVC) ----

set SRC=src
set OBJ=build_debug
set OUT=dist
set EXE=%OUT%\wofl-ide-debug.exe
set PDB=%OUT%\wofl-ide-debug.pdb

if not exist %OBJ% mkdir %OBJ%
if not exist %OUT% mkdir %OUT%

echo ========================================
echo Building WOFL IDE (Debug Mode)
echo ========================================

where cl >nul 2>&1 || (
  echo [ERROR] cl.exe not found. Use "x64 Native Tools Command Prompt for VS".
  exit /b 1
)

echo Compiling (Debug)...
pushd %OBJ%
cl /nologo /Zi /Od /utf-8 /W4 /WX /DDEBUG /I ..\include /c ..\%SRC%\*.c
popd

echo Linking...
link /nologo /SUBSYSTEM:WINDOWS /DEBUG /MACHINE:X86 %OBJ%\*.obj user32.lib gdi32.lib comdlg32.lib shell32.lib /OUT:%EXE% /PDB:%PDB%

if not %ERRORLEVEL%==0 (
  echo ========================================
  echo DEBUG BUILD FAILED (link stage)
  echo ========================================
  exit /b %ERRORLEVEL%
)

if not exist %EXE% (
  echo ========================================
  echo DEBUG BUILD FAILED (no output produced)
  echo ========================================
  exit /b 1
)

echo ========================================
echo DEBUG BUILD SUCCESSFUL
echo ========================================
echo Output: %EXE%
echo PDB:    %PDB%
echo Run:    "%EXE%"
echo Debug:  windbgx.exe -o "%EXE%"
echo ========================================