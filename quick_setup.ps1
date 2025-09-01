
# Ejecutar:
#   powershell -ExecutionPolicy Bypass -File .\quick_setup.ps1
#   powershell -ExecutionPolicy Bypass -File .\quick_setup.ps1 -RestoreRenderer

param(
  [switch]$RestoreRenderer
)

try {
  [Console]::InputEncoding  = [System.Text.Encoding]::UTF8
  [Console]::OutputEncoding = [System.Text.Encoding]::UTF8
} catch {}

function Test-Command { param($Command) try { Get-Command $Command -ErrorAction Stop | Out-Null; $true } catch { $false } }

$projectDir = Get-Location

Write-Host ''
Write-Host '[SETUP] Verificando herramientas...'
$tools = @{
  'git'            = 'https://git-scm.com/download/win'
  'cmake'          = 'https://cmake.org/download/'
  'g++'            = 'https://github.com/niXman/mingw-builds-binaries/releases'
  'mingw32-make'   = 'https://github.com/niXman/mingw-builds-binaries/releases'
}
$allOk = $true
foreach ($t in $tools.Keys) {
  if (Test-Command $t) { Write-Host "  OK: $t" } else { Write-Host "  FALTA: $t -> $($tools[$t])"; $allOk = $false }
}
if (-not $allOk) { Read-Host "`nInstala lo faltante y presiona Enter para salir"; exit 1 }

# Detectar MinGW a partir de g++
$gppCmd = Get-Command g++ -ErrorAction SilentlyContinue
if (-not $gppCmd) { Write-Host 'No se encontro g++ en PATH'; exit 1 }
$mingwBin = Split-Path $gppCmd.Source
$gccExe   = Join-Path $mingwBin 'gcc.exe'
$gppExe   = Join-Path $mingwBin 'g++.exe'
$makeExe  = Join-Path $mingwBin 'mingw32-make.exe'
if (-not (Test-Path $makeExe)) {
  if (-not (Test-Command 'mingw32-make')) {
    Write-Host "No se encontro mingw32-make.exe en $mingwBin ni en PATH." -ForegroundColor Red
    exit 1
  } else { $makeExe = (Get-Command mingw32-make).Source }
}
if ($env:Path -notlike "*$mingwBin*") { $env:Path = "$mingwBin;$env:Path" }

# Triplet MinGW (evita Visual Studio)
$VcpkgTriplet = 'x64-mingw-dynamic'
$env:VCPKG_DEFAULT_TRIPLET = $VcpkgTriplet
$env:VCPKG_DEFAULT_HOST_TRIPLET = $VcpkgTriplet

# vcpkg
Write-Host "`n[SETUP] vcpkg..."
if (-not (Test-Path 'C:\vcpkg\vcpkg.exe')) {
  Write-Host '  Instalando vcpkg en C:\vcpkg'
  Push-Location C:\
  git clone https://github.com/microsoft/vcpkg.git
  if (-not (Test-Path 'C:\vcpkg')) { Write-Host '  Error clonando vcpkg'; exit 1 }
  Set-Location C:\vcpkg
  .\bootstrap-vcpkg.bat
  .\vcpkg integrate install
  Pop-Location
} else { Write-Host '  vcpkg ya esta instalado' }

# Activar ULTRA (automatico si existe el archivo)
$rendererDir   = Join-Path $projectDir 'src\gfx'
$baseRenderer  = Join-Path $rendererDir 'renderer_sdl2.cpp'
$backupRenderer= Join-Path $rendererDir 'renderer_sdl2_backup.cpp'
$ultraRenderer = Join-Path $rendererDir 'renderer_sdl2_ultra.cpp'

if ($RestoreRenderer) {
  Write-Host "`n[RENDERER] Restaurando renderer original..."
  if (Test-Path $backupRenderer) {
    Copy-Item $backupRenderer $baseRenderer -Force
    Write-Host '  OK: Restaurado desde renderer_sdl2_backup.cpp'
  } else { Write-Host '  Aviso: No existe backup para restaurar' }
} elseif (Test-Path $ultraRenderer) {
  Write-Host "`n[RENDERER] Activando renderer ULTRA..."
  if (-not (Test-Path $backupRenderer) -and (Test-Path $baseRenderer)) {
    Copy-Item $baseRenderer $backupRenderer -Force
    Write-Host '  OK: Backup creado (renderer_sdl2_backup.cpp)'
  }
  Copy-Item $ultraRenderer $baseRenderer -Force
  Write-Host '  OK: ULTRA instalado como renderer_sdl2.cpp'
} else {
  Write-Host "`n[RENDERER] Archivo ULTRA no encontrado, se usara el renderer actual"
}

# SDL2 via vcpkg (manifest o classic)
Write-Host "`n[SETUP] SDL2..."
$manifestPath = Join-Path $projectDir 'vcpkg.json'
$code = 0
if ((Test-Path $manifestPath) -and ((Get-Content $manifestPath -Raw).Trim().Length -gt 0)) {
  Write-Host '  Usando manifest (vcpkg.json) ...'
  Push-Location $projectDir
  & C:\vcpkg\vcpkg.exe install --triplet $VcpkgTriplet --host-triplet $VcpkgTriplet
  $code = $LASTEXITCODE
  Pop-Location
} else {
  $sdlInstalled = & C:\vcpkg\vcpkg.exe list 2>$null | Select-String "sdl2.*$VcpkgTriplet"
  if (-not $sdlInstalled) {
    Write-Host "  Instalando sdl2:$VcpkgTriplet (classic mode)..."
    Push-Location C:\vcpkg
    & .\vcpkg.exe install "sdl2:$VcpkgTriplet" --host-triplet $VcpkgTriplet --feature-flags=-manifests
    $code = $LASTEXITCODE
    Pop-Location
  } else { Write-Host '  SDL2 ya instalado (classic mode)' }
}
if ($code -ne 0) { Write-Host '  Error instalando SDL2'; exit 1 }

# Build
Write-Host "`n[BUILD] Preparando build..."
Set-Location $projectDir
$buildDir = 'build-visual'
if (Test-Path $buildDir) { Remove-Item -Recurse -Force $buildDir }
New-Item -ItemType Directory -Path $buildDir | Out-Null
Set-Location $buildDir

Write-Host '  Configurando CMake...'
$cmakeArgs = @(
  '..',
  '-G', 'MinGW Makefiles',
  '-DCMAKE_BUILD_TYPE=Release',
  '-DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake',
  "-DVCPKG_TARGET_TRIPLET=$VcpkgTriplet",
  "-DVCPKG_HOST_TRIPLET=$VcpkgTriplet",
  "-DCMAKE_C_COMPILER=$gccExe",
  "-DCMAKE_CXX_COMPILER=$gppExe",
  "-DCMAKE_MAKE_PROGRAM=$makeExe",
  '-DENABLE_OPENMP=ON',
  '-DENABLE_SDL2=ON'
)
cmake @cmakeArgs
if ($LASTEXITCODE -ne 0) { Write-Host '  Error en CMake'; exit 1 }

Write-Host '  Compilando...'
cmake --build . --config Release -j 8
if ($LASTEXITCODE -ne 0) { Write-Host '  Error en compilacion'; exit 1 }

# Copiar SDL2.dll
Write-Host "`n[POST] Copiando SDL2.dll..."
$manifestBin = Join-Path $projectDir "vcpkg_installed\$VcpkgTriplet\bin\SDL2.dll"
$classBin    = "C:\vcpkg\installed\$VcpkgTriplet\bin\SDL2.dll"
if (Test-Path $manifestBin) { Copy-Item $manifestBin -Destination . -Force; Write-Host '  SDL2.dll copiado (manifest)' }
elseif (Test-Path $classBin) { Copy-Item $classBin -Destination . -Force; Write-Host '  SDL2.dll copiado (classic)' }
else { Write-Host '  Aviso: no se encontro SDL2.dll en rutas conocidas' }

# Verificar ejecutables
Write-Host "`n[CHECK] Ejecutables generados..."
$exes = @('seq_sdl2.exe', 'omp_for_sdl2.exe', 'omp_simd_sdl2.exe', 'omp_tasks_sdl2.exe')
$foundExe = $null
foreach ($e in $exes) { if (Test-Path $e) { Write-Host "  OK: $e"; if (-not $foundExe) { $foundExe = $e } } }
if (-not $foundExe) { Write-Host '  No se encontraron ejecutables'; exit 1 }

# Menu simple de ejecucion
Write-Host "`n[RUN] Opciones:"
Write-Host '  1) Demo rapida (200 particulas, 30 s)'
Write-Host '  2) Demo normal (400 particulas, 60 s)'
Write-Host '  3) Demo completa (800 particulas, 120 s)'
Write-Host '  4) Personalizado'
Write-Host '  Q) Salir'
$choice = Read-Host 'Elige una opcion'
$exe = 'omp_for_sdl2.exe'; $params = ''
switch ($choice) {
  '1' { $params = '--n 200 --steps 1800 --threads 4' }
  '2' { $params = '--n 400 --steps 3600 --threads 4' }
  '3' { $params = '--n 800 --steps 7200 --threads 6' }
  '4' {
    $n = Read-Host 'Numero de particulas (ej: 500)'
    $s = Read-Host 'Duracion en segundos (ej: 60)'
    $t = Read-Host 'Numero de threads (ej: 4)'
    $steps = [int]$s * 60
    $params = "--n $n --steps $steps --threads $t"
  }
  'Q' { exit 0 }
  'q' { exit 0 }
  default { $params = '--n 300 --steps 1800 --threads 4' }
}
Write-Host ''
Write-Host "Ejecutando: $exe $params"
Start-Process -FilePath ".\$exe" -ArgumentList $params -Wait

Set-Location $projectDir
Read-Host "`nListo. Presiona Enter para salir"
