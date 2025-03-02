-- SQL求正向邻居节点
SELECT COUNT(DISTINCT e2.to_node) AS second_degree_neighbors_count
FROM edges e1
JOIN edges e2 ON e1.to_node = e2.from_node
WHERE e1.from_node = '0';

SELECT COUNT(DISTINCT e2.to_node) AS second_degree_neighbors_count
FROM edges e1
JOIN edges e2 ON e1.to_node = e2.from_node
WHERE e1.from_node = '0';


SELECT COUNT(DISTINCT e3.to_node) AS third_degree_neighbors_count
FROM edges e1
JOIN edges e2 ON e1.to_node = e2.from_node
JOIN edges e3 ON e2.to_node = e3.from_node
WHERE e1.from_node = '0';

SELECT DISTINCT e3.to_node AS third_degree_neighbors_count
FROM edges e1
JOIN edges e2 ON e1.to_node = e2.from_node
JOIN edges e3 ON e2.to_node = e3.from_node
WHERE e1.from_node = '0';


SELECT COUNT(DISTINCT e4.to_node) AS fourth_degree_neighbors_count
FROM edges e1
JOIN edges e2 ON e1.to_node = e2.from_node
JOIN edges e3 ON e2.to_node = e3.from_node
JOIN edges e4 ON e3.to_node = e4.from_node
WHERE e1.from_node = '0'; 

SELECT COUNT(DISTINCT e4.to_node) AS fourth_degree_neighbors_count
FROM edges e1
JOIN edges e2 ON e1.to_node = e2.from_node
JOIN edges e3 ON e2.to_node = e3.from_node
JOIN edges e4 ON e3.to_node = e4.from_node
WHERE e1.from_node = '0'; 

SELECT COUNT(DISTINCT e5.to_node) AS fifth_degree_neighbors_count
FROM edges e1
JOIN edges e2 ON e1.to_node = e2.from_node
JOIN edges e3 ON e2.to_node = e3.from_node
JOIN edges e4 ON e3.to_node = e4.from_node
JOIN edges e5 ON e4.to_node = e5.from_node
WHERE e1.from_node = '0';

SELECT DISTINCT e5.to_node AS fifth_degree_neighbors_count
FROM edges e1
JOIN edges e2 ON e1.to_node = e2.from_node
JOIN edges e3 ON e2.to_node = e3.from_node
JOIN edges e4 ON e3.to_node = e4.from_node
JOIN edges e5 ON e4.to_node = e5.from_node
WHERE e1.from_node = '0';

-- SQL反向二阶邻居节点
SELECT COUNT(DISTINCT e3.to_node) AS reverse_second_degree_neighbors_count
FROM edges e1
JOIN edges e2 ON e1.from_node = e2.to_node
JOIN edges e3 ON e2.from_node = e3.to_node
WHERE e1.to_node = '1000';

-- Cypher求正向邻居节点
MATCH (n:Node {id: 0})-[:CONNECTED_TO]->(neighbor)
RETURN COUNT(DISTINCT neighbor) AS neighbor_count;

MATCH (n:Node {id: 0})-[:CONNECTED_TO]->()-[:CONNECTED_TO]->()-[:CONNECTED_TO]->()-[:CONNECTED_TO]->()-[:CONNECTED_TO]->(neighbor)
RETURN COUNT(DISTINCT neighbor) AS neighbor_count;


-- Cypher导出节点
MATCH (n:Node {id: 0})-[:CONNECTED_TO]->()-[:CONNECTED_TO]->()-[:CONNECTED_TO]->(neighbor)
WITH collect(DISTINCT neighbor.id) AS ids
RETURN reduce(s = '', id IN ids | s + CASE WHEN s = '' THEN '' ELSE ',' END + toString(id)) AS node_ids
