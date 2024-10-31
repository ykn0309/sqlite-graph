all:
	g++ -g -fPIC -I src/include -shared src/sqlite-graph2.cpp -o graph.so