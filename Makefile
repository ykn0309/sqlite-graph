all:
	g++ -g -fPIC -shared  -std=c++17 -I src/include src/sqlite-graph.cpp -o graph.so