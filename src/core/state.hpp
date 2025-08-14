#pragma once
#include <vector>
#include <cstdint>
#include "types.hpp"
#include "rng.hpp"

using namespace std;

struct State {
    // Número de partículas
    int N;
    // Tamaño de la ventana
    int width, height;

    // Vectores con posiciones y velocidades
    vector<float> x, y, vx, vy;
    vector<uint32_t> color;

    // Esto es el constructor
    explicit State(int n, int w, int h, uint32_t seed=1234)
      : N(n), width(w), height(h),
        x(n), y(n), vx(n), vy(n), color(n)
    {
        // Se asignan valores aleatorios a las posiciones y velocidades
        RNG rng(seed);
        for (int i=0;i<N;i++) {
            x[i] = rng.uniform(0.0f, float(w));
            y[i] = rng.uniform(0.0f, float(h));
            vx[i] = rng.uniform(-120.0f, 120.0f);
            vy[i] = rng.uniform(-120.0f, 120.0f);
            // Colores semi aleatorios, se inicia con 0xFF y se le asigna un color aleatorio
            color[i] = 0xFF000000u | (rng.u32() & 0x00FFFFFFu);
        }
    }
};
