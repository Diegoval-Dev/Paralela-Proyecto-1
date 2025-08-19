param(
  [int[]]$NList = @(2000,4000,8000),
  [int[]]$ThreadsList = @(1,2,4,6,12),
  [string[]]$Schedules = @("static","dynamic_64","guided_64")  # usar nombres con ':' -> '_' (como los generó el runner)
)

function Get-AvgMsFromCsvs {
  param([System.IO.FileInfo[]]$Files)
  if (-not $Files -or $Files.Count -eq 0) { return $null }
  $means = @()
  foreach ($f in $Files) {
    # Cada CSV tiene columnas: frame,ms
    try {
      $rows = Import-Csv -Path $f.FullName
      if ($rows.Count -eq 0) { continue }
      $avg = ($rows | Measure-Object -Property ms -Average).Average
      if ($null -ne $avg) { $means += [double]$avg }
    } catch {
      Write-Warning "No pude leer $($f.FullName): $_"
    }
  }
  if ($means.Count -eq 0) { return $null }
  return ($means | Measure-Object -Average).Average
}

$root = (Get-Location).Path
$outDir = Join-Path $root "data\results"
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

$summaryPath = Join-Path $outDir "summary_task.csv"
$lines = @()
$lines += "N,Threads,Schedule,Tb_ms,To_ms,Speedup,Efficiency"

foreach ($N in $NList) {
  # Baseline secuencial Tb para este N (promedio de promedios)
  $seqFiles = Get-ChildItem -Path $outDir -Filter ("seq_N{0}_r*.csv" -f $N) -File | Sort-Object Name
  $Tb = Get-AvgMsFromCsvs $seqFiles
  if ($null -eq $Tb) {
    Write-Warning "No hay CSVs secuenciales para N=$N. Saltando."
    continue
  }

  foreach ($T in $ThreadsList) {
    foreach ($S in $Schedules) {
      $pattern = ("omp_tasks_N{0}_T{1}_S{2}_r*.csv" -f $N,$T,$S)
      $ompFiles = Get-ChildItem -Path $outDir -Filter $pattern -File | Sort-Object Name
      $To = Get-AvgMsFromCsvs $ompFiles
      if ($null -eq $To) {
        Write-Warning "Faltan CSVs para N=$N T=$T S=$S. (pattern=$pattern)"
        continue
      }
      $SPEED = $Tb / $To
      $EFF   = $SPEED / $T
      $lines += ("{0},{1},{2},{3:N6},{4:N6},{5:N3},{6:N3}" -f $N,$T,$S,$Tb,$To,$SPEED,$EFF)
    }
  }
}

$lines | Set-Content -Path $summaryPath -Encoding UTF8
Write-Host "Resumen creado: $summaryPath"
