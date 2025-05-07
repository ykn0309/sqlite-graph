from flask import Flask, render_template, request
import sqlite3
import pandas as pd
import networkx as nx
import matplotlib.pyplot as plt

app = Flask(__name__)

# 获取所有站点
def get_stations():
    conn = sqlite3.connect('delhi.db')
    stations = conn.execute("SELECT DISTINCT label FROM nodes").fetchall()
    conn.close()
    return [station[0] for station in stations]

# 执行导航计算
def nav(start, end):
    conn = sqlite3.connect('delhi.db')
    conn.enable_load_extension(True)
    conn.load_extension('./graph')
    conn.execute("select createGraph('nodes', 'edges', 'label', 'attribute', 'label', 'attribute', 'from_node', 'to_node');")
    
    try:
        sql = f"select dijkstra('{start}', '{end}', '');"
        result = conn.execute(sql).fetchone()[0]
    except Exception as e:
        print("查询失败：", e)
    finally:
        conn.close()

    if result:
        try:
            path_str, distance_str = result.strip().split('\n')
            path = path_str.split('->')
            distance = float(distance_str.replace("Distance:", "").strip())

            file_path = 'experiments/scenario/delhi_metro.csv'
            delhi_metro_df = pd.read_csv(file_path)
            G = nx.MultiGraph() # 创建一个空的多重图
            lines = delhi_metro_df['Line'].unique() # 提取出不同的线路名称
            line_colors = ['blue', 'blue', 'green', 'green', 'grey', 'magenta', 'orange', 'pink', 'red', 'violet', 'yellow'] # 线路的颜色
            color_map = {line: line_colors[idx % len(line_colors)] for idx, line in enumerate(lines)} # 得到一个{线路：颜色}的字典
            for idx, row in delhi_metro_df.iterrows():
                G.add_node(
                    row['Station'],
                    line = row['Line'],
                    pos = (row['Longitude'], row['Latitude']),
                    order = row['Order'],
                    isJunction = row['Is Junction'],
                    color = color_map[row['Line']]
                )
            for line in lines:
                stations = delhi_metro_df[delhi_metro_df['Line'] == line]
                stations = stations.sort_values(by='Order', ascending=True)
                stations_num = len(stations)
                for i in range(0, stations_num-1):
                    G.add_edge(stations.iloc[i]['Station'], stations.iloc[i+1]['Station'], color = color_map[line])
            for node, degree in dict(G.degree()).items():
                if degree > 2:
                    G.nodes[node]['color'] = 'black' # 将换乘站的颜色设为黑色
            node_colors = ['white' for i in range(len(G))]
            pos = nx.get_node_attributes(G, 'pos')
            for idx, node in enumerate(G.nodes()):
                if node in path:
                    node_colors[idx] = 'green'
            plt.figure(figsize=(15, 15))
            nx.draw(
                G, pos, 
                node_color=node_colors,
                with_labels=True, 
                node_size=100,
                font_size=6
            )
            plt.savefig('experiments/scenario/static/images/delhi.png')
            return path, distance
        except Exception as e:
            print("解析失败：", e)
            return None, None
    else:
        return None, None
        
        

# 首页路由
@app.route('/')
def index():
    stations = get_stations()
    return render_template('index.html', stations=stations, result=None)

# 路由处理表单提交
@app.route('/calculate', methods=['POST'])
def calculate():
    start_station = request.form['start']
    end_station = request.form['end']

    if start_station == end_station:
        return render_template('index.html', stations=get_stations(), result="起点和终点不能相同！")

    path, distance = nav(start_station, end_station)

    if path:
        image_url = '/static/images/delhi.png'
        result = {
            'path': ' -> '.join(path),
            'distance': distance,
            'image_url': image_url
        }
        return render_template('index.html', stations=get_stations(), result=result)
    else:
        return render_template('index.html', stations=get_stations(), result="未找到路径")

if __name__ == '__main__':
    app.run(debug=True)
