#include "update_omp_tasks.hpp"
#include "core/physics.hpp"
#include "core/grid.hpp"
#ifdef _OPENMP
  #include <omp.h>
#endif

// Actualiza posiciones y velocidades usando OpenMP. Reajusta límites y reconstruye la grilla.
void update_step_omp_tasks(State& s) {
    const float dt = 1.0f/60.0f;
    // Integración + rebotes en paralelo (igual que omp_for)
    #pragma omp parallel
    {
      #pragma omp for schedule(guided)
      for (int i=0;i<s.N;i++) {
        s.x[i] += s.vx[i]*dt;
        s.y[i] += s.vy[i]*dt;
      }
      #pragma omp for schedule(guided)
      for (int i=0;i<s.N;i++) {
        if (s.x[i] < 0.f) { s.x[i]=0.f; s.vx[i] = -s.vx[i]; }
        if (s.x[i] > s.width) { s.x[i]=float(s.width); s.vx[i] = -s.vx[i]; }
        if (s.y[i] < 0.f) { s.y[i]=0.f; s.vy[i] = -s.vy[i]; }
        if (s.y[i] > s.height){ s.y[i]=float(s.height); s.vy[i] = -s.vy[i]; }
      }
    }
    // Construir grid y procesar por celdas con tasks
    Grid g(s.width, s.height, 64);
    g.build(s);

    // Procesar colisiones por celda
    #pragma omp parallel
    {
      #pragma omp single nowait
      for (int cy=0; cy<g.rows; ++cy) {
        for (int cx=0; cx<g.cols; ++cx) {
          #pragma omp task firstprivate(cx,cy) shared(g,s)
          {
            int head = g.head[cy*g.cols + cx];
            // TODO: Procesar colisiones en la celda
            (void)head;
          }
        }
      }
    }
}