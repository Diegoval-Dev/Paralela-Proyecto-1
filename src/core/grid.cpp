// src/core/grid.cpp
#include "grid.hpp"
#include <algorithm>
#include <limits>

// Construir la Grid
Grid::Grid(int width, int height, int wantedCells) {
    // Calcular el número de columnas y filas basado en el número de celdas deseadas
    cols = rows = wantedCells;
    cellW = width / float(cols);
    cellH = height / float(rows);
}

// Construccion de la estructura del Grid
void Grid::build(const State& s) {
    // Inicializar las estructuras
    head.assign(cols*rows, -1);
    next.assign(s.N, -1);
    // Insertar cada particula en su celda que le corresponde
    for (int i=0;i<s.N;i++) {
        int cx = std::min(cols-1, std::max(0, int(s.x[i] / cellW)));
        int cy = std::min(rows-1, std::max(0, int(s.y[i] / cellH)));
        int idx = cy*cols + cx;
        // Inserta al inicio de la lista de esa celda
        next[i] = head[idx];
        head[idx] = i;
    }
}