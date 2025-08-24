# ---- WOFL IDE: CLEAN (PowerShell) ----
$ErrorActionPreference = "SilentlyContinue"
Remove-Item -Recurse -Force build_debug
Remove-Item -Recurse -Force build_release
Remove-Item -Force dist\wofl-ide-debug.exe
Remove-Item -Force dist\wofl-ide-debug.pdb
Remove-Item -Force dist\wofl-ide.exe
Remove-Item -Force dist\wofl-ide.pdb
Write-Host "Cleaned."
