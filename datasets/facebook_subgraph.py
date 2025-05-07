import networkx as nx
from collections import deque
import pandas as pd

def bfs_sample_subgraph(G, size):
    """
    从图 G 中使用 BFS 采样出 size 个节点组成的子图
    """
    if size > G.number_of_nodes():
        raise ValueError("采样大小超过图的节点数")

    start_node = next(iter(G.nodes))  # 任取一个起点
    visited = set()
    queue = deque([start_node])

    while queue and len(visited) < size:
        node = queue.popleft()
        if node in visited:
            continue
        visited.add(node)
        neighbors = list(G.successors(node)) + list(G.predecessors(node))
        for neighbor in neighbors:
            if neighbor not in visited and len(visited) < size:
                queue.append(neighbor)

    return G.subgraph(visited).copy()

def export_subgraph_sql(G, size, output_path):
    if G.number_of_nodes() < size:
        print(f"Graph has less than {size} nodes, skipping...")
        return

    subgraph = bfs_sample_subgraph(G, size)

    # 生成 SQL 插入语句
    nodes_sqls = []
    for node in subgraph.nodes:
        sql = f"INSERT INTO nodes (label) VALUES ('{node}');\n"
        nodes_sqls.append(sql)

    edges_sqls = []
    for i, edge in enumerate(subgraph.edges):
        sql = f"INSERT INTO edges (from_node, to_node, label) VALUES ('{edge[0]}', '{edge[1]}', '{i}');\n"
        edges_sqls.append(sql)

    # 写入 SQL 文件
    with open(output_path, "w") as f:
        f.write("BEGIN TRANSACTION;\n")
        f.writelines(nodes_sqls)
        f.writelines(edges_sqls)
        f.write("COMMIT;")

    print(f"✅ Exported subgraph with {size} nodes to {output_path}")


# 主流程
facebook = pd.read_csv( # 从文件中读取数据
    "./datasets/facebook_combined.txt.gz",
    compression="gzip",
    sep=" ",
    names=["start_node", "end_node"],
)

G = nx.from_pandas_edgelist(facebook, "start_node", "end_node", create_using=nx.DiGraph())

subgraph_sizes = [5, 10, 50, 100, 500, 1000, 2000, 4000]

for size in subgraph_sizes:
    export_subgraph_sql(G, size, f"./datasets/facebook_subgraph_{size}.sql")
