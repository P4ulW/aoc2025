import numpy as np
import matplotlib.pyplot as plt
from rich import print
from glob import glob

data = np.loadtxt("./points.csv", delimiter=";")
print(data)

plt.figure(figsize=(20, 20))
plt.plot([x[0] for x in data], [x[1] for x in data])
for i, point in enumerate(data):
    plt.scatter(
        point[0],
        point[1],
        marker=f"${i}$",
        color="k",
        edgecolor="None",
        s=50,
        zorder=10,
    )
plt.savefig("view.png", dpi=400)
# plt.show()
