
#include "update_seq.hpp"
#include "core/physics.hpp"
#include "core/grid.hpp"

// Actualiza el estado del sistema: integra física, aplica rebotes y organiza objetos en una cuadrícula espacial.
void update_step_seq(State& s) {
    integrate(s, 1.0f/60.0f);
    bounce(s);
    Grid g(s.width, s.height, 64);
    g.build(s);
}