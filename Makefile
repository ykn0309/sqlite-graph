all:
	g++ -g -fPIC -shared -I src/include src/sqlite-graph.cpp -o graph.so