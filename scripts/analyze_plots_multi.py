import sys
import pandas as pd
import matplotlib.pyplot as plt

# Uso: python analyze_plots_multi.py <summary.csv> <prefix> [title]
# Ej:  python analyze_plots_multi.py data/results/summary_simd.csv omp_simd "SIMD schedules"

if len(sys.argv) < 3:
    print("Uso: python analyze_plots_multi.py <summary.csv> <prefix> [title]")
    sys.exit(1)

summary_path = sys.argv[1]
prefix = sys.argv[2]
title = sys.argv[3] if len(sys.argv) > 3 else prefix

df = pd.read_csv(summary_path)

# Normaliza el Schedule para visualización
if "Schedule" in df.columns:
    df["Schedule"] = df["Schedule"].astype(str).str.replace("_", ":")

# ---- Speedup vs Threads ----
plt.figure()
for N in sorted(df["N"].unique()):
    for sched in sorted(df["Schedule"].unique()):
        sub = df[(df["N"] == N) & (df["Schedule"] == sched)].sort_values("Threads")
        if sub.empty: 
            continue
        plt.plot(sub["Threads"], sub["Speedup"], marker="o", label=f"N={N}, {sched}")
# Línea ideal (y = x) solo si Threads está presente
if "Threads" in df.columns:
    xs = sorted(df["Threads"].unique())
    if len(xs) > 0:
        plt.plot(xs, xs, linestyle="--", label="Ideal")

plt.xlabel("Threads")
plt.ylabel("Speedup (Tb/To)")
plt.title(f"Speedup - {title}")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig(f"data/results/{prefix}_speedup.png", dpi=150)

# ---- Eficiencia vs Threads ----
plt.figure()
for N in sorted(df["N"].unique()):
    for sched in sorted(df["Schedule"].unique()):
        sub = df[(df["N"] == N) & (df["Schedule"] == sched)].sort_values("Threads")
        if sub.empty:
            continue
        plt.plot(sub["Threads"], sub["Efficiency"], marker="o", label=f"N={N}, {sched}")

plt.xlabel("Threads")
plt.ylabel("Efficiency (Speedup/Threads)")
plt.title(f"Eficiencia - {title}")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig(f"data/results/{prefix}_efficiency.png", dpi=150)

print(f"OK: data/results/{prefix}_speedup.png")
print(f"OK: data/results/{prefix}_efficiency.png")
