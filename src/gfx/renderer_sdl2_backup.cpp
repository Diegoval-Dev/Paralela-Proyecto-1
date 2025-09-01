// src/gfx/renderer_sdl2.cpp
#include "gfx/renderer.hpp"
#include <SDL2/SDL.h>
#include <memory>
#include <stdexcept>
#include <cmath>
#include <algorithm>

class SDL2Renderer : public IRenderer {
private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    int width, height;
    bool vsync;
    
    // Para el efecto trail/estela del screensaver
    SDL_Texture* trailTexture = nullptr;
    
public:
    explicit SDL2Renderer(const RendererConfig& cfg) 
        : width(cfg.width), height(cfg.height), vsync(cfg.vsync) {
        
        // Inicializar SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            throw std::runtime_error("SDL_Init failed: " + std::string(SDL_GetError()));
        }
        
        // Crear ventana
        window = SDL_CreateWindow(
            "OpenMP Screensaver",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            width, height,
            SDL_WINDOW_SHOWN
        );
        
        if (!window) {
            SDL_Quit();
            throw std::runtime_error("SDL_CreateWindow failed: " + std::string(SDL_GetError()));
        }
        
        // Crear renderer con aceleración de hardware
        Uint32 flags = SDL_RENDERER_ACCELERATED;
        if (vsync) flags |= SDL_RENDERER_PRESENTVSYNC;
        
        renderer = SDL_CreateRenderer(window, -1, flags);
        if (!renderer) {
            SDL_DestroyWindow(window);
            SDL_Quit();
            throw std::runtime_error("SDL_CreateRenderer failed: " + std::string(SDL_GetError()));
        }
        
        // Habilitar blend mode para efectos de transparencia
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        
        // Crear textura para el efecto trail
        trailTexture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET,
            width, height
        );
        
        if (!trailTexture) {
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
            throw std::runtime_error("SDL_CreateTexture failed: " + std::string(SDL_GetError()));
        }
        
        SDL_SetTextureBlendMode(trailTexture, SDL_BLENDMODE_BLEND);
        
        // Limpiar la textura inicial
        SDL_SetRenderTarget(renderer, trailTexture);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderTarget(renderer, nullptr);
    }
    
    ~SDL2Renderer() {
        if (trailTexture) SDL_DestroyTexture(trailTexture);
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        SDL_Quit();
    }
    
    void beginFrame() override {
        // Procesar eventos de SDL (cerrar ventana, etc.)
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                // En una app real, manejaríamos esto mejor
                // Por ahora solo continuamos
            }
        }
        
        // Copiar el frame anterior con transparencia para crear efecto trail
        SDL_SetRenderTarget(renderer, trailTexture);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 15); // Fade gradual
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderFillRect(renderer, nullptr);
        SDL_SetRenderTarget(renderer, nullptr);
        
        // Limpiar el framebuffer principal
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        // Renderizar la textura trail primero
        SDL_RenderCopy(renderer, trailTexture, nullptr, nullptr);
    }
    
    void drawState(const State& s) override {
        // Dibujar las partículas
        for (int i = 0; i < s.N; ++i) {
            // Extraer componentes de color ARGB
            uint32_t c = s.color[i];
            uint8_t a = (c >> 24) & 0xFF;
            uint8_t r = (c >> 16) & 0xFF;
            uint8_t g = (c >> 8) & 0xFF;
            uint8_t b = c & 0xFF;
            
            // Calcular velocidad para modular el brillo
            float speed = std::sqrt(s.vx[i]*s.vx[i] + s.vy[i]*s.vy[i]);
            float brightness = std::min(1.0f, speed / 200.0f);
            
            // Ajustar color basado en velocidad
            r = uint8_t(r * (0.5f + 0.5f * brightness));
            g = uint8_t(g * (0.5f + 0.5f * brightness));
            b = uint8_t(b * (0.5f + 0.5f * brightness));
            
            // Dibujar partícula principal
            SDL_SetRenderDrawColor(renderer, r, g, b, a);
            
            // Dibujar como un pequeño círculo (aproximado con rectángulo)
            int size = 4;
            SDL_Rect rect = {
                int(s.x[i] - size/2),
                int(s.y[i] - size/2),
                size,
                size
            };
            SDL_RenderFillRect(renderer, &rect);
            
            // Dibujar un punto más brillante en el centro
            SDL_SetRenderDrawColor(renderer, 
                std::min(255, r + 50),
                std::min(255, g + 50),
                std::min(255, b + 50),
                255
            );
            SDL_RenderDrawPoint(renderer, int(s.x[i]), int(s.y[i]));
            
            // Copiar a la textura trail
            SDL_SetRenderTarget(renderer, trailTexture);
            SDL_SetRenderDrawColor(renderer, r, g, b, 200);
            SDL_RenderFillRect(renderer, &rect);
            SDL_SetRenderTarget(renderer, nullptr);
        }
        
        // Dibujar líneas de conexión entre partículas cercanas (efecto screensaver)
        const float maxDist = 100.0f;
        const float maxDistSq = maxDist * maxDist;
        
        for (int i = 0; i < s.N - 1; ++i) {
            for (int j = i + 1; j < std::min(i + 20, s.N); ++j) { // Limitar búsqueda
                float dx = s.x[i] - s.x[j];
                float dy = s.y[i] - s.y[j];
                float distSq = dx*dx + dy*dy;
                
                if (distSq < maxDistSq) {
                    float dist = std::sqrt(distSq);
                    float alpha = 1.0f - (dist / maxDist);
                    alpha = alpha * alpha; // Curve for smoother fade
                    
                    // Color basado en la mezcla de ambas partículas
                    uint32_t c1 = s.color[i];
                    uint32_t c2 = s.color[j];
                    uint8_t r = ((c1 >> 16) & 0xFF) / 2 + ((c2 >> 16) & 0xFF) / 2;
                    uint8_t g = ((c1 >> 8) & 0xFF) / 2 + ((c2 >> 8) & 0xFF) / 2;
                    uint8_t b = (c1 & 0xFF) / 2 + (c2 & 0xFF) / 2;
                    
                    SDL_SetRenderDrawColor(renderer, r, g, b, uint8_t(alpha * 100));
                    SDL_RenderDrawLine(renderer, 
                        int(s.x[i]), int(s.y[i]),
                        int(s.x[j]), int(s.y[j])
                    );
                }
            }
        }
    }
    
    void endFrame() override {
        // Presentar el frame renderizado
        SDL_RenderPresent(renderer);
    }
};

// Factory function
RendererPtr createRenderer(const RendererConfig& cfg) {
    return std::make_unique<SDL2Renderer>(cfg);
}