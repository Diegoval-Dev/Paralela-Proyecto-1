#include "update_omp_simd.hpp"
#include "core/physics.hpp"
#include "core/grid.hpp"
#ifdef _OPENMP
  #include <omp.h>
#endif

// Actualiza posiciones y velocidades usando OpenMP. Reajusta límites y reconstruye la grilla.
void update_step_omp_simd(State& s) {
    const float dt = 1.0f/60.0f;
    // Actualiza posiciones y velocidades
    #pragma omp parallel
    {
      #pragma omp for schedule(static)
      for (int i=0;i<s.N;i++) {
        #pragma omp simd
        for (int k=0;k<1;k++) { // simd forzado sobre la operación (truco simple)
          s.x[i] += s.vx[i]*dt;
          s.y[i] += s.vy[i]*dt;
        }
      }
      // Uso de condiciones de frontera
      #pragma omp for schedule(static)
      for (int i=0;i<s.N;i++) {
        float xi=s.x[i], yi=s.y[i];
        if (xi < 0.f) { xi=0.f; s.vx[i] = -s.vx[i]; }
        if (xi > s.width) { xi=float(s.width); s.vx[i] = -s.vx[i]; }
        if (yi < 0.f) { yi=0.f; s.vy[i] = -s.vy[i]; }
        if (yi > s.height){ yi=float(s.height); s.vy[i] = -s.vy[i]; }
        s.x[i]=xi; s.y[i]=yi;
      }
    }
    // Reconstrucción de la cuadrícula
    Grid g(s.width, s.height, 64);
    g.build(s);
}