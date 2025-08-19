param(
  [int[]]$NList = @(2000,4000,8000),
  [int[]]$ThreadsList = @(1,2,4,6,12),
  [string[]]$Schedules = @("static","dynamic:64","guided:64"),
  [int]$Steps = 1000,
  [int]$Reps = 10
)

# Validaciones rápidas
if (-not $NList -or -not $ThreadsList -or -not $Schedules) {
  Write-Error "Parámetros vacíos: NList/ThreadsList/Schedules."
  exit 1
}

$root = (Get-Location).Path
$bin  = Join-Path $root "build\mingw-release"
$out  = Join-Path $root "data\results"
New-Item -ItemType Directory -Force -Path $out | Out-Null

# Quita variables de afinidad que rompen con libgomp en MinGW
Remove-Item Env:OMP_PROC_BIND -ErrorAction SilentlyContinue
Remove-Item Env:OMP_PLACES    -ErrorAction SilentlyContinue

$stamp = Get-Date -Format "yyyyMMdd_HHmmss"
$log = Join-Path $out "bench_sched_$stamp.log"

Write-Output "Starting bench/schedules at $stamp" | Tee-Object -FilePath $log -Append
Write-Output "BIN=$bin" | Tee-Object -FilePath $log -Append

foreach ($N in $NList) {
  # Baseline secuencial
  foreach ($r in 1..$Reps) {
    $csv = Join-Path $out ("seq_N{0}_r{1}.csv" -f $N, $r)
    Write-Output ("RUN  N={0}  MODE=seq  r={1} -> {2}" -f $N,$r,$csv) | Tee-Object -FilePath $log -Append
    & "$bin\seq.exe" --n $N --steps $Steps --record $csv
    Write-Output ("EXIT CODE: {0}" -f $LASTEXITCODE) | Tee-Object -FilePath $log -Append
  }

  foreach ($T in $ThreadsList) {
    foreach ($S in $Schedules) {
      $tag = ($S -replace "[:]", "_")
      foreach ($r in 1..$Reps) {
        $csv = Join-Path $out ("omp_for_N{0}_T{1}_S{2}_r{3}.csv" -f $N, $T, $tag, $r)
        $cmd = "`"$bin\omp_for.exe`" --n $N --steps $Steps --threads $T --schedule $S --record `"$csv`""
        Write-Output ("RUN  N={0}  T={1}  S={2}  r={3} -> {4}" -f $N,$T,$S,$r,$csv) | Tee-Object -FilePath $log -Append
        Write-Output ("CMD  {0}" -f $cmd) | Tee-Object -FilePath $log -Append
        try {
          & "$bin\omp_for.exe" --n $N --steps $Steps --threads $T --schedule $S --record $csv
          Write-Output ("EXIT CODE: {0}" -f $LASTEXITCODE) | Tee-Object -FilePath $log -Append
        } catch {
          Write-Warning "Fallo en N=$N T=$T S=$S r=$r : $_"
        }
      }
    }
  }
}

Write-Output "Bench done." | Tee-Object -FilePath $log -Append
