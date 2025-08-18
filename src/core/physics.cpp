// src/core/physics.cpp
#include "physics.hpp"
#include <algorithm>

// Actualiza posiciones con integración
void integrate(State& s, float dt) {
    // Actualiza las posiciones de las partículas
    for (int i=0;i<s.N;i++) {
        s.x[i] += s.vx[i] * dt;
        s.y[i] += s.vy[i] * dt;
    }
}

// Rebota las partículas al chocar con los bordes del área
void bounce(State& s) {
    // Rebotar en los bordes
    for (int i=0;i<s.N;i++) {
        if (s.x[i] < 0.f) { s.x[i]=0.f; s.vx[i] = -s.vx[i]; }
        if (s.x[i] > s.width) { s.x[i]=float(s.width); s.vx[i] = -s.vx[i]; }
        if (s.y[i] < 0.f) { s.y[i]=0.f; s.vy[i] = -s.vy[i]; }
        if (s.y[i] > s.height){ s.y[i]=float(s.height); s.vy[i] = -s.vy[i]; }
    }
}