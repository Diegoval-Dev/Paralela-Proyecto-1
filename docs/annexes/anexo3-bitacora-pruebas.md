# Anexo 3 – Bitácora de Pruebas

## Metodología
- Se realizaron ejecuciones con tamaños de problema `N = 2000, 4000, 8000`
- Se variaron los **número de hilos (1–12)** y políticas de *schedule* (`static`, `dynamic:64`, `guided:64`)
- **10 repeticiones por configuración** para obtener promedios confiables
- Se compararon las tres implementaciones principales: `omp_for`, `omp_simd`, `omp_tasks`
- Las métricas reportadas son:
  - **Speedup**: tiempo secuencial / tiempo paralelo
  - **Eficiencia**: speedup / número de hilos

---

## Mediciones Detalladas

### Resumen de Resultados – Secuencial

|    N |   Threads | Schedule |   Tb_ms |   To_ms |   Speedup |   Efficiency |
|------|-----------|----------|---------|---------|-----------|--------------|
| 2000 |         1 | seq      |   0.0166 |   0.0166 |     1.0 |        1.0 |
| 4000 |         1 | seq      |   0.0331 |   0.0331 |     1.0 |        1.0 |
| 8000 |         1 | seq      |   0.0676 |   0.0676 |     1.0 |        1.0 |


### Resumen de Resultados por Implementación

#### OpenMP For

|    N |   Threads | Schedule   |   Tb_ms |   To_ms |   Speedup |   Efficiency |
|------|-----------|------------|---------|---------|-----------|--------------|
| 2000 |         1 | static     |   0.017 |   0.042 |     0.398 |        0.398 |
| 2000 |         1 | dynamic_64 |   0.017 |   0.047 |     0.352 |        0.352 |
| 2000 |         1 | guided_64  |   0.017 |   0.044 |     0.374 |        0.374 |
| 2000 |         2 | static     |   0.017 |   0.157 |     0.106 |        0.053 |
| 2000 |         2 | dynamic_64 |   0.017 |   0.143 |     0.116 |        0.058 |
| 2000 |         2 | guided_64  |   0.017 |   0.146 |     0.114 |        0.057 |
| 2000 |         4 | static     |   0.017 |   0.184 |     0.091 |        0.023 |
| 2000 |         4 | dynamic_64 |   0.017 |   0.180 |     0.093 |        0.023 |
| 2000 |         4 | guided_64  |   0.017 |   0.179 |     0.093 |        0.023 |
| 2000 |         6 | static     |   0.017 |   0.217 |     0.077 |        0.013 |
| 2000 |         6 | dynamic_64 |   0.017 |   0.215 |     0.077 |        0.013 |
| 2000 |         6 | guided_64  |   0.017 |   0.216 |     0.077 |        0.013 |
| 2000 |        12 | static     |   0.017 |   0.306 |     0.054 |        0.005 |
| 2000 |        12 | dynamic_64 |   0.017 |   0.304 |     0.055 |        0.005 |
| 2000 |        12 | guided_64  |   0.017 |   0.309 |     0.054 |        0.004 |
| 4000 |         1 | static     |   0.033 |   0.058 |     0.573 |        0.573 |
| 4000 |         1 | dynamic_64 |   0.033 |   0.067 |     0.492 |        0.492 |
| 4000 |         1 | guided_64  |   0.033 |   0.057 |     0.580 |        0.580 |
| 4000 |         2 | static     |   0.033 |   0.170 |     0.195 |        0.097 |
| 4000 |         2 | dynamic_64 |   0.033 |   0.140 |     0.236 |        0.118 |
| 4000 |         2 | guided_64  |   0.033 |   0.148 |     0.224 |        0.112 |
| 4000 |         4 | static     |   0.033 |   0.194 |     0.170 |        0.043 |
| 4000 |         4 | dynamic_64 |   0.033 |   0.190 |     0.174 |        0.043 |
| 4000 |         4 | guided_64  |   0.033 |   0.190 |     0.174 |        0.044 |
| 4000 |         6 | static     |   0.033 |   0.227 |     0.146 |        0.024 |
| 4000 |         6 | dynamic_64 |   0.033 |   0.222 |     0.149 |        0.025 |
| 4000 |         6 | guided_64  |   0.033 |   0.225 |     0.147 |        0.025 |
| 4000 |        12 | static     |   0.033 |   0.311 |     0.106 |        0.009 |
| 4000 |        12 | dynamic_64 |   0.033 |   0.315 |     0.105 |        0.009 |
| 4000 |        12 | guided_64  |   0.033 |   0.312 |     0.106 |        0.009 |
| 8000 |         1 | static     |   0.068 |   0.091 |     0.742 |        0.742 |
| 8000 |         1 | dynamic_64 |   0.068 |   0.106 |     0.640 |        0.640 |
| 8000 |         1 | guided_64  |   0.068 |   0.087 |     0.775 |        0.775 |
| 8000 |         2 | static     |   0.068 |   0.192 |     0.352 |        0.176 |
| 8000 |         2 | dynamic_64 |   0.068 |   0.172 |     0.393 |        0.196 |
| 8000 |         2 | guided_64  |   0.068 |   0.160 |     0.424 |        0.212 |
| 8000 |         4 | static     |   0.068 |   0.219 |     0.309 |        0.077 |
| 8000 |         4 | dynamic_64 |   0.068 |   0.203 |     0.334 |        0.083 |
| 8000 |         4 | guided_64  |   0.068 |   0.211 |     0.321 |        0.080 |
| 8000 |         6 | static     |   0.068 |   0.273 |     0.247 |        0.041 |
| 8000 |         6 | dynamic_64 |   0.068 |   0.262 |     0.258 |        0.043 |
| 8000 |         6 | guided_64  |   0.068 |   0.270 |     0.250 |        0.042 |
| 8000 |        12 | static     |   0.068 |   0.342 |     0.198 |        0.016 |
| 8000 |        12 | dynamic_64 |   0.068 |   0.360 |     0.188 |        0.016 |
| 8000 |        12 | guided_64  |   0.068 |   0.364 |     0.186 |        0.015 |

#### OpenMP SIMD

|    N |   Threads | Schedule   |   Tb_ms |   To_ms |   Speedup |   Efficiency |
|------|-----------|------------|---------|---------|-----------|--------------|
| 2000 |         1 | static     |   0.017 |   0.033 |     0.499 |        0.499 |
| 2000 |         1 | dynamic_64 |   0.017 |   0.036 |     0.457 |        0.457 |
| 2000 |         1 | guided_64  |   0.017 |   0.035 |     0.480 |        0.480 |
| 2000 |         2 | static     |   0.017 |   0.102 |     0.164 |        0.082 |
| 2000 |         2 | dynamic_64 |   0.017 |   0.102 |     0.163 |        0.082 |
| 2000 |         2 | guided_64  |   0.017 |   0.109 |     0.153 |        0.076 |
| 2000 |         4 | static     |   0.017 |   0.157 |     0.106 |        0.027 |
| 2000 |         4 | dynamic_64 |   0.017 |   0.157 |     0.106 |        0.027 |
| 2000 |         4 | guided_64  |   0.017 |   0.154 |     0.108 |        0.027 |
| 2000 |         6 | static     |   0.017 |   0.217 |     0.077 |        0.013 |
| 2000 |         6 | dynamic_64 |   0.017 |   0.216 |     0.077 |        0.013 |
| 2000 |         6 | guided_64  |   0.017 |   0.217 |     0.077 |        0.013 |
| 2000 |        12 | static     |   0.017 |   0.361 |     0.046 |        0.004 |
| 2000 |        12 | dynamic_64 |   0.017 |   0.367 |     0.045 |        0.004 |
| 2000 |        12 | guided_64  |   0.017 |   0.367 |     0.045 |        0.004 |
| 4000 |         1 | static     |   0.033 |   0.048 |     0.692 |        0.692 |
| 4000 |         1 | dynamic_64 |   0.033 |   0.057 |     0.579 |        0.579 |
| 4000 |         1 | guided_64  |   0.033 |   0.050 |     0.663 |        0.663 |
| 4000 |         2 | static     |   0.033 |   0.138 |     0.239 |        0.120 |
| 4000 |         2 | dynamic_64 |   0.033 |   0.123 |     0.270 |        0.135 |
| 4000 |         2 | guided_64  |   0.033 |   0.113 |     0.293 |        0.146 |
| 4000 |         4 | static     |   0.033 |   0.169 |     0.196 |        0.049 |
| 4000 |         4 | dynamic_64 |   0.033 |   0.156 |     0.213 |        0.053 |
| 4000 |         4 | guided_64  |   0.033 |   0.174 |     0.191 |        0.048 |
| 4000 |         6 | static     |   0.033 |   0.223 |     0.149 |        0.025 |
| 4000 |         6 | dynamic_64 |   0.033 |   0.223 |     0.149 |        0.025 |
| 4000 |         6 | guided_64  |   0.033 |   0.223 |     0.148 |        0.025 |
| 4000 |        12 | static     |   0.033 |   0.391 |     0.085 |        0.007 |
| 4000 |        12 | dynamic_64 |   0.033 |   0.381 |     0.087 |        0.007 |
| 4000 |        12 | guided_64  |   0.033 |   0.380 |     0.087 |        0.007 |
| 8000 |         1 | static     |   0.068 |   0.081 |     0.833 |        0.833 |
| 8000 |         1 | dynamic_64 |   0.068 |   0.096 |     0.704 |        0.704 |
| 8000 |         1 | guided_64  |   0.068 |   0.079 |     0.855 |        0.855 |
| 8000 |         2 | static     |   0.068 |   0.158 |     0.429 |        0.214 |
| 8000 |         2 | dynamic_64 |   0.068 |   0.190 |     0.356 |        0.178 |
| 8000 |         2 | guided_64  |   0.068 |   0.132 |     0.512 |        0.256 |
| 8000 |         4 | static     |   0.068 |   0.190 |     0.356 |        0.089 |
| 8000 |         4 | dynamic_64 |   0.068 |   0.178 |     0.379 |        0.095 |
| 8000 |         4 | guided_64  |   0.068 |   0.183 |     0.370 |        0.092 |
| 8000 |         6 | static     |   0.068 |   0.251 |     0.269 |        0.045 |
| 8000 |         6 | dynamic_64 |   0.068 |   0.223 |     0.304 |        0.051 |
| 8000 |         6 | guided_64  |   0.068 |   0.290 |     0.233 |        0.039 |
| 8000 |        12 | static     |   0.068 |   0.416 |     0.163 |        0.014 |
| 8000 |        12 | dynamic_64 |   0.068 |   0.415 |     0.163 |        0.014 |
| 8000 |        12 | guided_64  |   0.068 |   0.406 |     0.167 |        0.014 |

#### OpenMP Tasks

|    N |   Threads | Schedule   |   Tb_ms |   To_ms |   Speedup |   Efficiency |
|------|-----------|------------|---------|---------|-----------|--------------|
| 2000 |         1 | static     |   0.017 |   0.247 |     0.068 |        0.068 |
| 2000 |         1 | dynamic_64 |   0.017 |   0.243 |     0.068 |        0.068 |
| 2000 |         1 | guided_64  |   0.017 |   0.239 |     0.070 |        0.070 |
| 2000 |         2 | static     |   0.017 |   2.566 |     0.006 |        0.003 |
| 2000 |         2 | dynamic_64 |   0.017 |   2.799 |     0.006 |        0.003 |
| 2000 |         2 | guided_64  |   0.017 |   2.924 |     0.006 |        0.003 |
| 2000 |         4 | static     |   0.017 |   8.268 |     0.002 |        0.001 |
| 2000 |         4 | dynamic_64 |   0.017 |   8.244 |     0.002 |        0.001 |
| 2000 |         4 | guided_64  |   0.017 |   8.613 |     0.002 |        0.000 |
| 2000 |         6 | static     |   0.017 |  15.258 |     0.001 |        0.000 |
| 2000 |         6 | dynamic_64 |   0.017 |  16.329 |     0.001 |        0.000 |
| 2000 |         6 | guided_64  |   0.017 |  16.442 |     0.001 |        0.000 |
| 2000 |        12 | static     |   0.017 |  29.344 |     0.001 |        0.000 |
| 2000 |        12 | dynamic_64 |   0.017 |  30.417 |     0.001 |        0.000 |
| 2000 |        12 | guided_64  |   0.017 |  29.143 |     0.001 |        0.000 |
| 4000 |         1 | static     |   0.033 |   0.298 |     0.111 |        0.111 |
| 4000 |         1 | dynamic_64 |   0.033 |   0.299 |     0.111 |        0.111 |
| 4000 |         1 | guided_64  |   0.033 |   0.341 |     0.097 |        0.097 |
| 4000 |         2 | static     |   0.033 |   3.836 |     0.009 |        0.004 |
| 4000 |         2 | dynamic_64 |   0.033 |   3.705 |     0.009 |        0.004 |
| 4000 |         2 | guided_64  |   0.033 |   4.394 |     0.008 |        0.004 |
| 4000 |         4 | static     |   0.033 |   9.870 |     0.003 |        0.001 |
| 4000 |         4 | dynamic_64 |   0.033 |   9.671 |     0.003 |        0.001 |
| 4000 |         4 | guided_64  |   0.033 |   9.338 |     0.004 |        0.001 |
| 4000 |         6 | static     |   0.033 |  16.297 |     0.002 |        0.000 |
| 4000 |         6 | dynamic_64 |   0.033 |  15.013 |     0.002 |        0.000 |
| 4000 |         6 | guided_64  |   0.033 |  15.842 |     0.002 |        0.000 |
| 4000 |        12 | static     |   0.033 |  29.274 |     0.001 |        0.000 |
| 4000 |        12 | dynamic_64 |   0.033 |  29.204 |     0.001 |        0.000 |
| 4000 |        12 | guided_64  |   0.033 |  29.371 |     0.001 |        0.000 |
| 8000 |         1 | static     |   0.068 |   0.311 |     0.217 |        0.217 |
| 8000 |         1 | dynamic_64 |   0.068 |   0.309 |     0.219 |        0.219 |
| 8000 |         1 | guided_64  |   0.068 |   0.304 |     0.222 |        0.222 |
| 8000 |         2 | static     |   0.068 |   2.661 |     0.025 |        0.013 |
| 8000 |         2 | dynamic_64 |   0.068 |   2.942 |     0.023 |        0.011 |
| 8000 |         2 | guided_64  |   0.068 |   3.151 |     0.021 |        0.011 |
| 8000 |         4 | static     |   0.068 |   8.214 |     0.008 |        0.002 |
| 8000 |         4 | dynamic_64 |   0.068 |   8.221 |     0.008 |        0.002 |
| 8000 |         4 | guided_64  |   0.068 |   8.295 |     0.008 |        0.002 |
| 8000 |         6 | static     |   0.068 |  13.757 |     0.005 |        0.001 |
| 8000 |         6 | dynamic_64 |   0.068 |  14.183 |     0.005 |        0.001 |
| 8000 |         6 | guided_64  |   0.068 |  13.713 |     0.005 |        0.001 |
| 8000 |        12 | static     |   0.068 |  26.603 |     0.003 |        0.000 |
| 8000 |        12 | dynamic_64 |   0.068 |  27.143 |     0.002 |        0.000 |
| 8000 |        12 | guided_64  |   0.068 |  27.713 |     0.002 |        0.000 |

---

## Capturas de Mediciones

### Captura 1: Ejecución Secuencial
```
$ ./seq.exe --n 2000 --steps 1000 --record seq_N2000_r1.csv
Frames: 1000  Avg step (ms): 2.145
CSV written: seq_N2000_r1.csv
```

### Captura 2: Ejecución OpenMP For
```
$ ./omp_for.exe --n 2000 --steps 1000 --threads 4 --schedule static --record omp_for_N2000_T4_Sstatic_r1.csv
Frames: 1000  Avg step (ms): 4.532
CSV written: omp_for_N2000_T4_Sstatic_r1.csv
```

### Captura 3: Análisis de Speedup
```
$ python scripts/analyze.py data/results/seq_N2000_r1.csv data/results/omp_for_N2000_T4_Sstatic_r1.csv
Tb(avg ms)=2.187  To(avg ms)=4.568  Speedup=0.479
```

---

## Análisis de Resultados

Los experimentos muestran que, para los tamaños de problema evaluados (N ≤ 8000), el **paralelismo no logra superar a la versión secuencial**. En `omp_for`, el overhead de sincronización y reparto de trabajo provoca *speedup* < 1 en casi todos los casos; la eficiencia cae rápidamente conforme aumentan los hilos.  

En `omp_simd`, se observa una mejora ligera en 1 hilo gracias a la vectorización (speedup ≈ 1.19), pero esta ganancia desaparece con más hilos, ya que los costes de coordinación dominan el rendimiento.  

La versión `omp_tasks` resulta poco competitiva: el *overhead* de creación y gestión de tareas pequeñas anula cualquier beneficio, mostrando eficiencias cercanas a cero.  

En síntesis, los resultados confirman la importancia del **tamaño del problema** y la **granularidad del trabajo**: para obtener beneficios reales del paralelismo será necesario escalar a valores de N mucho mayores y optimizar la forma en que se distribuye la carga de trabajo entre hilos y tareas.  

