#include <iostream>
#include <vector>
#include <chrono>
#include <string>
#include <cstdlib>
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

// Args simples: --n, --steps, --threads, --record
struct Args {
    int N = 1000;
    int steps = 1000;        // frames simulados
    int threads = 0;         // 0 = default
    std::string recordCsv;   // si no vacío, escribe CSV
    std::string schedule = "static";
};

Args parse_args(int argc, char** argv) {
    Args a;
    for (int i=1;i<argc;i++) {
        std::string s = argv[i];
        auto next = [&](){ if (i+1<argc) return std::string(argv[++i]); throw std::runtime_error("Missing value for "+s); };
        if (s=="--n") a.N = std::stoi(next());
        else if (s=="--steps") a.steps = std::stoi(next());
        else if (s=="--threads") a.threads = std::stoi(next());
        else if (s=="--record") a.recordCsv = next();
        else if (s=="--schedule") a.schedule = next();
        else if (s=="--help") {
            std::cout << "Usage: --n INT --steps INT --threads INT --record path.csv\n";
            std::exit(0);
        }
    }
    if (a.N < 1) throw std::runtime_error("N must be >= 1");
    if (a.steps < 1) throw std::runtime_error("steps must be >= 1");
    return a;
}

int main(int argc, char** argv) {
    try {
        auto args = parse_args(argc, argv);

        #ifdef _OPENMP
        if (args.threads > 0) {
            omp_set_num_threads(args.threads);
        }
        #endif

        // Estado inicial
        State s(args.N, 1280, 720, /*seed*/ 42);
        RendererConfig rcfg{1280, 720, /*vsync*/ false};
        RendererPtr renderer = createRenderer(rcfg); // dummy por defecto; SDL2/SFML si activas

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
        for (int i=0;i<50;i++) step_fn(s);

        // Medición
        using clock = std::chrono::steady_clock;
        std::vector<double> samples;
        samples.reserve(args.steps);
        for (int i=0;i<args.steps;i++) {
            auto t0 = clock::now();
            step_fn(s);
            auto t1 = clock::now();
            double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
            samples.push_back(ms);

            renderer->beginFrame();
            renderer->drawState(s);
            renderer->endFrame();
        }

        // Reporte básico
        double sum=0; for (double v: samples) sum+=v;
        double avg = sum / samples.size();
        std::cout << "Frames: " << samples.size() << "  Avg step (ms): " << avg << "\n";

        if (!args.recordCsv.empty()) {
            FILE* f = fopen(args.recordCsv.c_str(), "wb");
            if (f) {
                fputs("frame,ms\n", f);
                for (size_t i=0;i<samples.size();++i) {
                    fprintf(f, "%zu,%.6f\n", i, samples[i]);
                }
                fclose(f);
                std::cout << "CSV written: " << args.recordCsv << "\n";
            } else {
                std::cerr << "Unable to write CSV: " << args.recordCsv << "\n";
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[error] " << e.what() << "\n";
        return 1;
    }
    return 0;
}
