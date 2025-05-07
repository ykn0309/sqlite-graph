import pandas as pd
import networkx as nx

facebook = pd.read_csv( # 从文件中读取数据
    "./datasets/facebook_combined.txt.gz",
    compression="gzip",
    sep=" ",
    names=["start_node", "end_node"],
)

G = nx.from_pandas_edgelist(facebook, "start_node", "end_node", create_using=nx.DiGraph())

nodes = G.nodes()
nodes_sqls = []
for node in nodes:
    sql = f"INSERT INTO nodes (label) VALUES ('{node}');\n"
    nodes_sqls.append(sql)

edges=G.edges()
edges_sqls = []
count = 0
for edge in edges:
    sql = f"INSERT INTO edges (from_node, to_node, label) VALUES ('{edge[0]}', '{edge[1]}', '{count}');\n"
    count += 1
    edges_sqls.append(sql)

with open("./datasets/facebook.sql", "w") as f:
    f.write("BEGIN TRANSACTION;\n")
    f.writelines(nodes_sqls)
    f.writelines(edges_sqls)
    f.write("COMMIT;")