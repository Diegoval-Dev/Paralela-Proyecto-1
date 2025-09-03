# Screensaver Paralelo con OpenMP

Este proyecto implementa un **screensaver paralelo** en C++ utilizando **OpenMP** y la librería **SDL2**. 

## Descripción
El programa dibuja un conjunto de partículas en movimiento que rebotan en los bordes de la pantalla. 
Se implementan distintas versiones:
- **Secuencial (seq)**
- **OpenMP parallel for (omp_for)**
- **OpenMP simd (omp_simd)**
- **OpenMP tasks (omp_tasks)**

Incluye herramientas de **benchmarking** y **gráficas** de *speedup* y *eficiencia*.

## Requisitos
- **CMake >= 3.20**
- **Compilador C++17** con soporte OpenMP (g++, clang, MSVC)
- **SDL2** (solo si se usa la versión visual)
- **Python 3** + `matplotlib` y `pandas` (para análisis de resultados)

## Uso

Para ejecutar el proyecto se incluye un script en PowerShell que automatiza todo el proceso:

```powershell
# Compila, configura SDL2 y ejecuta el screensaver
powershell -ExecutionPolicy Bypass -File .\quick_setup.ps1
```

Si deseas restaurar el renderer por defecto:

```powershell
powershell -ExecutionPolicy Bypass -File .\quick_setup.ps1 -RestoreRenderer
```

## Benchmarks
Los scripts de PowerShell y Python en `scripts/` permiten:
- Ejecutar múltiples configuraciones (`run_bench_*.ps1`)
- Resumir resultados (`summarize_speedup_*.ps1`)
- Graficar speedup y eficiencia (`analyze.py`)

Ejemplo de resultado:

![speedup](data/results/speedup_plot.png)

## Créditos
- **Equipo**: Proyecto 1, Computación Paralela y Distribuida  
- **Docente**: Marlon Fuentes  
- **Universidad del Valle de Guatemala** (UVG)
