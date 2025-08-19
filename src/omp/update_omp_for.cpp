#include "update_omp_for.hpp"
#include "core/physics.hpp"
#include "core/grid.hpp"
#ifdef _OPENMP
    #include <omp.h>
#endif

// Actualiza posiciones y velocidades usando OpenMP. Reajusta límites y reconstruye la grilla.
void update_step_omp_for(State& s) {
    const float dt = 1.0f/60.0f;
    // Actualiza posiciones y velocidades
    #pragma omp parallel for if(s.N>256) schedule(runtime)
    for (int i=0;i<s.N;i++) {
        s.x[i] += s.vx[i]*dt;
        s.y[i] += s.vy[i]*dt;
    }
    // Uso de condiciones de frontera
    #pragma omp parallel for if(s.N>256) schedule(runtime)
    for (int i=0;i<s.N;i++) {
        if (s.x[i] < 0.f) { s.x[i]=0.f; s.vx[i] = -s.vx[i]; }
        if (s.x[i] > s.width) { s.x[i]=float(s.width); s.vx[i] = -s.vx[i]; }
        if (s.y[i] < 0.f) { s.y[i]=0.f; s.vy[i] = -s.vy[i]; }
        if (s.y[i] > s.height){ s.y[i]=float(s.height); s.vy[i] = -s.vy[i]; }
    }
    // Reconstrucción de la cuadrícula
    Grid g(s.width, s.height, 64);
    g.build(s);
}