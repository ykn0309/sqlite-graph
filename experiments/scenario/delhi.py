import sqlite3

def parse_result(result):
    lines = result.strip().splitlines()
    
    # 提取路径
    path_str = lines[0]
    path = path_str.split("->")
    
    # 提取距离
    distance_line = lines[1]
    distance = float(distance_line.split(":")[1].strip())
    
    return path, distance


def nav(start, end):
    conn = sqlite3.connect('delhi.db')
    conn.enable_load_extension(True)
    conn.load_extension('./graph')
    conn.execute("select createGraph('nodes', 'edges', 'label', 'attribute', 'label', 'attribute', 'from_node', 'to_node');")
    sql = f"select dijkstra('{start}', '{end}', '');"
    result = conn.execute(sql).fetchone()[0]
    path, distance = parse_result(result)
    return path, distance
    
nav('Yamuna Bank', 'MG Road')