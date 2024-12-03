# sqlite-graph
A graph extension for SQLite

## to-do

* statement有的没有释放
* 有bug未修复

```
create table nodes(id integer, label text, attribute text);

create table edges(id integer, from_node integer, to_node integer, label text, attribute text);

insert into nodes values
    (0, 'zero', 'color: red'),
    (1, 'one', 'color: blue'),
    (2, 'two', 'color: green'),
    (3, 'three', 'color: yellow'),
    (4, 'four', 'color: orange');

insert into edges values
    (0, 0, 1, 'A', 'weight: 1'),
    (1, 0, 2, 'B', 'weight: 1'),
    (2, 1, 3, 'C', 'weight: 1'),
    (3, 2, 3, 'D', 'weight: 1'),
    (4, 0, 4, 'E', 'weight: 1'),
    (5, 1, 4, 'F', 'weight: 1'),
    (6, 2, 4, 'G', 'weight: 1'),
    (7, 3, 4, 'H', 'weight: 1');

.open graph.db

.load ./graph

select create('nodes', 'edges', 'label', 'attribute', 'label', 'attribute', 'from_node', 'to_node');

select showAdjTable();
```