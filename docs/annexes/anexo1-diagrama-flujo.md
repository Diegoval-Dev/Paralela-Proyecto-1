# Anexo 1 - Diagrama de Flujo del Programa

## Pasos principales

1. **Captura de argumentos (src/app/main.cpp)**
   - Flags soportados:
     - `--n`: número de partículas.
     - `--steps`: pasos de simulación.
     - `--threads`: número de hilos OpenMP (0 = auto).
     - `--schedule`: política de schedule (`static`, `dynamic:K`, `guided:K`).
     - `--record`: archivo CSV de salida.
     - `--help`: muestra la ayuda.
   - Programación defensiva: validación de rangos, mensaje de error si hay parámetros inválidos.

2. **Inicialización**
   - Se crea un `State` (src/core/state.hpp) con arreglos `x, y, vx, vy, color`.
   - `init_state` asigna valores aleatorios reproducibles (semilla fija).
   - Se realiza un *warm-up* antes de medir para estabilizar.

3. **Bucle principal**
   - Se llama a `step_fn`, puntero a la versión elegida:
     - `update_seq` (secuencial).
     - `update_omp_for`.
     - `update_omp_simd`.
     - `update_omp_tasks`.
   - Se ejecuta el renderizador:
     - `renderer_dummy` (modo benchmark).
     - `renderer_sdl2` (modo visual con SDL2).
   - El tiempo de cada iteración se mide con `Timer`.
   - Si se especificó `--record`, se escribe en CSV (`frame,time_ms`).

4. **Mecanismos de paralelismo y sincronía**
   - `#pragma omp parallel for` con `schedule(runtime)` en `update_omp_for`.
   - `#pragma omp simd` en `update_omp_simd`.
   - `#pragma omp parallel` + `#pragma omp single` + `#pragma omp task` en `update_omp_tasks`.
   - Barreras implícitas de OpenMP y `#pragma omp taskwait`.

5. **Despliegue de resultados**
   - Análisis posterior con scripts de Python (`analyze.py`, `analyze_plots*.py`) que producen gráficas de speedup y eficiencia.
