// src/core/physics.hpp
#pragma once
#include "state.hpp"

// Actualizar el estado con integración
void integrate(State& s, float dt);
// Aplicar rebote a un estado 
void bounce(State& s);
