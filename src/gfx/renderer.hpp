#pragma once
#include <memory>
#include "core/types.hpp"
#include "core/state.hpp"

// Interfaz para el renderizador
struct IRenderer {
    virtual ~IRenderer() = default;
    virtual void beginFrame() = 0;
    virtual void drawState(const State& s) = 0;
    virtual void endFrame() = 0;
};
using RendererPtr = std::unique_ptr<IRenderer>;

// Crea una instancia del renderizador
RendererPtr createRenderer(const RendererConfig&);