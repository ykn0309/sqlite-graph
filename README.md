# sqlite-graph
A graph extension for SQLite.

SQLite is a relational database and does not natively support graph model data.

So, I developed a graph extension for SQLite using its [run-time loadable extension mechanism](https://sqlite.org/loadext.html).

## Features
* Create in-memory graph from nodes and edges table.
* Support some basic graph algorithms: BFS, DFS, Dijkstra.
* Support a cypher-like graph query language for complex graph query.

## How it works?
sqlite-graph uses two tables, ``nodes`` and ``edges``, to store the nodes and edges of a graph in disk. When sqlite-graph is loaded,  `createGraph()` function scans the `nodes` and `edges` tables and then creates the graph data structure in memory. Subsequent graph operations will be performed on the in-memory graph data structure.

## Quick start

Compile dynamic library of sqlite-graph.
> It works well on my WSL2 and Macbook. You can modify the Makefile according to your own machine.

```bash
make
```

Run SQLite and load sqlite-graph extension.

```bash
sqlite3 test.db # It depends on your own SQLite3 path and database file.
.load ./graph # Maybe ".load ./graph.so" or ".load ./graph.dylib" in other machines
```

Create ``nodes`` and ``edge`` table.
> The names of ``node`` and ``edge`` table can be customized. And the column names can also be defined freely.

```sql
create table nodes(id INTEGER PRIMARY KEY AUTOINCREMENT, label TEXT UNIQUE, attribute TEXT);

create table edges(id INTEGER PRIMARY KEY AUTOINCREMENT, from_node TEXT, to_node TEXT, label TEXT UNIQUE, attribute TEXT);
```

Insert nodes and edges data.
```sql
insert into nodes (label, attribute) values
    ('1', '{"color": "red"}'),
    ('2', '{"color": "blue"}'),
    ('3', '{"color": "green"}'),
    ('4', '{"color": "yellow"}'),
    ('5', '{"color": "orange"}');

insert into edges (from_node, to_node, label, attribute) values
    ('1', '2', 'A', '{"weight": 1}'),
    ('1', '3', 'B', '{"weight": 1}'),
    ('2', '4', 'C', '{"weight": 1}'),
    ('3', '4', 'D', '{"weight": 1}'),
    ('1', '5', 'E', '{"weight": 1}'),
    ('2', '5', 'F', '{"weight": 1}'),
    ('3', '5', 'G', '{"weight": 1}'),
    ('4', '5', 'H', '{"weight": 1}');
```

Create graph data stucture in memory.

```sql
select createGraph('nodes', 'edges', 'label', 'attribute', 'label', 'attribute', 'from_node', 'to_node');
```
> * ``nodes``: table name of nodes table
> * ``edges``: table name of edges table
> * ``lable``: column name of label column in nodes table
> * ``attribute``: column name of attribute column in nodes table
> * ``lable``: column name of label column in edges table
> * ``attribute``: column name of attribute column in edges table
> * ``from_node``: column name of from_node column in edges table
> * ``to_node``: column name of to_node column in edges table

Use sqlite-graph to process and analyse graph.
``` sql
select showAdjTable();

select dijkstra('1', '6', 'weight');

select dijkstra('Brigadier Hoshiyar Singh', 'Raja Nahar Singh', 'weight');

select cypher('("0")-->(x)', 'x');

select cypher('("0")-->(x)-->(y)', 'x', 'y');

select cypher('("0")-->()-->()-->(x)', 'x');

select cypher('("0")-->()-->()-->()-->(x)', 'x');

select cypher('("0")-->()-->()-->()-->()-->(x)', 'x');

select cypher('("1")-[]->(x)-[]->(y)', 'x', 'y');

select cypher('(x)-->("6")', 'x');

select cypher('({"color": "red"})-->(x)', 'x');

select cypher('(x)-->({"color": "yellow"})', 'x');

select cypher('(x)-->(y)-->({"color": "yellow"})', 'x', 'y');

select cypher('(x)-->(y)-->("1000")', 'x', 'y');

select cypher('(x)-->()-->()-->()-->()-->("1000")', 'x');

select cypher('(x)-->()-->()-->("1000")', 'x');

select cypher('(x)-->()-->("1000")', 'x');

select cypher('(x)-->("1000")', 'x');

select cypher('(x)-->()', 'x');

create virtual table test using cypher('("0")-->(x)-->(y)-->(z)-->(w)-->(a)', x, y, z, w, a);

create virtual table test1 using cypher('(x)-[a]->(y)-[b]->(z)-[c]->("1000")', x, y, z, a, b, c);
```