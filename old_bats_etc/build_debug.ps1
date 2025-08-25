# ---- WOFL IDE: DEBUG BUILD (PowerShell) ----
$ErrorActionPreference = "Stop"
$SRC = "src"
$OBJ = "build_debug"
$OUT = "dist"
$EXE = Join-Path $OUT "wofl-ide-debug.exe"
$PDB = Join-Path $OUT "wofl-ide-debug.pdb"

New-Item -ItemType Directory -Force -Path $OBJ | Out-Null
New-Item -ItemType Directory -Force -Path $OUT | Out-Null

Write-Host "========================================"
Write-Host "Building WOFL IDE (Debug Mode)"
Write-Host "========================================"

if (-not (Get-Command cl -ErrorAction SilentlyContinue)) {
  Write-Host "[ERROR] cl.exe not found. Use 'x64 Native Tools Command Prompt for VS' or call from a VS Dev shell."
  exit 1
}

Write-Host "Compiling (Debug)..."
& cl /nologo /Zi /Od /utf-8 /W4 /WX /DDEBUG /I include /Fo "$OBJ\" /c "$SRC\*.c"
if ($LASTEXITCODE -ne 0) {
  Write-Host ""
  Write-Host "========================================"
  Write-Host "DEBUG BUILD FAILED (compile stage)"
  Write-Host "========================================"
  exit $LASTEXITCODE
}

Write-Host "Linking..."
& link /nologo /SUBSYSTEM:WINDOWS /DEBUG "$OBJ\*.obj" user32.lib gdi32.lib comdlg32.lib shell32.lib `
  /OUT:"$EXE" /PDB:"$PDB"
if ($LASTEXITCODE -ne 0) {
  Write-Host ""
  Write-Host "========================================"
  Write-Host "DEBUG BUILD FAILED (link stage)"
  Write-Host "========================================"
  exit $LASTEXITCODE
}

if (-not (Test-Path $EXE)) {
  Write-Host ""
  Write-Host "========================================"
  Write-Host "DEBUG BUILD FAILED (no output produced)"
  Write-Host "========================================"
  exit 1
}

Write-Host ""
Write-Host "========================================"
Write-Host "DEBUG BUILD SUCCESSFUL"
Write-Host "========================================"
Write-Host "Output: $EXE"
Write-Host "PDB:    $PDB"
Write-Host ""
Write-Host "Run:    `"$EXE`""
Write-Host "Debug:  windbgx.exe -o `"$EXE`""
Write-Host "========================================"
exit 0
