#include <iostream>
#include <vector>
#include <chrono>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include <cstdio>
#include <algorithm>
#include <thread> 
#include "core/state.hpp"
#include "core/physics.hpp"
#include "omp/update_seq.hpp"
#include "omp/update_omp_for.hpp"
#include "omp/update_omp_simd.hpp"
#include "omp/update_omp_tasks.hpp"
#include "gfx/renderer.hpp"

#ifdef _OPENMP
  #include <omp.h>
#endif

// -------------------- CLI --------------------
struct Args {
    int N = 1000;
    int steps = 1000;        // frames simulados
    int threads = 0;         // 0 = auto (OpenMP decide)
    std::string recordCsv;   // si no vacío, escribe CSV
    std::string schedule = "static"; // static | dynamic[:chunk] | guided[:chunk]
};

static void print_usage(const char* prog) {
    std::cout
      << "Uso: " << prog << " [opciones]\n\n"
      << "Opciones:\n"
      << "  --n INT             Numero de elementos (>=1)\n"
      << "  --steps INT         Frames a simular/medir (>=1)\n"
      << "  --threads INT       Numero de hilos (1..num_procs). 0 = auto\n"
      << "  --schedule STR      static | dynamic:CHUNK | guided:CHUNK\n"
      << "  --record path.csv   Archivo CSV para registrar tiempos por frame\n"
      << "  --help              Muestra esta ayuda\n";
}

static Args parse_args(int argc, char** argv) {
    Args a;
    for (int i = 1; i < argc; ++i) {
        std::string s = argv[i];
        auto next = [&]() -> std::string {
            if (i + 1 < argc) return std::string(argv[++i]);
            throw std::runtime_error("Falta valor para " + s);
        };
        if      (s == "--n")        a.N = std::stoi(next());
        else if (s == "--steps")    a.steps = std::stoi(next());
        else if (s == "--threads")  a.threads = std::stoi(next());
        else if (s == "--record")   a.recordCsv = next();
        else if (s == "--schedule") a.schedule = next();
        else if (s == "--help")     { print_usage(argv[0]); std::exit(0); }
        else {
            std::cerr << "[warn] Opcion desconocida: " << s << "\n";
        }
    }
    if (a.N < 1)    throw std::runtime_error("--n debe ser >= 1");
    if (a.steps < 1)throw std::runtime_error("--steps debe ser >= 1");
    return a;
}

// -------------------- Utilidades defensivas --------------------
static int max_threads_available() {
#ifdef _OPENMP
    // omp_get_num_procs(): numero de procesadores disponibles
    int p = omp_get_num_procs();
    if (p > 0) return p;
#endif
    unsigned hc = std::thread::hardware_concurrency();
    return (hc == 0 ? 1 : int(hc));
}

#ifdef _OPENMP
static void configure_openmp_threads(int user_threads) {
    const int maxp = max_threads_available();
    if (user_threads < 0) {
        std::cerr << "[warn] --threads < 0 no es valido. Ignorando y usando auto.\n";
        return;
    }
    if (user_threads == 0) {
        // auto: deja que OMP decida (respetando OMP_NUM_THREADS si existe)
        return;
    }
    if (user_threads > maxp) {
        std::cerr << "[warn] --threads=" << user_threads
                  << " excede num_procs=" << maxp << ". Ajustando a " << maxp << ".\n";
        omp_set_num_threads(maxp);
        return;
    }
    omp_set_num_threads(user_threads);
}

static void configure_openmp_schedule(const std::string& sch_raw) {
    // Formatos válidos: "static", "dynamic", "guided", con ":chunk" opcional para dynamic/guided
    omp_sched_t kind = omp_sched_static;
    int chunk = 0;

    auto pos = sch_raw.find(':');
    std::string name = (pos == std::string::npos) ? sch_raw : sch_raw.substr(0, pos);

    auto to_lower = [](std::string s){ for(char& c: s) c = char(::tolower(c)); return s; };
    name = to_lower(name);

    if      (name == "static")  { kind = omp_sched_static;  chunk = 0; }
    else if (name == "dynamic") { kind = omp_sched_dynamic; chunk = 0; }
    else if (name == "guided")  { kind = omp_sched_guided;  chunk = 0; }
    else {
        std::cerr << "[warn] --schedule=\"" << sch_raw
                  << "\" no es valido. Usando 'static'.\n";
        omp_set_schedule(omp_sched_static, 0);
        return;
    }

    if (pos != std::string::npos) {
        const std::string tail = sch_raw.substr(pos + 1);
        try {
            int v = std::max(1, std::stoi(tail));
            chunk = v;
        } catch (...) {
            std::cerr << "[warn] chunk invalido en --schedule=" << sch_raw
                      << ". Ignorando chunk.\n";
        }
    }

    omp_set_schedule(kind, chunk);
}
#endif // _OPENMP

// -------------------- main --------------------
int main(int argc, char** argv) {
    try {
        const auto args = parse_args(argc, argv);

#ifdef _OPENMP
        configure_openmp_threads(args.threads);
        configure_openmp_schedule(args.schedule);
#else
        if (args.threads != 0) {
            std::cerr << "[warn] Build sin OpenMP. --threads no tendra efecto.\n";
        }
        if (args.schedule != "static") {
            std::cerr << "[warn] Build sin OpenMP. --schedule no tendra efecto.\n";
        }
#endif

        // Estado inicial
        State s(args.N, 1280, 720, /*seed*/ 42);
        RendererConfig rcfg{1280, 720, /*vsync*/ false};
        RendererPtr renderer = createRenderer(rcfg); 

        // Elegir backend según macro de build
        auto step_fn =
        #if defined(BUILD_MODE_OMP_TASKS)
            update_step_omp_tasks;
        #elif defined(BUILD_MODE_OMP_SIMD)
            update_step_omp_simd;
        #elif defined(BUILD_MODE_OMP_FOR)
            update_step_omp_for;
        #else
            update_step_seq;
        #endif

        // Warm-up
        for (int i = 0; i < 50; ++i) step_fn(s);

        // Medición
        using clock = std::chrono::steady_clock;
        std::vector<double> samples;
        samples.reserve(args.steps);

        for (int i = 0; i < args.steps; ++i) {
            const auto t0 = clock::now();
            step_fn(s);
            const auto t1 = clock::now();
            const double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
            samples.push_back(ms);

            renderer->beginFrame();
            renderer->drawState(s);
            renderer->endFrame();
        }

        // Reporte básico
        double sum = 0.0;
        for (double v : samples) sum += v;
        const double avg = sum / samples.size();
        std::cout << "Frames: " << samples.size() << "  Avg step (ms): " << avg << "\n";

        // Guardar CSV (si falla: informar y continuar sin romper)
        if (!args.recordCsv.empty()) {
            if (FILE* f = std::fopen(args.recordCsv.c_str(), "wb")) {
                std::fputs("frame,ms\n", f);
                for (size_t i = 0; i < samples.size(); ++i) {
                    std::fprintf(f, "%zu,%.6f\n", i, samples[i]);
                }
                std::fclose(f);
                std::cout << "CSV written: " << args.recordCsv << "\n";
            } else {
                std::cerr << "[error] No se pudo escribir CSV en: " << args.recordCsv
                          << " (ruta inexistente o sin permisos). Continuando.\n";
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[error] " << e.what() << "\n";
        return 1;
    }
    return 0;
}
