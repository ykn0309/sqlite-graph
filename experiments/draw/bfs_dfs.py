import matplotlib.pyplot as plt
import numpy as np

plt.rcParams['font.sans-serif']=['Microsoft YaHei']
plt.rcParams["axes.unicode_minus"]=False 
plt.rcParams["font.size"]=16

# 图中结点的个数
n_values = np.array([5, 10, 50, 100, 500, 1000, 2000, 4000])

# BFS 的图模型和关系模型运行时间
bfs_graph_model = np.array([2.97e-5, 4.89e-5, 2.40e-4, 4.17e-4, 2.58e-3, 5.94e-3, 0.0133, 0.0284])
bfs_relation_model = np.array([4.25e-5, 1.04e-4, 4.65e-4, 1.59e-3, 0.0673, 0.466, 3.00, 13.1])

# DFS 的图模型和关系模型运行时间
dfs_graph_model = np.array([2.68e-5, 4.52e-5, 2.14e-4, 4.14e-4, 2.60e-3, 6.10e-3, 0.0134, 0.0287])
dfs_relation_model = np.array([2.27e-5, 4.53e-5, 3.79e-4, 1.59e-3, 0.0667, 0.467, 2.98, 13.1])

# 绘制 BFS 图
plt.figure(figsize=(7, 5))
plt.grid(visible=True, which='major', linestyle='-')
plt.grid(visible=True, which='minor', linestyle='--', alpha=0.5)
plt.plot(n_values, bfs_graph_model, linestyle='-', marker='o', color='#1f77b4', label='图模型', linewidth=1.5)
plt.plot(n_values, bfs_relation_model, linestyle='-.', marker='s', color='#ff7f0e', label='关系模型', linewidth=1.5)
plt.minorticks_on()
plt.yscale('log')
plt.xlabel('图中结点个数 n')
plt.ylabel('运行时间（秒）')
plt.title('BFS 算法性能对比')
plt.legend()
plt.grid(True)
plt.tight_layout()
# plt.show()
plt.savefig('bfs.png')

# 绘制 DFS 图
plt.figure(figsize=(7, 5))
plt.grid(visible=True, which='major', linestyle='-')
plt.grid(visible=True, which='minor', linestyle='--', alpha=0.5)
plt.plot(n_values, dfs_graph_model, linestyle='-', marker='o', color='#1f77b4', label='图模型', linewidth=1.5)
plt.plot(n_values, dfs_relation_model, linestyle='-.', marker='s', color='#ff7f0e', label='关系模型', linewidth=1.5)
plt.minorticks_on()
plt.yscale('log')
plt.xlabel('图中结点个数 n')
plt.ylabel('运行时间（秒, 对数尺度）')
plt.title('DFS 算法性能对比')
plt.legend()
plt.grid(True)
plt.tight_layout()
# plt.show()
plt.savefig('dfs.png')