#pragma once
#include <vector>
#include <cstdint>
#include "types.hpp"
#include "rng.hpp"

using namespace std;

struct State {
    int N;
    int width, height;
    vector<float> x, y, vx, vy;
    vector<uint32_t> color;

    explicit State(int n, int w, int h, uint32_t seed=1234)
      : N(n), width(w), height(h),
        x(n), y(n), vx(n), vy(n), color(n)
    {
        RNG rng(seed);
        for (int i=0;i<N;i++) {
            x[i] = rng.uniform(0.0f, float(w));
            y[i] = rng.uniform(0.0f, float(h));
            vx[i] = rng.uniform(-120.0f, 120.0f);
            vy[i] = rng.uniform(-120.0f, 120.0f);
            color[i] = 0xFF000000u | (rng.u32() & 0x00FFFFFFu);
        }
    }
};
