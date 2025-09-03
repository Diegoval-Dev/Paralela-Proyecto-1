# Anexo 1 - Diagrama de Flujo del Programa

## Descripción Detallada de Todos los Pasos

### 1. CAPTURA DE ARGUMENTOS

**Función responsable**: `parse_args()` en `src/app/main.cpp`

El programa inicia capturando y validando los argumentos de línea de comandos:

#### 1.1 Argumentos Soportados
- `--n INT`: Número de partículas a simular (>=1)
- `--steps INT`: Número de frames a ejecutar (>=1)  
- `--threads INT`: Número de hilos OpenMP (0=automático, 1-max_procs)
- `--schedule STR`: Política de scheduling (`static`, `dynamic:CHUNK`, `guided:CHUNK`)
- `--record path.csv`: Archivo CSV para guardar tiempos de ejecución
- `--help`: Muestra ayuda y termina el programa

#### 1.2 Proceso de Parsing
```cpp
for (int i = 1; i < argc; ++i) {
    std::string s = argv[i];
    if (s == "--n") a.N = std::stoi(next());
    else if (s == "--steps") a.steps = std::stoi(next());
    // ... otros argumentos
}
```

#### 1.3 Programación Defensiva en Argumentos
- **Validación de rangos**: `--n >= 1` y `--steps >= 1`
- **Manejo de excepciones**: `std::stoi()` puede lanzar excepción si el valor no es numérico
- **Valores por defecto**: N=1000, steps=1000, threads=0, schedule="static"
- **Advertencias**: Si se encuentran argumentos desconocidos, se emite warning pero continúa

### 2. SOLICITUD DE INGRESO DE DATOS

**Nota**: Este programa no solicita datos interactivamente, todos los parámetros se pasan por CLI.

#### 2.1 Datos Requeridos vs Opcionales
- **Requeridos**: Ninguno (todos tienen defaults)
- **Opcionales**: Todos los parámetros pueden ser personalizados
- **Modo de operación**: Completamente batch/no-interactivo

#### 2.2 Configuración Automática
Si no se especifican parámetros, el programa usa:
- N = 1000 partículas
- steps = 1000 frames  
- threads = 0 (OpenMP decide automáticamente)
- schedule = "static"
- Sin grabación de CSV

### 3. PROGRAMACIÓN DEFENSIVA

#### 3.1 Validación de Entrada (`parse_args()`)
```cpp
if (a.N < 1) throw std::runtime_error("--n debe ser >= 1");
if (a.steps < 1) throw std::runtime_error("--steps debe ser >= 1");
```

#### 3.2 Configuración Segura de OpenMP (`configure_openmp_threads()`)
```cpp
if (user_threads < 0) {
    std::cerr << "[warn] --threads < 0 no es valido. Ignorando.\n";
    return; // No aplica cambios inválidos
}
if (user_threads > maxp) {
    std::cerr << "[warn] Ajustando threads de " << user_threads 
              << " a " << maxp << ".\n";
    omp_set_num_threads(maxp); // Ajusta a máximo disponible
}
```

#### 3.3 Manejo de Errores en I/O
```cpp
if (FILE* f = std::fopen(args.recordCsv.c_str(), "wb")) {
    // Escribir CSV exitosamente
} else {
    std::cerr << "[error] No se pudo escribir CSV: " << args.recordCsv
              << " (ruta inexistente o sin permisos). Continuando.\n";
    // Programa continúa sin fallar
}
```

#### 3.4 Protección contra Builds sin OpenMP
```cpp
#ifdef _OPENMP
    configure_openmp_threads(args.threads);
#else
    if (args.threads != 0) {
        std::cerr << "[warn] Build sin OpenMP. --threads no tendra efecto.\n";
    }
#endif
```

### 4. SECCIONES PARALELAS

#### 4.1 Selección de Backend Paralelo (Tiempo de Compilación)
```cpp
auto step_fn = 
#if defined(BUILD_MODE_OMP_TASKS)
    update_step_omp_tasks;
#elif defined(BUILD_MODE_OMP_SIMD) 
    update_step_omp_simd;
#elif defined(BUILD_MODE_OMP_FOR)
    update_step_omp_for;
#else
    update_step_seq;  // Versión secuencial
#endif
```

#### 4.2 Versión OpenMP Parallel For (`update_omp_for.cpp`)
**Sección paralela 1 - Integración**:
```cpp
#pragma omp parallel for if(s.N>256) schedule(runtime)
for (int i=0; i<s.N; i++) {
    s.x[i] += s.vx[i]*dt;
    s.y[i] += s.vy[i]*dt;
}
```

**Sección paralela 2 - Rebotes**:
```cpp
#pragma omp parallel for if(s.N>256) schedule(runtime)
for (int i=0; i<s.N; i++) {
    // Detección y corrección de colisiones con bordes
    if (s.x[i] < 0.f) { s.x[i]=0.f; s.vx[i] = -s.vx[i]; }
    // ... más condiciones de borde
}
```

#### 4.3 Versión OpenMP SIMD (`update_omp_simd.cpp`)
**Región paralela combinada**:
```cpp
#pragma omp parallel
{
    #pragma omp for schedule(runtime)
    for (int i=0; i<s.N; i++) {
        #pragma omp simd  // Vectorización forzada
        for (int k=0; k<1; k++) {
            s.x[i] += s.vx[i]*dt;
            s.y[i] += s.vy[i]*dt;
        }
    }
    
    #pragma omp for schedule(runtime)  
    for (int i=0; i<s.N; i++) {
        // Rebotes (sin SIMD por las condicionales)
    }
}
```

#### 4.4 Versión OpenMP Tasks (`update_omp_tasks.cpp`)
**Fase 1 - Integración y rebotes paralelos**:
```cpp
#pragma omp parallel
{
    #pragma omp for schedule(guided)
    for (int i=0; i<s.N; i++) { /* integración */ }
    
    #pragma omp for schedule(guided) 
    for (int i=0; i<s.N; i++) { /* rebotes */ }
}
```

**Fase 2 - Tareas por celdas espaciales**:
```cpp
#pragma omp parallel
{
    #pragma omp single nowait  // Un solo hilo genera las tareas
    for (int cy=0; cy<g.rows; ++cy) {
        for (int cx=0; cx<g.cols; ++cx) {
            #pragma omp task firstprivate(cx,cy) shared(g,s)
            {
                // Procesar colisiones en celda (cx,cy)
                // TODO: Implementación pendiente
            }
        }
    }
    // Barrera implícita al salir de la región paralela
}
```

### 5. MECANISMOS DE SINCRONÍA

#### 5.1 Barreras Implícitas de OpenMP
- **Final de `#pragma omp parallel for`**: Todos los hilos esperan a que termine el bucle antes de continuar
- **Final de `#pragma omp parallel`**: Barrera implícita antes de continuar con código secuencial
- **`#pragma omp single`**: Solo un hilo ejecuta, otros esperan automáticamente

#### 5.2 Sincronización Explícita en Tasks
```cpp
#pragma omp parallel
{
    #pragma omp single nowait  // nowait evita barrera innecesaria
    {
        // Generar todas las tareas
        for (...) {
            #pragma omp task { /* trabajo */ }
        }
    }
    // Barrera implícita aquí: todos esperan que terminen todas las tareas
}
```

#### 5.3 Políticas de Scheduling Runtime
- **static**: Chunks de tamaño fijo distribuidos al inicio
- **dynamic:N**: Chunks de tamaño N asignados dinámicamente
- **guided:N**: Chunks que decrecen exponencialmente, mínimo N
- **Configuración**: `omp_set_schedule()` configurado por `--schedule`

#### 5.4 Control de Condiciones de Carrera
- **Memoria compartida**: Arrays `x, y, vx, vy` del `State`
- **Acceso seguro**: Cada hilo trabaja en partición disjunta de índices
- **Sin variables compartidas modificadas**: No hay necesidad de locks/críticas

### 6. DESPLIEGUE DE RESULTADOS

#### 6.1 Medición de Performance
**Warm-up** (evitar efectos de cold start):
```cpp
for (int i = 0; i < 50; ++i) step_fn(s);  // 50 iteraciones de calentamiento
```

**Medición principal**:
```cpp
std::vector<double> samples;
for (int i = 0; i < args.steps; ++i) {
    const auto t0 = std::chrono::steady_clock::now();
    step_fn(s);  // SOLO medir la actualización, no el render
    const auto t1 = std::chrono::steady_clock::now(); 
    const double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    samples.push_back(ms);
    
    // Render (fuera de la medición)
    renderer->beginFrame();
    renderer->drawState(s);
    renderer->endFrame();
}
```

#### 6.2 Cálculo de Estadísticas
```cpp
double sum = 0.0;
for (double v : samples) sum += v;
const double avg = sum / samples.size();
std::cout << "Frames: " << samples.size() << "  Avg step (ms): " << avg << "\n";
```

#### 6.3 Exportación de Datos (Opcional)
**Formato CSV**:
```cpp
if (!args.recordCsv.empty()) {
    FILE* f = std::fopen(args.recordCsv.c_str(), "wb");
    std::fputs("frame,ms\n", f);  // Header
    for (size_t i = 0; i < samples.size(); ++i) {
        std::fprintf(f, "%zu,%.6f\n", i, samples[i]);  // frame,tiempo
    }
    std::fclose(f);
}
```

#### 6.4 Manejo de Errores en Output
- **Archivo no escribible**: Warning pero programa continúa
- **Ruta inexistente**: Error informativo sin crash
- **Sin permisos**: Mensaje claro al usuario

---

## Flujo de Control Resumido

```
INICIO
  ↓
[Captura y validación de argumentos CLI]
  ↓
[Configuración defensiva de OpenMP]  
  ↓
[Inicialización de Estado y Renderer]
  ↓
[Selección de función de actualización según BUILD_MODE]
  ↓
[Warm-up: 50 iteraciones sin medir]
  ↓
[Bucle principal: args.steps iteraciones]
  ├─ Cronómetro INICIO
  ├─ step_fn(s) ← AQUÍ OCURRE LA PARALELIZACIÓN
  │   ├─ Integración (paralela si aplica)
  │   ├─ Rebotes (paralela si aplica)  
  │   └─ Grid rebuild (secuencial)
  ├─ Cronómetro FIN
  ├─ Render frame (no medido)
  └─ Repetir
  ↓
[Cálculo de estadísticas de tiempo]
  ↓
[Exportar CSV si se solicitó]
  ↓
[Mostrar resultados en consola]
  ↓
FIN
```