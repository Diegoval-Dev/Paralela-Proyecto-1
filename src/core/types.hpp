#pragma once
#include <cstdint>

// Esto es para la configuración del renderizador
struct RendererConfig {
    int width;
    int height;
    bool vsync = false;
};
