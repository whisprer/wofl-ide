@echo off
setlocal ENABLEDELAYEDEXPANSION
REM ---- WOFL IDE: RELEASE BUILD (MSVC) ----

set "SRC=src"
set "OBJ=build_release"
set "OUT=dist"
set "EXE=%OUT%\wofl-ide.exe"
set "PDB=%OUT%\wofl-ide.pdb"

if not exist "%OBJ%" mkdir "%OBJ%"
if not exist "%OUT%" mkdir "%OUT%"

echo ========================================
echo Building WOFL IDE (Release Mode)
echo ========================================

where cl >nul 2>&1 || (
  echo [ERROR] cl.exe not found. Use "x64 Native Tools Command Prompt for VS".
  exit /b 1
)

echo Compiling (Release)...
cl /nologo /O2 /GL /utf-8 /W4 /I include /DNDEBUG /Fo "%OBJ%\" /c "%SRC%\*.c"
if errorlevel 1 (
  echo.
  echo ========================================
  echo RELEASE BUILD FAILED (compile stage)
  echo ========================================
  exit /b 1
)

echo Linking...
link /nologo /SUBSYSTEM:WINDOWS /LTCG "%OBJ%\*.obj" user32.lib gdi32.lib comdlg32.lib shell32.lib ^
  /OUT:"%EXE%" /PDB:"%PDB%"
set "RC=%ERRORLEVEL%"

if not "%RC%"=="0" (
  echo.
  echo ========================================
  echo RELEASE BUILD FAILED (link stage)
  echo ========================================
  exit /b %RC%
)

if not exist "%EXE%" (
  echo.
  echo ========================================
  echo RELEASE BUILD FAILED (no output produced)
  echo ========================================
  exit /b 1
)

echo.
echo ========================================
echo RELEASE BUILD SUCCESSFUL
echo ========================================
echo Output: %EXE%
echo PDB:    %PDB%
echo ========================================
exit /b 0
