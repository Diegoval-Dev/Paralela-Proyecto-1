// src/core/rng.hpp
#pragma once
#include <cstdint>

// Generador de números aleatorios xorshift32
struct RNG {
    uint32_t s;
    // Constructor con semilla
    explicit RNG(uint32_t seed=1u): s(seed) {}
    // Devuelve un entero de 32 bits usando el xorshift32
    inline uint32_t u32() {
        uint32_t x = s;
        x ^= x << 13; x ^= x >> 17; x ^= x << 5;
        s = x; return x;
    }
    // Devuelve un número aleatorio uniforme en el rango [a, b)
    inline float uniform(float a, float b) {
        return a + (b-a) * ( (u32() >> 8) * (1.0f/16777216.0f) ); 
    }
};
