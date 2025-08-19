param(
  [int[]]$NList = @(2000,4000,8000),
  [int[]]$ThreadsList = @(1,2,4,6,12),
  [string[]]$Schedules = @("static","dynamic:64","guided:64"),
  [int]$Steps = 1000,
  [int]$Reps = 10
)

$root = (Get-Location).Path
$bin  = Join-Path $root "build\mingw-release"
$out  = Join-Path $root "data\results"
New-Item -ItemType Directory -Force -Path $out | Out-Null

$stamp = Get-Date -Format "yyyyMMdd_HHmmss"
$log = Join-Path $out "bench_simd_$stamp.log"

# Nota: en tu MinGW/libgomp puede no soportar affinity; si te molesta el warning, comenta estas dos líneas:
$env:OMP_PROC_BIND="close"
$env:OMP_PLACES="cores"

Write-Output "Starting bench/SIMD at $stamp" | Tee-Object -FilePath $log -Append

foreach ($N in $NList) {
  # baseline secuencial (10 repeticiones)
  foreach ($r in 1..$Reps) {
    $csv = Join-Path $out ("seq_N{0}_r{1}.csv" -f $N, $r)
    & "$bin\seq.exe" --n $N --steps $Steps --record $csv | Tee-Object -FilePath $log -Append
  }

  foreach ($T in $ThreadsList) {
    foreach ($S in $Schedules) {
      foreach ($r in 1..$Reps) {
        $csv = Join-Path $out ("omp_simd_N{0}_T{1}_S{2}_r{3}.csv" -f $N, $T, ($S -replace "[:]", "_"), $r)
        & "$bin\omp_simd.exe" --n $N --steps $Steps --threads $T --schedule $S --record $csv | Tee-Object -FilePath $log -Append
      }
    }
  }
}

Write-Output "Bench SIMD done." | Tee-Object -FilePath $log -Append
