@echo off
REM ---- WOFL IDE: RELEASE BUILD (MSVC) ----

set SRC=src
set OBJ=build_release
set OUT=dist
set EXE=%OUT%\wofl-ide.exe

if not exist %OBJ% mkdir %OBJ%
if not exist %OUT% mkdir %OUT%

echo ========================================
echo Building WOFL IDE (Release Mode)
echo ========================================

where cl >nul 2>&1 || (
  echo [ERROR] cl.exe not found. Use "x64 Native Tools Command Prompt for VS".
  exit /b 1
)

echo Compiling (Release)...
pushd %OBJ%
cl /nologo /O2 /Oi /Ot /GL /utf-8 /W4 /WX /DNDEBUG /I ..\include /c ..\%SRC%\*.c
popd

echo Linking...
link /nologo /SUBSYSTEM:WINDOWS /MACHINE:X86 /OPT:REF /OPT:ICF /LTCG %OBJ%\*.obj user32.lib gdi32.lib comdlg32.lib shell32.lib /OUT:%EXE%

if not %ERRORLEVEL%==0 (
  echo ========================================
  echo RELEASE BUILD FAILED (link stage)
  echo ========================================
  exit /b %ERRORLEVEL%
)

if not exist %EXE% (
  echo ========================================
  echo RELEASE BUILD FAILED (no output produced)
  echo ========================================
  exit /b 1
)

echo File size optimization...
for %%A in (%EXE%) do set SIZE=%%~zA
set /A SIZE_KB=%SIZE%/1024

echo ========================================
echo RELEASE BUILD SUCCESSFUL
echo ========================================
echo Output: %EXE%
echo Size:   %SIZE_KB% KB
echo Run:    "%EXE%"
echo ========================================