param(
  [int[]]$NList = @(2000,4000,8000),
  [int[]]$ThreadsList = @(1,2,4,6,12),
  [string[]]$Schedules = @("static","dynamic_64","guided_64"),
  [switch]$VerboseScan
)

$root   = (Get-Location).Path
$outDir = Join-Path $root "data\results"
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

$summaryPath = Join-Path $outDir "summary_schedules.csv"
$lines = @("N,Threads,Schedule,Tb_ms,To_ms,Speedup,Efficiency")

Write-Host "Params:"
Write-Host ("  NList       = {0}" -f ($NList -join ","))
Write-Host ("  ThreadsList = {0}" -f ($ThreadsList -join ","))
Write-Host ("  Schedules   = {0}" -f ($Schedules -join ","))

function Get-AvgMs {
  param([System.IO.FileInfo[]]$Files)
  if (-not $Files -or $Files.Count -eq 0) { return $null }
  $vals = @()
  foreach ($f in $Files) {
    try {
      $rows = Import-Csv -Path $f.FullName
      foreach ($r in $rows) {
        if ($r.ms -ne $null -and $r.ms -ne "") {
          $vals += [double]::Parse($r.ms, [System.Globalization.CultureInfo]::InvariantCulture)
        }
      }
    } catch {
      Write-Warning "No pude leer $($f.Name): $_"
    }
  }
  if ($vals.Count -eq 0) { return $null }
  return ($vals | Measure-Object -Average).Average
}

function FindFilesPattern {
  param([string]$Pattern)
  $all = Get-ChildItem -Path $outDir -File
  $matched = $all | Where-Object { $_.Name -like $Pattern }
  if ($VerboseScan) { Write-Host ("MATCH {0} -> {1}" -f $Pattern, ($matched | Measure-Object).Count) }
  return $matched
}

foreach ($N in $NList) {
  $seqPat = ("seq_N{0}_r*.csv" -f $N)
  $seqFiles = FindFilesPattern $seqPat
  $Tb = Get-AvgMs $seqFiles
  if ($null -eq $Tb) {
    Write-Warning "No hay CSVs secuenciales para N=$N (pat=$seqPat). Saltando N."
    continue
  }

  foreach ($T in $ThreadsList) {
    foreach ($S in $Schedules) {
      $pat = ("omp_for_N{0}_T{1}_S{2}_r*.csv" -f $N,$T,$S)
      $ompFiles = FindFilesPattern $pat
      if (($ompFiles | Measure-Object).Count -eq 0) {
        Write-Warning "Faltan CSVs para N=$N T=$T S=$S (pat=$pat)"
        continue
      }
      $To = Get-AvgMs $ompFiles
      if ($null -eq $To) {
        Write-Warning "CSV sin datos válidos para N=$N T=$T S=$S (pat=$pat)"
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
