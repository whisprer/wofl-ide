@echo off
setlocal ENABLEDELAYEDEXPANSION
REM ---- WOFL IDE: DEBUG BUILD (MSVC) ----

set "SRC=src"
set "OBJ=build_debug"
set "OUT=dist"
set "EXE=%OUT%\wofl-ide-debug.exe"
set "PDB=%OUT%\wofl-ide-debug.pdb"

if not exist "%OBJ%" mkdir "%OBJ%"
if not exist "%OUT%" mkdir "%OUT%"

echo ========================================
echo Building WOFL IDE (Debug Mode)
echo ========================================

where cl >nul 2>&1 || (
  echo [ERROR] cl.exe not found. Use "x64 Native Tools Command Prompt for VS".
  exit /b 1
)

echo Compiling (Debug)...
pushd "%OBJ%"
cl /nologo /Zi /Od /utf-8 /W4 /WX /DDEBUG /I "..\include" /c "..\%SRC%\*.c"
set "COMPILE_RC=%ERRORLEVEL%"
popd

echo Debug: Compile RC = %COMPILE_RC%
echo Debug: About to check compile result...

if "%COMPILE_RC%"=="0" (
  echo Debug: Compile check passed!
  echo Compile successful, proceeding to linking...
) else (
  echo Debug: Compile failed with code %COMPILE_RC%, exiting...
  echo.
  echo ========================================
  echo DEBUG BUILD FAILED (compile stage)
  echo ========================================
  exit /b %COMPILE_RC%
)

echo Linking...
echo Debug: About to run link command...
link /nologo /SUBSYSTEM:WINDOWS /DEBUG /MACHINE:X64 "%OBJ%\*.obj" user32.lib gdi32.lib comdlg32.lib shell32.lib ^
  /OUT:"%EXE%" /PDB:"%PDB%"
set "RC=%ERRORLEVEL%"

if not "%RC%"=="0" (
  echo.
  echo ========================================
  echo DEBUG BUILD FAILED (link stage)
  echo ========================================
  exit /b %RC%
)

if not exist "%EXE%" (
  echo.
  echo ========================================
  echo DEBUG BUILD FAILED (no output produced)
  echo ========================================
  exit /b 1
)

echo.
echo ========================================
echo DEBUG BUILD SUCCESSFUL
echo ========================================
echo Output: %EXE%
echo PDB:    %PDB%
echo Run:    "%EXE%"
echo Debug:  windbgx.exe -o "%EXE%"
echo ========================================
exit /b 0