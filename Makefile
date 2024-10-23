all:
	g++ -g -fPIC -I src/include -shared src/sqlite-graph.cpp -o graph.so