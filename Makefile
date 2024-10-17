all:
	gcc -g -fPIC -I src/include -shared src/sqlite-graph.c -o graph.so