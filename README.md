# sqlite-graph
A graph extension for SQLite

## to-do
* 重构代码为Cpp。
* 现在的代码是按照结点的个数创建一个数组，结点的id和数组的index是一一对应的。这样不够灵活，涉及到增加和删除结点的操作就会有问题。后续打算修改成哈希表。
* 增加一些数据结构，如NodeList, EdgeList，用来保存所有的结点和边

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

select createAdjList('nodes', 'edges');

select showAdjList();
```