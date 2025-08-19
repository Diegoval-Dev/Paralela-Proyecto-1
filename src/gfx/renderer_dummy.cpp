#include "gfx/renderer.hpp"
#include <iostream>

// Renderizador de prueba que no hace nada
struct DummyRenderer : IRenderer {
    void beginFrame() override {}
    void drawState(const State&) override {}
    void endFrame() override {}
};

// Crea una instancia del renderizador
RendererPtr createRenderer(const RendererConfig&) {
    return std::make_unique<DummyRenderer>();
}