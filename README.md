# sqlite-graph
A graph extension for SQLite

## to-do



``` sql
create table nodes(id integer primary key autoincrement, label text, attribute text);

create table edges(id integer primary key autoincrement, from_node text, to_node text, label text, attribute text);

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

-- insert into nodes (label, attribute) values
--     ('1', ''),
--     ('2', ''),
--     ('3', ''),
--     ('4', ''),
--     ('5', ''),
--     ('6', '');
-- insert into edges (from_node, to_node, label, attribute) values
--     ('1', '3', 'a', '"weight": 10'),
--     ('2', '3', 'b', '"weight": 5'),
--     ('1', '5', 'c', '"weight": 30'),
--     ('1', '6', 'd', '"weight": 100'),
--     ('3', '4', 'e', '"weight": 50'),
--     ('5', '4', 'f', '"weight": 20'),
--     ('4', '6', 'g', '"weight": 10');

.open graph.db

.load ./graph

select createGraph('nodes', 'edges', 'label', 'attribute', 'label', 'attribute', 'from_node', 'to_node');

select showAdjTable();

select dijkstra('1', '6', 'weight');

select cypher('("0")-->(x)', 'x');

select cypher('("0")-->()-->(x)', 'x');

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
```