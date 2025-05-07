all:
	g++ -g -fPIC -shared -std=c++17 -I src/include src/sqlite-graph.cpp -o graph.so

bfs_dfs_performance: experiments/bfs_dfs_performance.cpp
	g++ -g experiments/bfs_dfs_performance.cpp  -I src/include -o bfs_dfs_performance -lsqlite3

bfs_dfs_performance_relation: experiments/bfs_dfs_performance_relation.cpp
	g++ -g experiments/bfs_dfs_performance_relation.cpp  -I src/include -o bfs_dfs_performance_relation -lsqlite3

dijkstra_performance_relation: experiments/dijkstra_performance_relation.cpp
	g++ -g experiments/dijkstra_performance_relation.cpp  -I src/include -o dijkstra_performance_relation -lsqlite3

dijkstra_performance: experiments/dijkstra_performance.cpp
	g++ -g experiments/dijkstra_performance.cpp  -I src/include -o dijkstra_performance -lsqlite3

query_sql: experiments/query_sql.cpp
	g++ -g experiments/query_sql.cpp  -I src/include -o query_sql -lsqlite3

query_vtab: experiments/query_vtab.cpp
	g++ -g experiments/query_vtab.cpp  -I src/include -o query_vtab -lsqlite3

query_relation: experiments/query_relation.cpp
	g++ -g experiments/query_relation.cpp  -I src/include -o query_relation -lsqlite3