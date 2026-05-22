#!/usr/bin/env python3
import argparse
import numpy as np
import matplotlib.pyplot as plt
import os

parser = argparse.ArgumentParser(description="Plot .npy files")
parser.add_argument("files", nargs="+", help="Y-axis .npy files")
parser.add_argument("--xaxis", metavar="FILE", help="X-axis .npy file")
args = parser.parse_args()

y_arrays = [np.load(f) for f in args.files]

if args.xaxis:
    x_array = np.load(args.xaxis)
    n = min(len(x_array), min(len(a) for a in y_arrays))
    x = x_array[:n]
else:
    n = min(len(a) for a in y_arrays)
    x = np.arange(n)

fig, ax = plt.subplots()
for f, a in zip(args.files, y_arrays):
    ax.plot(x, a[:n], label=os.path.splitext(os.path.basename(f))[0])

ax.set_xlabel(os.path.splitext(os.path.basename(args.xaxis))[0] if args.xaxis else "index")
ax.legend()
ax.grid(True)
plt.tight_layout()
plt.show()
