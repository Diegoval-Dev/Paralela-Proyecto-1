# Anexo 3 – Bitácora de Pruebas

## Metodología
- Se realizaron ejecuciones con tamaños de problema `N = 2000, 4000, 8000`.  
- Se variaron los **número de hilos (1–12)** y políticas de *schedule* (`static`, `dynamic:64`, `guided:64`).  
- Se compararon las tres implementaciones principales:  
  - `omp_for`  
  - `omp_simd`  
  - `omp_tasks`  
- Las métricas reportadas son:
  - **Speedup**: tiempo secuencial / tiempo paralelo.  
  - **Eficiencia**: speedup / número de hilos.  

Los resultados están documentados en los gráficos generados con `analyze.py` y guardados en `data/results/`.

---

## Resultados

### 1. `omp_tasks` (omp_tasks_speedup.png, omp_tasks_efficiency.png)
- **Speedup**: prácticamente nulo para todos los tamaños (2000, 4000, 8000) y hasta 12 hilos.  
  - Las curvas están pegadas en ~0.0–0.1.  
  - La línea ideal crece hasta 12×, pero la implementación no escala.  
- **Eficiencia**: muy baja (<0.2 incluso con 1 hilo) y cae a 0 casi de inmediato.  
- **Interpretación**: la implementación con *tasks* aún está incompleta (colisiones no implementadas). El overhead de tareas domina y no se observan mejoras reales.

### 2. `omp_for` (plot_speedup.png, plot_efficiency.png)
- **Speedup**:  
  - Para N=2000: máximo ~0.4–0.5× con 1 hilo y cae con más hilos.  
  - Para N=4000 y N=8000: patrones similares, llegando apenas a ~0.6–0.7× en 1 hilo y decreciendo.  
  - En ningún caso se acerca a la curva ideal.  
- **Eficiencia**:  
  - Inicia en ~0.3–0.7 con 1 hilo (dependiendo de N).  
  - Con 2 hilos baja drásticamente a 0.1–0.2.  
  - De 4 a 12 hilos la eficiencia es residual (<0.1).  
- **Interpretación**: indica overhead fuerte en la versión *for*, probablemente por tamaño de problema demasiado pequeño para amortizar costos de sincronización.

### 3. `omp_simd` (omp_simd_speedup.png, omp_simd_efficiency.png)
- **Speedup**:  
  - Valores iniciales entre 0.5–0.8× con 1 hilo.  
  - Al aumentar hilos, el speedup decrece en lugar de crecer.  
  - La curva ideal queda muy por encima de las medidas.  
- **Eficiencia**:  
  - Muy buena en 1 hilo (0.6–0.8).  
  - Se degrada rápidamente (<0.1 desde 4 hilos en adelante).  
- **Interpretación**: el *pragma omp simd* mejora el rendimiento secuencial (vectorización), pero no hay escalamiento con hilos; el beneficio se agota rápido.

---

## Conclusiones
1. **omp_tasks** no muestra *speedup* debido a implementación incompleta.  
2. **omp_for** tiene overhead elevado y no escala para N ≤ 8000; se necesitan problemas más grandes para observar mejoras.  
3. **omp_simd** sí aprovecha vectorización: buen rendimiento en 1 hilo, pero baja eficiencia paralela.  
4. En general, los experimentos confirman que el tamaño de problema (N) es demasiado pequeño para que las implementaciones paralelas muestren *speedup* notorio. 
5. Los gráficos en `data/results/` (speedup y eficiencia) son evidencia de la falta de mejora en paralelo y servirán para la discusión en el reporte. 
