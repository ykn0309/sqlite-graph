import networkx as nx
import pandas as pd
from collections import deque

import networkx as nx
from collections import deque

def is_valid_bfs(G: nx.DiGraph, bfs_order: list, start_node) -> bool:
    if not bfs_order or bfs_order[0] != start_node:
        return False

    visited = set()
    index = {node: i for i, node in enumerate(bfs_order)}

    queue = deque([start_node])
    visited.add(start_node)
    pos = 1  # 当前应访问的 bfs_order 位置

    while queue:
        current = queue.popleft()

        # 只考虑出边邻居
        neighbors = list(G.successors(current))
        unvisited_neighbors = [n for n in neighbors if n not in visited]

        # 保证出队顺序与 bfs_order 一致
        unvisited_neighbors.sort(key=lambda x: index.get(x, float('inf')))

        for n in unvisited_neighbors:
            if pos >= len(bfs_order) or bfs_order[pos] != n:
                return False
            visited.add(n)
            queue.append(n)
            pos += 1

    return pos == len(bfs_order)

facebook = pd.read_csv( # 从文件中读取数据
    "./datasets/facebook_combined.txt.gz",
    compression="gzip",
    sep=" ",
    names=["start_node", "end_node"],
)

G = nx.from_pandas_edgelist(facebook, "start_node", "end_node", create_using=nx.DiGraph())

# bfs_order = list(nx.bfs_tree(G, source=0).nodes())
# print("BFS Traversal Order:", len(bfs_order))

# shortest_path = nx.shortest_path(G, source=0, target=3437)
# print(shortest_path)
# 起点（你程序中开始遍历的节点）
start = 0
my_bfs = []
# 验证
result = is_valid_bfs(G, my_bfs, start)
print("✅ 合法的 BFS 顺序" if result else "❌ 非法的 BFS 顺序")