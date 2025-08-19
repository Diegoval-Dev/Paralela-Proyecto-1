import sys, pandas as pd
import matplotlib.pyplot as plt

if len(sys.argv) < 3:
    print("Usage: analyze.py base.csv other.csv [title]")
    sys.exit(1)

# Cargar los datos
base = pd.read_csv(sys.argv[1])  # frame,ms
other = pd.read_csv(sys.argv[2])

# Calcular estadísticas
Tb = base['ms'].mean()
To = other['ms'].mean()
speedup = Tb / To

# Mostrar resultados
print(f"Tb(avg ms)={Tb:.3f}  To(avg ms)={To:.3f}  Speedup={speedup:.3f}")
