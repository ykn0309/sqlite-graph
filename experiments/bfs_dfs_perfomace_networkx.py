import sqlite3
import networkx as nx
import time

# 连接数据库
def connect_to_db(db_path):
    conn = sqlite3.connect(db_path)
    return conn

# 从数据库中导入图
def load_graph_from_db(db_path):
    conn = connect_to_db(db_path)
    cursor = conn.cursor()
    
    # 创建一个有向图对象
    G = nx.DiGraph()

    # 获取所有结点并添加到图中
    cursor.execute("SELECT id, label FROM nodes")
    nodes = cursor.fetchall()
    for node in nodes:
        G.add_node(node[1], id=node[0])

    # 获取所有边并添加到图中
    cursor.execute("SELECT from_node, to_node FROM edges")
    edges = cursor.fetchall()
    for edge in edges:
        G.add_edge(edge[0], edge[1])

    conn.close()
    return G

# 执行BFS
def bfs(G, start_node):
    return list(nx.bfs_edges(G, start_node))

# 执行DFS
def dfs(G, start_node):
    return list(nx.dfs_edges(G, start_node))

# 测试BFS和DFS的时间
def test_algorithm(G, start_node, round_num=5):
    bfs_times = []
    dfs_times = []

    for _ in range(round_num):
        start = time.time()
        bfs(G, start_node)
        bfs_times.append(time.time() - start)

        start = time.time()
        dfs(G, start_node)
        dfs_times.append(time.time() - start)

    avg_bfs_time = sum(bfs_times) / round_num
    avg_dfs_time = sum(dfs_times) / round_num

    return avg_bfs_time, avg_dfs_time

# 主函数
def main():
    db_paths = [
        'facebook_5.db', 
        'facebook_10.db', 
        'facebook_50.db', 
        'facebook_100.db', 
        'facebook_500.db',
        'facebook_1000.db',
        'facebook_2000.db',
        'facebook_4000.db'
    ]
    start_node = '0'  # 假设从 label 为 "0" 的结点开始

    for db_path in db_paths:
        print(f"Testing on {db_path}")
        # 导入图
        G = load_graph_from_db(db_path)

        # 执行BFS和DFS并获取平均时间
        avg_bfs_time, avg_dfs_time = test_algorithm(G, start_node)

        # 输出平均时间
        print(f"Average BFS time for {db_path}: {avg_bfs_time} seconds")
        print(f"Average DFS time for {db_path}: {avg_dfs_time} seconds")
        print("-" * 50)

if __name__ == "__main__":
    main()
