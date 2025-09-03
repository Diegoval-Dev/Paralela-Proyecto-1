# Anexo 2 – Catálogo de Funciones (basado en src/ del proyecto)

> Este anexo lista **estructuras, interfaces y funciones públicas** encontradas en `src/` del proyecto tal como están en el ZIP. Se indican archivo, entradas, salidas y propósito.

---

## CORE

### `struct RendererConfig`  *(src/core/types.hpp)*
- **Campos**: `int width`, `int height`, `bool vsync=false`
- **Propósito**: Configurar el renderizador (tamaño de ventana y vsync).

### `struct RNG`  *(src/core/rng.hpp)*
- **Campos**: `uint32_t s`
- **Constructores**: `explicit RNG(uint32_t seed=1u)`
- **Métodos**:
  - `uint32_t u32()` → entero aleatorio 32-bit (xorshift32).
  - `float uniform(float a, float b)` → flotante uniforme en `[a,b)`.
- **Propósito**: Generador de números aleatorios ligero para inicialización.

### `struct State`  *(src/core/state.hpp)*
- **Campos**:
  - `int N; int width, height;`
  - `std::vector<float> x, y, vx, vy;`
  - `std::vector<uint32_t> color;`
- **Constructor**: `explicit State(int n, int w, int h, uint32_t seed=1234)` → reserva y **inicializa** posiciones/velocidades/colores con `RNG`.
- **Propósito**: Contenedor SoA (Structure of Arrays) del sistema de partículas.

### `void integrate(State& s, float dt)`  *(src/core/physics.hpp, .cpp)*
- **Entradas**: `s` (estado), `dt` (paso de tiempo).
- **Salidas**: Actualiza `s.x[i]` y `s.y[i]` sumando `vx*dt`, `vy*dt`.
- **Propósito**: Integración explícita de posiciones (secuencial).

### `void bounce(State& s)`  *(src/core/physics.hpp, .cpp)*
- **Entradas**: `s` (estado).
- **Salidas**: Ajusta `x,y` al rango `[0,width]x[0,height]` e invierte `vx,vy` en colisiones con bordes.
- **Propósito**: Condiciones de frontera (rebotes).

### `struct Grid`  *(src/core/grid.hpp, .cpp)*
- **Campos**: `int cols, rows; float cellW, cellH; std::vector<int> head, next;`
- **Constructor**: `explicit Grid(int width, int height, int wantedCells=64)` → calcula `cols=rows=wantedCells`, tamaños de celda.
- **Métodos**:
  - `void build(const State& s)` → llena listas por celda (`head/next`) insertando cada partícula según `(x,y)`.
- **Propósito**: Estructura espacial para particionar partículas por celdas.

---

## OMP (actualización de estado)

### `void update_step_seq(State& s)`  *(src/omp/update_seq.hpp, .cpp)*
- **Entradas**: `s`.
- **Salidas**: Modifica `s` in-place.
- **Propósito**: Pipeline **secuencial**: `integrate(s, 1/60)`, `bounce(s)`, `Grid g(...); g.build(s)`.

### `void update_step_omp_for(State& s)`  *(src/omp/update_omp_for.hpp, .cpp)*
- **Entradas**: `s`.
- **Directivas**: `#pragma omp parallel for if(s.N>256) schedule(runtime)` en **dos bucles** (integración y rebotes).
- **Salidas**: Modifica `s`; reconstruye `Grid`.
- **Propósito**: Versión paralela con OpenMP `for` y `schedule(runtime)`.

### `void update_step_omp_simd(State& s)`  *(src/omp/update_omp_simd.hpp, .cpp)*
- **Entradas**: `s`.
- **Directivas**: región `#pragma omp parallel` con:
  - `#pragma omp for schedule(runtime)` y **`#pragma omp simd`** dentro para integración.
  - `#pragma omp for schedule(runtime)` para rebotes.
- **Salidas**: Modifica `s`; reconstruye `Grid`.
- **Propósito**: Paralelismo + **vectorización** (SIMD) sobre bucles calientes.

### `void update_step_omp_tasks(State& s)`  *(src/omp/update_omp_tasks.hpp, .cpp)*
- **Entradas**: `s`.
- **Directivas**:
  - Región `#pragma omp parallel` + `#pragma omp for schedule(guided)` para integración+rebotes.
  - Luego `Grid g(...); g.build(s);`
  - Bucle de celdas con `#pragma omp single nowait` y **`#pragma omp task firstprivate(cx,cy) shared(g,s)`**.
- **Salidas**: Modifica `s`.
- **Estado**: **En progreso**: dentro de cada `task` hay `// TODO` para colisiones en la celda.
- **Propósito**: Esqueleto de versión basada en **tareas por celda**.

---

## GFX (renderizado)

### `struct IRenderer`  *(src/gfx/renderer.hpp)*
- **Métodos virtuales puros**:
  - `void beginFrame()`
  - `void drawState(const State& s)`
  - `void endFrame()`
- **Typedef**: `using RendererPtr = std::unique_ptr<IRenderer>;`
- **Propósito**: Interfaz común para renderers.

### `RendererPtr createRenderer(const RendererConfig&)`  *(src/gfx/renderer.hpp, .cpp)*
- **Implementación (dummy)**: *(src/gfx/renderer_dummy.cpp)* crea `DummyRenderer` que no dibuja.
- **Implementaciones SDL2**: *(src/gfx/renderer_sdl2.cpp, renderer_sdl2_ultra.cpp, renderer_sdl2_backup.cpp)* clases derivadas que abren ventana y dibujan partículas/efectos.
- **Propósito**: Fábrica que retorna un renderer según el build/archivos presentes.

---

## APP (ejecutable)

### `static void print_usage(const char* prog)`  *(src/app/main.cpp)*
- **Entradas**: `prog`.
- **Salidas**: Imprime ayuda a `stdout` con flags soportados.
- **Propósito**: Mensaje de uso.

### `struct Args`  *(src/app/main.cpp)*
- **Campos**: `int N=10000; int steps=1000; int threads=0; std::string recordCsv; std::string schedule="static";`
- **Propósito**: Configuración de ejecución.

### `static Args parse_args(int argc, char** argv)`  *(src/app/main.cpp)*
- **Flags soportados**: `--n`, `--steps`, `--threads`, `--record`, `--schedule`, `--help`.
- **Validaciones**: `--n >= 1`, `--steps >= 1` (lanza `std::runtime_error` si inválido).
- **Salida**: `Args` poblado.
- **Propósito**: Lectura y validación de argumentos CLI.

### `static int max_threads_available()`  *(src/app/main.cpp)*
- **Salida**: número de procesadores/hilos disponibles (prefiere `omp_get_num_procs()`; fallback `std::thread::hardware_concurrency()`).
- **Propósito**: Descubrir concurrencia máxima.

### `static void configure_openmp_threads(int user_threads)`  *(src/app/main.cpp, solo si `_OPENMP`)*
- **Entradas**: `user_threads` (0=auto).
- **Efecto**: Llama a `omp_set_num_threads(...)` si procede; valida rangos y advierte si inválido.
- **Propósito**: Configurar hilos OpenMP desde CLI.

### `static void configure_openmp_schedule(const std::string& sch_raw)`  *(src/app/main.cpp, solo si `_OPENMP`)*
- **Entradas**: cadena en forma `static | dynamic[:chunk] | guided[:chunk]` (case-insensitive).
- **Efecto**: Parsea y llama `omp_set_schedule(kind, chunk)`; advierte si valor inválido.
- **Propósito**: Configurar `schedule(runtime)`.

### `int main(int argc, char** argv)`  *(src/app/main.cpp)*
- **Flujo**:
  1) `parse_args` → valida y lee flags.
  2) Si `_OPENMP`, aplica `configure_openmp_threads` y `configure_openmp_schedule`.
  3) Crea `State s(N, 1280, 720, seed=42)` y `RendererConfig rcfg{1280,720,false}`; obtiene `RendererPtr renderer = createRenderer(rcfg)`.
  4) Selecciona backend **en tiempo de compilación** con macros: `BUILD_MODE_OMP_TASKS` / `BUILD_MODE_OMP_SIMD` / `BUILD_MODE_OMP_FOR` / (default) `seq`.
  5) Warm-up: 50 iteraciones de `step_fn(s)`.
  6) Medición: bucle de `steps` usando `std::chrono::steady_clock`; **mide solo la actualización** (`step_fn(s)`), no el render.
  7) Render: llama a `renderer->beginFrame(); drawState(s); endFrame();` en cada paso.
  8) Si `--record` no vacío, escribe CSV con encabezado `frame,ms` y una fila por iteración.
- **Salida**: `0` si éxito; `1` si excepción.
- **Propósito**: Orquestación de ejecución, medición y salida.
