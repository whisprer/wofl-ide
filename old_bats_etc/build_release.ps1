# ---- WOFL IDE: RELEASE BUILD (PowerShell) ----
$ErrorActionPreference = "Stop"
$SRC = "src"
$OBJ = "build_release"
$OUT = "dist"
$EXE = Join-Path $OUT "wofl-ide.exe"
$PDB = Join-Path $OUT "wofl-ide.pdb"

New-Item -ItemType Directory -Force -Path $OBJ | Out-Null
New-Item -ItemType Directory -Force -Path $OUT | Out-Null

Write-Host "========================================"
Write-Host "Building WOFL IDE (Release Mode)"
Write-Host "========================================"

if (-not (Get-Command cl -ErrorAction SilentlyContinue)) {
  Write-Host "[ERROR] cl.exe not found. Use 'x64 Native Tools Command Prompt for VS' or call from a VS Dev shell."
  exit 1
}

Write-Host "Compiling (Release)..."
& cl /nologo /O2 /GL /utf-8 /W4 /I include /DNDEBUG /Fo "$OBJ\" /c "$SRC\*.c"
if ($LASTEXITCODE -ne 0) {
  Write-Host ""
  Write-Host "========================================"
  Write-Host "RELEASE BUILD FAILED (compile stage)"
  Write-Host "========================================"
  exit $LASTEXITCODE
}

Write-Host "Linking..."
& link /nologo /SUBSYSTEM:WINDOWS /LTCG "$OBJ\*.obj" user32.lib gdi32.lib comdlg32.lib shell32.lib `
  /OUT:"$EXE" /PDB:"$PDB"
if ($LASTEXITCODE -ne 0) {
  Write-Host ""
  Write-Host "========================================"
  Write-Host "RELEASE BUILD FAILED (link stage)"
  Write-Host "========================================"
  exit $LASTEXITCODE
}

if (-not (Test-Path $EXE)) {
  Write-Host ""
  Write-Host "========================================"
  Write-Host "RELEASE BUILD FAILED (no output produced)"
  Write-Host "========================================"
  exit 1
}

Write-Host ""
Write-Host "========================================"
Write-Host "RELEASE BUILD SUCCESSFUL"
Write-Host "========================================"
Write-Host "Output: $EXE"
Write-Host "PDB:    $PDB"
Write-Host "========================================"
exit 0
