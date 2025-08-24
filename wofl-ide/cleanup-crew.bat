@echo off
setlocal
if exist build_debug rmdir /s /q build_debug
if exist build_release rmdir /s /q build_release
if exist dist\wofl-ide-debug.exe del /f /q dist\wofl-ide-debug.exe
if exist dist\wofl-ide-debug.pdb del /f /q dist\wofl-ide-debug.pdb
if exist dist\wofl-ide.exe del /f /q dist\wofl-ide.exe
if exist dist\wofl-ide.pdb del /f /q dist\wofl-ide.pdb
echo Cleaned.
