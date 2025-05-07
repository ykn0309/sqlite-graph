import matplotlib.pyplot as plt
import numpy as np

plt.rcParams['font.sans-serif']=['Microsoft YaHei']
plt.rcParams["axes.unicode_minus"]=False 
plt.rcParams["font.size"]=16

# 图中结点的个数
n_values = np.array([5, 10, 50, 100, 500, 1000, 2000, 4000])

dijkstra_graph_model = np.array([8.45e-5, 7.67e-5, 2.34e-4, 2.55e-4, 1.43e-3, 4.93e-3, 0.0126, 0.0228])
dijkstra_relation_model = np.array([6.76e-5, 7.42e-4, 4.66e-4, 1.83e-3, 0.0536, 0.179, 0.559, 14.1])

plt.figure(figsize=(7, 5))
plt.grid(visible=True, which='major', linestyle='-')
plt.grid(visible=True, which='minor', linestyle='--', alpha=0.5)
plt.plot(n_values, dijkstra_graph_model, linestyle='-', marker='o', color='#1f77b4', label='图模型', linewidth=1.5)
plt.plot(n_values, dijkstra_relation_model, linestyle='-.', marker='s', color='#ff7f0e', label='关系模型', linewidth=1.5)
plt.minorticks_on()
plt.yscale('log')
plt.xlabel('图中结点个数 n')
plt.ylabel('运行时间（秒）')
plt.title('Dijkstra 算法性能对比')
plt.legend()
plt.grid(True)
plt.tight_layout()
# plt.show()
plt.savefig('dijkstra.png')