// src/core/grid.hpp
#pragma once
#include "state.hpp"
#include <vector>

// Estructura para el grid
struct Grid {
    // Numero de filas y columnas
    int cols, rows;
    // Ancho y alto de cada celda
    float cellW, cellH;
    // Indice del primer elemento en cada celda y el siguiente elemento en la lista
    std::vector<int> head, next; 
    // Construir el grid
    explicit Grid(int width, int height, int wantedCells=64);
    // Llena el grid con las particulas del estado actual
    void build(const State& s);
};
