import sys
import pandas as pd
import matplotlib.pyplot as plt

if len(sys.argv) < 3:
    print("Uso: python analyze_plots.py summary.csv titulo")
    sys.exit(1)

summary_path = sys.argv[1]
title = sys.argv[2]

df = pd.read_csv(summary_path)

# Normaliza el schedule por estética
df["Schedule"] = df["Schedule"].str.replace("_", ":")

# Plot 1: Speedup vs Threads
plt.figure()
for N in sorted(df["N"].unique()):
    for sched in df["Schedule"].unique():
        subset = df[(df["N"]==N) & (df["Schedule"]==sched)]
        if subset.empty: 
            continue
        plt.plot(subset["Threads"], subset["Speedup"], marker="o",
                 label=f"N={N}, {sched}")
plt.xlabel("Threads")
plt.ylabel("Speedup (Tb/To)")
plt.title(f"Speedup - {title}")
plt.legend()
plt.grid(True)
plt.savefig("data/results/plot_speedup.png", dpi=150)

# Plot 2: Eficiencia vs Threads
plt.figure()
for N in sorted(df["N"].unique()):
    for sched in df["Schedule"].unique():
        subset = df[(df["N"]==N) & (df["Schedule"]==sched)]
        if subset.empty:
            continue
        plt.plot(subset["Threads"], subset["Efficiency"], marker="o",
                 label=f"N={N}, {sched}")
plt.xlabel("Threads")
plt.ylabel("Efficiency (Speedup/Threads)")
plt.title(f"Eficiencia - {title}")
plt.legend()
plt.grid(True)
plt.savefig("data/results/plot_efficiency.png", dpi=150)

print("Gráficas guardadas en data/results/")
