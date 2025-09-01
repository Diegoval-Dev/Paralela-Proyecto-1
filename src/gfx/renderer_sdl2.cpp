#include "gfx/renderer.hpp"
#include <SDL2/SDL.h>
#include <memory>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <vector>
#include <deque>
#include <random>

// =============== Utilidades ===============
constexpr float PI = 3.14159265359f;
constexpr float TWO_PI = 2.0f * PI;

struct ParticleTrail
{
    std::deque<std::pair<float, float>> positions;
    std::deque<uint32_t> colors;
    static constexpr size_t MAX_TRAIL_LENGTH = 30;
    void addPosition(float x, float y, uint32_t color)
    {
        positions.push_front({x, y});
        colors.push_front(color);
        if (positions.size() > MAX_TRAIL_LENGTH)
        {
            positions.pop_back();
            colors.pop_back();
        }
    }
    void clear()
    {
        positions.clear();
        colors.clear();
    }
};

class SDL2UltraRenderer : public IRenderer
{
private:
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    int width, height;
    bool vsync;

    // Texturas
    SDL_Texture *trailTexture = nullptr;      // acumulación de estelas
    SDL_Texture *glowTexture = nullptr;       // bloom barato
    SDL_Texture *backgroundTexture = nullptr; // fondo valle nocturno

    // Estado general
    std::vector<ParticleTrail> trails; // para modo clásico
    float time = 0.0f;
    int frameCount = 0;

    // Escena valle
    struct Star
    {
        float x, y, size, twinkleSpeed, baseAlpha;
    };
    std::vector<Star> stars;
    std::vector<int> ridgeFarY, ridgeNearY; // siluetas por columna
    int horizonY = 0;
    std::mt19937 rng{1337u};

    // Fuegos artificiales (modo 9)
    struct Rocket
    {
        float x, y, vx, vy, fuse, targetY; // targetY: altura (y) a la que debe explotar si llega
        float lastX, lastY;
        SDL_Color color;
    };
    struct Spark
    {
        float x, y, vx, vy, life, ttl, lastX, lastY;
        SDL_Color color;
    };
    struct Shockwave
    {
        float x, y, r, dr, alpha;
    };
    std::vector<Rocket> rockets;
    std::vector<Spark> sparks;
    std::vector<Shockwave> waves;
    float spawnAccumulator = 0.0f;

    enum VisualMode
    {
        MODE_CLASSIC = 0,
        MODE_FIREWORKS = 1
    };
    int visualMode = MODE_CLASSIC;

public:
    explicit SDL2UltraRenderer(const RendererConfig &cfg)
        : width(cfg.width), height(cfg.height), vsync(cfg.vsync)
    {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
        {
            throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());
        }
        Uint32 windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE; // Agregar flag resizable
        if (width == 0 && height == 0)
        {
            windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
            SDL_DisplayMode dm;
            SDL_GetDesktopDisplayMode(0, &dm);
            width = dm.w;
            height = dm.h;
        }
        window = SDL_CreateWindow("Ultra Screensaver - OpenMP [1: Clásico, 9: Fuegos artificiales, F11 fullscreen, ESC]", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, windowFlags);
        if (!window)
        {
            SDL_Quit();
            throw std::runtime_error(std::string("SDL_CreateWindow failed: ") + SDL_GetError());
        }
        Uint32 rendererFlags = SDL_RENDERER_ACCELERATED;
        if (vsync)
            rendererFlags |= SDL_RENDERER_PRESENTVSYNC;
        renderer = SDL_CreateRenderer(window, -1, rendererFlags);
        if (!renderer)
        {
            SDL_DestroyWindow(window);
            SDL_Quit();
            throw std::runtime_error(std::string("SDL_CreateRenderer failed: ") + SDL_GetError());
        }
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        
        // Inicialización completa
        initializeResources();
        
        rockets.reserve(64);
        sparks.reserve(4096);
        waves.reserve(128);
    }
    
    ~SDL2UltraRenderer()
    {
        cleanupTextures();
        if (renderer)
            SDL_DestroyRenderer(renderer);
        if (window)
            SDL_DestroyWindow(window);
        SDL_Quit();
    }

private:
    // ---------- Gestión de recursos ----------
    void cleanupTextures()
    {
        if (trailTexture) {
            SDL_DestroyTexture(trailTexture);
            trailTexture = nullptr;
        }
        if (glowTexture) {
            SDL_DestroyTexture(glowTexture);
            glowTexture = nullptr;
        }
        if (backgroundTexture) {
            SDL_DestroyTexture(backgroundTexture);
            backgroundTexture = nullptr;
        }
    }
    
    void initializeResources()
    {
        // Obtener dimensiones actuales de la ventana
        SDL_GetWindowSize(window, &width, &height);
        
        cleanupTextures();
        createEffectTextures();
        buildNightValleyBackground();
        initStars(420);
    }
    
    void handleResize()
    {
        int newWidth, newHeight;
        SDL_GetWindowSize(window, &newWidth, &newHeight);
        
        if (newWidth != width || newHeight != height) {
            width = newWidth;
            height = newHeight;
            initializeResources();
            
            // Limpiar trails cuando cambia el tamaño
            for (auto& trail : trails) {
                trail.clear();
            }
        }
    }

    // ---------- Inicialización ----------
    void createEffectTextures()
    {
        trailTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height);
        glowTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height);
        backgroundTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height);
        
        if (!trailTexture || !glowTexture || !backgroundTexture) {
            throw std::runtime_error(std::string("Failed to create textures: ") + SDL_GetError());
        }
        
        SDL_SetTextureBlendMode(trailTexture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureBlendMode(glowTexture, SDL_BLENDMODE_ADD);
        SDL_SetTextureBlendMode(backgroundTexture, SDL_BLENDMODE_BLEND);
        
        // Limpiar trail texture inicialmente
        SDL_SetRenderTarget(renderer, trailTexture);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        SDL_SetRenderTarget(renderer, nullptr);
    }

    static float smoothNoise1D(int x, int seed = 0)
    {
        int n = x + seed * 57;
        n = (n << 13) ^ n;
        float nn = 1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f;
        return nn; // [-1,1]
    }
    static float fbm1D(float x, int octaves, float lac, float gain, int seed)
    {
        float amp = 1.0f, freq = 1.0f, sum = 0.0f, norm = 0.0f;
        for (int i = 0; i < octaves; ++i)
        {
            sum += smoothNoise1D(int(std::floor(x * freq)), seed + i) * amp;
            norm += amp;
            amp *= gain;
            freq *= lac;
        }
        return sum / std::max(0.0001f, norm);
    }

    void buildNightValleyBackground()
    {
        horizonY = int(height * 0.62f);
        ridgeFarY.assign(width, horizonY + 20);
        ridgeNearY.assign(width, horizonY + 60);
        for (int x = 0; x < width; ++x)
        {
            float nx = x * 0.015f;
            int yFar = horizonY - int(60 * (fbm1D(nx, 4, 2.0f, 0.55f, 11) * 0.5f + 0.5f));
            int yNear = horizonY + 50 - int(130 * (fbm1D(nx * 0.7f + 100.0f, 5, 2.0f, 0.5f, 21) * 0.5f + 0.5f));
            ridgeFarY[x] = std::clamp(yFar, 0, height - 1);
            ridgeNearY[x] = std::clamp(yNear, 0, height - 1);
        }
        // Render a backgroundTexture
        SDL_SetRenderTarget(renderer, backgroundTexture);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        // Cielo
        for (int y = 0; y < height; ++y)
        {
            float t = float(y) / float(std::max(1, horizonY));
            Uint8 r = Uint8(10 + 10 * (1.0f - t));
            Uint8 g = Uint8(14 + 30 * (1.0f - t));
            Uint8 b = Uint8(30 + 80 * (1.0f - t));
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            SDL_RenderDrawLine(renderer, 0, y, width, y);
        }
        // Luna con halo
        drawGlowingCircleRaw(int(width * 0.15f), int(horizonY * 0.35f), 40, 240, 240, 255);
        drawGlowingCircleRaw(int(width * 0.15f), int(horizonY * 0.35f), 26, 255, 255, 255);
        // Cordilleras
        for (int x = 0; x < width; ++x)
        {
            SDL_SetRenderDrawColor(renderer, 20, 22, 36, 255);
            SDL_RenderDrawLine(renderer, x, ridgeFarY[x], x, height);
        }
        for (int x = 0; x < width; ++x)
        {
            SDL_SetRenderDrawColor(renderer, 10, 12, 20, 255);
            SDL_RenderDrawLine(renderer, x, ridgeNearY[x], x, height);
        }
        // Neblina
        for (int y = horizonY + 10; y < height; ++y)
        {
            float t = float(y - (horizonY + 10)) / float(std::max(1, height - (horizonY + 10)));
            Uint8 a = Uint8(80 * (1.0f - t));
            SDL_SetRenderDrawColor(renderer, 100, 120, 160, a);
            SDL_RenderDrawLine(renderer, 0, y, width, y);
        }
        SDL_SetRenderTarget(renderer, nullptr);
    }

    void initStars(int count)
    {
        std::uniform_real_distribution<float> rx(0.0f, float(width));
        std::uniform_real_distribution<float> ry(0.0f, float(horizonY - 5));
        std::uniform_real_distribution<float> rsize(0.6f, 2.2f);
        std::uniform_real_distribution<float> rtw(0.4f, 2.5f);
        std::uniform_real_distribution<float> ralpha(120.0f, 220.0f);
        stars.clear();
        stars.reserve(count);
        for (int i = 0; i < count; ++i)
            stars.push_back({rx(rng), ry(rng), rsize(rng), rtw(rng), ralpha(rng)});
    }

    // ---------- Eventos ----------
    void handleEvents()
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            { /* noop */
            }
            else if (e.type == SDL_WINDOWEVENT)
            {
                if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
                    e.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    handleResize();
                }
            }
            else if (e.type == SDL_KEYDOWN)
            {
                switch (e.key.keysym.sym)
                {
                case SDLK_1:
                    visualMode = MODE_CLASSIC;
                    break;
                case SDLK_9:
                    visualMode = MODE_FIREWORKS;
                    break;
                case SDLK_F11:
                    toggleFullscreen();
                    break;
                case SDLK_ESCAPE:
                {
                    SDL_Event quit;
                    quit.type = SDL_QUIT;
                    SDL_PushEvent(&quit);
                }
                break;
                }
            }
        }
    }
    
    void toggleFullscreen()
    {
        Uint32 flags = SDL_GetWindowFlags(window);
        SDL_SetWindowFullscreen(window, (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
        
        // Esperar un frame para que SDL procese el cambio
        SDL_Delay(16);
        handleResize();
    }

public:
    // ---------- Ciclo de render ----------
    void beginFrame() override
    {
        handleEvents();
        time += 0.016f;
        ++frameCount;
        switch (visualMode)
        {
        case MODE_FIREWORKS:
            applyValleyBackground();
            updateFireworks();
            break;
        default:
            applyClassicEffect();
            break;
        }
    }

    void drawState(const State &s) override
    {
        if (visualMode == MODE_FIREWORKS)
        {
            drawFireworksMode();
            return;
        }
        // Modo clásico (usa el State original)
        if (trails.size() != size_t(s.N))
            trails.resize(s.N);
        drawClassicMode(s);
        for (int i = 0; i < s.N; ++i)
            trails[i].addPosition(s.x[i], s.y[i], s.color[i]);
    }

    void endFrame() override
    {
        if (visualMode == MODE_FIREWORKS)
            applyBloomEffect();
        SDL_RenderPresent(renderer);
    }

private:
    // ---------- Fondos ----------
    void applyClassicEffect()
    {
        SDL_SetRenderTarget(renderer, trailTexture);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 25); // desvanecer estelas
        SDL_RenderFillRect(renderer, nullptr);
        SDL_SetRenderTarget(renderer, nullptr);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, trailTexture, nullptr, nullptr);
    }

    void applyValleyBackground()
    {
        // Atenuar trail para persistencia suave
        SDL_SetRenderTarget(renderer, trailTexture);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 14);
        SDL_RenderFillRect(renderer, nullptr);
        SDL_SetRenderTarget(renderer, nullptr);
        // Fondo precompuesto
        SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);
        // Estrellas
        for (const auto &s : stars)
        {
            float tw = 0.5f + 0.5f * std::sin(time * s.twinkleSpeed + s.x * 0.01f);
            Uint8 a = Uint8(std::clamp(s.baseAlpha * tw, 0.0f, 255.0f));
            SDL_SetRenderDrawColor(renderer, 230, 235, 255, a);
            int rr = std::max(1, int(s.size));
            for (int r = 0; r < rr; ++r)
            {
                SDL_RenderDrawPoint(renderer, int(s.x), int(s.y) + r);
                SDL_RenderDrawPoint(renderer, int(s.x) + r, int(s.y));
            }
        }
        // Componer estelas aditivamente
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
        SDL_RenderCopy(renderer, trailTexture, nullptr, nullptr);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    }

    // ---------- Clásico ----------
    void drawClassicMode(const State &s)
    {
        // Trails de líneas
        for (int i = 0; i < s.N; ++i)
        {
            auto &tr = trails[i];
            for (size_t j = 1; j < tr.positions.size(); ++j)
            {
                float a = 1.0f - float(j) / float(tr.positions.size());
                uint32_t c = tr.colors[j];
                Uint8 r = (c >> 16) & 0xFF, g = (c >> 8) & 0xFF, b = c & 0xFF;
                SDL_SetRenderDrawColor(renderer, Uint8(r * a), Uint8(g * a), Uint8(b * a), Uint8(255 * a * a));
                SDL_RenderDrawLine(renderer, int(tr.positions[j - 1].first), int(tr.positions[j - 1].second), int(tr.positions[j].first), int(tr.positions[j].second));
            }
        }
        drawParticlesWithGlow(s);
        drawConnections(s, 150.0f);
    }
    void drawParticlesWithGlow(const State &s)
    {
        for (int i = 0; i < s.N; ++i)
        {
            uint32_t c = s.color[i];
            Uint8 r = (c >> 16) & 0xFF, g = (c >> 8) & 0xFF, b = c & 0xFF;
            for (int radius = 8; radius > 0; --radius)
            {
                Uint8 a = Uint8(64 * (1.0f - float(radius) / 8.0f));
                SDL_SetRenderDrawColor(renderer, r, g, b, a);
                for (int ang = 0; ang < 360; ang += 30)
                {
                    float rad = ang * PI / 180.0f;
                    int px = int(s.x[i] + radius * std::cos(rad));
                    int py = int(s.y[i] + radius * std::sin(rad));
                    SDL_RenderDrawPoint(renderer, px, py);
                }
            }
            SDL_SetRenderDrawColor(renderer, std::min(255, int(r) + 100), std::min(255, int(g) + 100), std::min(255, int(b) + 100), 255);
            SDL_Rect core = {int(s.x[i] - 2), int(s.y[i] - 2), 5, 5};
            SDL_RenderFillRect(renderer, &core);
        }
    }
    void drawConnections(const State &s, float maxDist)
    {
        float maxSq = maxDist * maxDist;
        for (int i = 0; i < s.N - 1; ++i)
        {
            for (int j = i + 1; j < std::min(i + 30, s.N); ++j)
            {
                float dx = s.x[i] - s.x[j], dy = s.y[i] - s.y[j];
                float d2 = dx * dx + dy * dy;
                if (d2 < maxSq)
                {
                    float d = std::sqrt(d2);
                    float a = 1.0f - (d / maxDist);
                    a = a * a * a;
                    uint32_t c1 = s.color[i], c2 = s.color[j];
                    Uint8 r = ((c1 >> 16) & 0xFF) / 2 + ((c2 >> 16) & 0xFF) / 2;
                    Uint8 g = ((c1 >> 8) & 0xFF) / 2 + ((c2 >> 8) & 0xFF) / 2;
                    Uint8 b = (c1 & 0xFF) / 2 + (c2 & 0xFF) / 2;
                    SDL_SetRenderDrawColor(renderer, r, g, b, Uint8(a * 150));
                    SDL_RenderDrawLine(renderer, int(s.x[i]), int(s.y[i]), int(s.x[j]), int(s.y[j]));
                    if (a > 0.5f)
                    {
                        int mx = int((s.x[i] + s.x[j]) / 2), my = int((s.y[i] + s.y[j]) / 2);
                        SDL_SetRenderDrawColor(renderer, 255, 255, 255, Uint8(a * 100));
                        SDL_Rect mid = {mx - 1, my - 1, 3, 3};
                        SDL_RenderFillRect(renderer, &mid);
                    }
                }
            }
        }
    }

    // ---------- Fuegos artificiales ----------
    void spawnRocket()
    {
        std::uniform_real_distribution<float> rx(width * 0.15f, width * 0.85f);
        std::uniform_real_distribution<float> rvx(-50.0f, 50.0f);
        std::uniform_real_distribution<float> rvy(-620.0f, -460.0f);
        std::uniform_real_distribution<float> rfuse(1.4f, 2.4f);
        std::uniform_int_distribution<int> rcol(0, 8);

        // Rango vertical ampliado (más aleatoriedad). y menor = más alto en pantalla.
        float minY = std::max(40.0f, float(horizonY) - 420.0f);          // límite superior (alto)
        float maxY = std::max(minY + 40.0f, float(horizonY) - 100.0f);    // límite inferior (más bajo)
        std::uniform_real_distribution<float> rTargetY(minY, maxY);

        static SDL_Color pal[] = {
            {255, 220, 180, 255},
            {255, 214, 98, 255},
            {255, 240, 170, 255},
            {255, 120, 120, 255},
            {120, 200, 255, 255},
            {130, 255, 160, 255},
            {220, 140, 255, 255},
            {255, 170, 230, 255},
            {255, 255, 255, 255}
        };

        float x = rx(rng), y = float(height) - 4.0f;
        float targetY = rTargetY(rng);
        rockets.push_back({x, y, rvx(rng), rvy(rng), rfuse(rng), targetY, x, y, pal[rcol(rng)]});
    }

    void explodeRocket(const Rocket &r)
    {
        // anillo de choque todavía más lento (antes dr=90, antes de eso 140)
        waves.push_back({r.x, r.y, 2.0f, 60.0f, 1.0f});

        // chispas aún más lentas y de mayor duración (antes 70-170 / 1.8-2.6)
        std::uniform_real_distribution<float> rspd(50.0f, 120.0f);
        std::uniform_real_distribution<float> rttl(2.2f, 3.2f);
        std::uniform_real_distribution<float> rj(0.85f, 1.2f); // variación de brillo/color

        for (int i = 0; i < 120; ++i)
        {
            float ang = (float(i) / 120.0f) * TWO_PI + (rand() % 100) * 0.0009f;
            float spd = rspd(rng);
            float vx = std::cos(ang) * spd;
            float vy = std::sin(ang) * spd;

            // "shimmer" cálido: acerca el color a dorado/blanco
            float j = rj(rng);
            SDL_Color base = r.color;
            SDL_Color col = {
                (Uint8)std::min(255, int(base.r * j + 25)),
                (Uint8)std::min(255, int(base.g * j + 15)),
                (Uint8)std::min(255, int(base.b * j)),
                255};

            sparks.push_back({r.x, r.y, vx, vy, 0.0f, rttl(rng), r.x, r.y, col});
        }
    }

    void updateFireworks()
    {
        const float dt = 0.016f;
        const float g = 260.0f; // caída más suave

        // lanzamientos un poco más espaciados
        spawnAccumulator += dt;
        const float spawnEvery = 0.65f;
        while (spawnAccumulator >= spawnEvery)
        {
            spawnAccumulator -= spawnEvery;
            spawnRocket();
        }

        SDL_SetRenderTarget(renderer, trailTexture);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);

        // Cohetes
        for (auto it = rockets.begin(); it != rockets.end();)
        {
            Rocket &r = *it;
            r.fuse -= dt;
            r.vy += g * dt * 0.25f; // suben y se frenan suavemente
            r.lastX = r.x;
            r.lastY = r.y;
            r.x += r.vx * dt;
            r.y += r.vy * dt;

            drawThickLine(int(r.lastX), int(r.lastY), int(r.x), int(r.y), 3, 255, 255, 255, 180);

            // Ahora explota si:
            // 1) se acaba el fusible, 2) pierde demasiada velocidad ascendente, 3) alcanza targetY aleatorio
            bool explode = (r.fuse <= 0.0f) || (r.vy > -5.0f) || (r.y <= r.targetY);
            if (explode)
            {
                explodeRocket(r);
                it = rockets.erase(it);
            }
            else
            {
                ++it;
            }
        }

        // Chispas
        for (auto it = sparks.begin(); it != sparks.end();)
        {
            Spark &p = *it;
            p.life += dt;
            // gravedad reducida para caída más lenta (antes g * dt)
            p.vy += g * dt * 0.8f;
            p.lastX = p.x;
            p.lastY = p.y;
            p.x += p.vx * dt;
            p.y += p.vy * dt;

            float t = std::clamp(1.0f - p.life / p.ttl, 0.0f, 1.0f);
            Uint8 a = Uint8(255 * t * t);
            SDL_SetRenderDrawColor(renderer, p.color.r, p.color.g, p.color.b, a);
            // SDL_RenderDrawLine(renderer, int(p.lastX), int(p.lastY), int(p.x), int(p.y));
            drawThickLine(int(p.lastX), int(p.lastY), int(p.x), int(p.y), 2, p.color.r, p.color.g, p.color.b, a);

            if (p.life >= p.ttl || p.y > height)
                it = sparks.erase(it);
            else
                ++it;
        }

        // Ondas de choque
        for (auto it = waves.begin(); it != waves.end();)
        {
            it->r += it->dr * dt;
            it->alpha -= dt * 0.8f; // desvanece más lento
            if (it->alpha <= 0.0f)
                it = waves.erase(it);
            else
                ++it;
        }

        SDL_SetRenderTarget(renderer, nullptr);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    }

    void drawFireworksMode()
    {
        for (const auto &r : rockets)
        {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_Rect core{int(r.x - 2), int(r.y - 2), 4, 4};
            SDL_RenderFillRect(renderer, &core);
        }
        for (const auto &p : sparks)
        {
            if (p.life < 0.03f)
            {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 220);
                SDL_RenderDrawPoint(renderer, int(p.x), int(p.y));
            }
        }
        for (const auto &w : waves)
        {
            Uint8 a = Uint8(std::clamp(w.alpha, 0.0f, 1.0f) * 180);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, a);
            for (int deg = 0; deg < 360; deg += 6)
            {
                float rad = deg * PI / 180.0f;
                int px = int(w.x + std::cos(rad) * w.r);
                int py = int(w.y + std::sin(rad) * w.r);
                SDL_RenderDrawPoint(renderer, px, py);
            }
        }
    }

    // ---------- Helpers ----------
    void drawGlowingCircleRaw(int cx, int cy, int radius, Uint8 r, Uint8 g, Uint8 b)
    {
        for (int rr = radius; rr > 0; --rr)
        {
            float t = float(rr) / float(radius);
            Uint8 a = Uint8(255 * t * t);
            SDL_SetRenderDrawColor(renderer, r, g, b, a);
            for (int d = 0; d < 360; d += 4)
            {
                float rad = d * PI / 180.0f;
                int px = int(cx + rr * std::cos(rad));
                int py = int(cy + rr * std::sin(rad));
                SDL_RenderDrawPoint(renderer, px, py);
            }
        }
    }

    // Nuevo: línea gruesa por offsets perpendiculares simples
    void drawThickLine(int x1, int y1, int x2, int y2, int thickness,
                       Uint8 r, Uint8 g, Uint8 b, Uint8 a)
    {
        if (thickness <= 1)
        {
            SDL_SetRenderDrawColor(renderer, r, g, b, a);
            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
            return;
        }
        float dx = float(x2 - x1);
        float dy = float(y2 - y1);
        float len = std::sqrt(dx * dx + dy * dy);
        if (len <= 0.0001f)
            return;
        dx /= len;
        dy /= len;
        // vector perpendicular normalizado
        float px = -dy;
        float py = dx;
        int half = thickness / 2;
        SDL_SetRenderDrawColor(renderer, r, g, b, a);
        for (int o = -half; o <= half; ++o)
        {
            int ox = int(px * o);
            int oy = int(py * o);
            SDL_RenderDrawLine(renderer, x1 + ox, y1 + oy, x2 + ox, y2 + oy);
        }
    }

    void applyBloomEffect()
    {
        SDL_SetRenderTarget(renderer, glowTexture);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        SDL_SetTextureBlendMode(trailTexture, SDL_BLENDMODE_ADD);
        SDL_RenderCopy(renderer, trailTexture, nullptr, nullptr);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 30);
        SDL_RenderFillRect(renderer, nullptr);
        SDL_SetRenderTarget(renderer, nullptr);
        SDL_RenderCopy(renderer, glowTexture, nullptr, nullptr);
    }
};

// Factory function
RendererPtr createRenderer(const RendererConfig &cfg) { return std::make_unique<SDL2UltraRenderer>(cfg); }