@echo off
setlocal enabledelayedexpansion

echo ========================================
echo Building WOFL IDE (Debug Mode)
echo ========================================

set CC=cl.exe
set CFLAGS=/nologo /Zi /Od /DDEBUG /WX /W4 /Iinclude
set LDFLAGS=/nologo /SUBSYSTEM:WINDOWS /DEBUG
set LIBS=user32.lib gdi32.lib comdlg32.lib shell32.lib

if not exist build_debug mkdir build_debug
pushd build_debug

echo Compiling (Debug)...
%CC% %CFLAGS% ..\src\*.c /c
if errorlevel 1 (
    echo.
    echo ========================================
    echo DEBUG BUILD FAILED
    echo ========================================
    popd
    exit /b 1
)

echo Linking...
link %LDFLAGS% *.obj %LIBS% /OUT:..\dist\wofl-ide-debug.exe /PDB:..\dist\wofl-ide-debug.pdb
if errorlevel 1 (
    echo.
    echo ========================================
    echo DEBUG BUILD FAILED (link stage)
    echo ========================================
    popd
    exit /b 1
)

popd
echo.
echo ========================================
echo DEBUG BUILD SUCCESSFUL
echo ========================================
echo Output: dist\wofl-ide-debug.exe
echo PDB:    dist\wofl-ide-debug.pdb
echo.
echo Debug with WinDbg:  windbgx.exe -o dist\wofl-ide-debug.exe
echo ========================================
