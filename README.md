# sqlite-graph
A graph extension for SQLite

``` sql
create table nodes(id INTEGER PRIMARY KEY AUTOINCREMENT, label TEXT UNIQUE, attribute TEXT);

create table edges(id INTEGER PRIMARY KEY AUTOINCREMENT, from_node TEXT, to_node TEXT, label TEXT UNIQUE, attribute TEXT);

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

.open graph.db

.load ./graph

select createGraph('nodes', 'edges', 'label', 'attribute', 'label', 'attribute', 'from_node', 'to_node');

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