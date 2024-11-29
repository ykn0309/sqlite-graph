#ifndef TYPES_H
#define TYPES_H

#include<unordered_map>
#include<set>
#include<vector>
#include<iostream>
#include"defs.h"

#ifdef __cplusplus
extern "C" {
#endif
#include"sqlite3.h"
#ifdef __cplusplus
}
#endif

struct Node {
    sqlite3_int64 iNode; //Node id
    std::set<sqlite3_int64>inNode; // In nodes
    std::set<sqlite3_int64>outNode; // Out nodes
    std::set<sqlite3_int64>inEdge; // In edges
    std::set<sqlite3_int64>outEdge; // Out edges
    Node(): iNode(-1) {}
    Node(sqlite3_int64 id): iNode(id) {}
};

struct Edge {
    sqlite3_int64 iEdge; // Edge id
    sqlite3_int64 fromNode; // From-node id
    sqlite3_int64 toNode; // To-node id
    Edge(): iEdge(-1), fromNode(-1), toNode(-1) {}
    Edge(sqlite3_int64 id, sqlite3_int64 in, sqlite3_int64 out): iEdge(id), fromNode(id), toNode(id) {} 
};

struct NodeMap {
    std::unordered_map<sqlite3_int64, Node*>map;

    unsigned int nNode; // Number of nodes

    // Find node by id. If node exists, return a pointer to node. Else, return nullptr.
    Node* find(sqlite3_int64 id) {
        auto it = map.find(id);
        if (it != map.end()) {
            return it->second;
        } else {
            return nullptr;
        }
    }

    // Insert node. If success, return 1. Else, return 0.
    int insert(sqlite3_int64 id, bool allow_duplicate) {
        if (id < 0) {
            std::cout<<"Node id can't smaller than 0.\n";
            return 0;
        }
        if (find(id) != nullptr) {
            if (!allow_duplicate) {
                std::cout<<"Node "<<id<<" already exists.\n";
                return 0;
            } else {
                return 1;
            }
        }
        Node *node = new Node(id);
        map[id] = node;
        nNode++;
        return 1;
    }

    // Remove node. If success, return 1. Else, return 0.
    int remove(sqlite3_int64 id) {
        if (id < 0) {
            std::cout<<"Node id can't smaller than 0.\n";
            return 0;
        }
        if (find(id) == nullptr) {
            std::cout<<"Node "<<id<<" doesn't exists.\n";
            return 0;
        } else {
            Node *node = map[id];
            map.erase(id);
            delete node;
            nNode--;
            return 1;
        }
    }

    // Return number of nodes
    unsigned int getNNode() {
        return nNode;
    }
};

struct EdgeMap {
    std::unordered_map<sqlite3_int64, Edge*>map;

    unsigned int nEdge; // Number of nodes

    // Return number of edges
    unsigned int getNEdge() {
        return nEdge;
    }

    // Find edge by id
    Edge* find(sqlite3_int64 id) {
        std::unordered_map<sqlite3_int64, Edge*>::iterator it = map.find(id);
        if (it != map.end()) {
            return it->second;
        } else {
            return nullptr;
        }
    }

    // Insert edge
    int insert(sqlite3_int64 id, sqlite3_int64 from, sqlite3_int64 to) {
        // Ensure edge id >= 0
        if (id < 0) {
            std::cout<<"Edge id can't smaller than 0.\n";
            return 0;
        }
        // Ensure from id and to id >= 0
        if (from < 0 || to < 0) {
            std::cout<<"Edge id can't smaller than 0.\n";
            return 0;
        }
        if (find(id) != nullptr) {
            std::cout<<"Edge "<<id<<" already exists.\n";
            return 0;
        } else {
            Edge *edge = new Edge(id, from, to); // Node in and Node out exist
            map[id] = edge;
            nEdge++;
            return 1;
        }
    }

    // remove edge
    int remove(sqlite3_int64 id) {
        // Ensure edge id >= 0
        if (id < 0) {
            std::cout<<"Edge id can't smaller than 0.\n";
            return 0;
        }
        if (find(id) == nullptr) {
            std::cout<<"Edge "<<id<<" doesn't exist.\n";
            return 0;
        } else {
            Edge *edge = map[id];
            map.erase(id);
            delete edge;
            nEdge--;
            return 1;
        }
    }
};

// 图和数据库中的表的绑定信息
struct BindingInfo {
    std::string node_table; // 结点表名
    std::string edge_table; // 边表名
    std::string node_label_alias; // 结点表中label列的别名
    std::string node_attribute_alias; // 结点表中attribute列的别名
    std::string edge_label_alias; // 边表中label列的别名
    std::string edge_attribute_alias; // 边表中attribute列的别名
    std::string from_node_alias; // 边表中from_node列的别名
    std::string to_node_alias; // 边表中to_node列的别名
    
    // 删除默认构造函数
    BindingInfo() = delete;

    // 构造函数
    BindingInfo(
        std::string nt,
        std::string et,
        std::string nla,
        std::string naa,
        std::string ela,
        std::string eaa,
        std::string fna,
        std::string tna
    ) :
    node_table(nt),
    edge_table(et),
    node_label_alias(nla),
    node_attribute_alias(naa),
    edge_label_alias(ela),
    edge_attribute_alias(eaa),
    from_node_alias(fna),
    to_node_alias(tna) {}
};

// Graph类
class Graph {
    private:
        sqlite3 *db; // 图的数据库连接
        sqlite3_int64 graph_id; // 图的id
        std::string graph_label; //图的label
        NodeMap *nodeMap; // 结点hash表
        EdgeMap *edgeMap; // 边hash表
        bool persistence; // 图是否持久化存储。如果persistence=1，数据库中的表需要和内存中的数据结构同步更新
        BindingInfo *binding_info; // 图和数据库中相关表的绑定信息

        // 通过结点的label查找结点id
        // label表示结点id
        // 找到对应结点，返回结点id；其他情况返回-1
        sqlite3_int64 getNodeIdByLabel(std::string label) {
            sqlite3_stmt *stmt;
            std::string sql;
            sql = "SELECT id FROM " + binding_info->node_table + " WHERE " + binding_info->node_label_alias + " = \"" + label + "\";";
            int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
            if (rc != SQLITE_OK) {
                std::cerr << "Error: " << sqlite3_errmsg(db) << std::endl;
                return -1;
            } else {
                if (sqlite3_step(stmt) == SQLITE_ROW) {
                    sqlite3_int64 id = sqlite3_column_int(stmt, 0);
                    return id;
                } else {
                    return -1;
                }
            }
        }

        // 通过label查找边id
        // label表示边id
        // 找到对应的边，返回边id；其他情况返回-1
        sqlite3_int64 getEdgeIdByLabel(std::string label) {
            sqlite3_stmt *stmt;
            std::string sql;
            sql = "SELECT id FROM " + binding_info->edge_table + " WHERE " + binding_info->edge_label_alias + " = \"" + label + "\";";
            int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
            if (rc != SQLITE_OK) {
                std::cerr << "Error: " << sqlite3_errmsg(db) <<std::endl;
                return -1;
            } else {
                if (sqlite3_step(stmt) == SQLITE_ROW) {
                    sqlite3_int64 id = sqlite3_column_int(stmt, 0);
                    return id;
                } else {
                    return -1;
                }
            }
        }

        // 向与Graph对象对应的NodeTable中插入一个结点
        // label和attribute是结点的标签和属性
        // 返回插入结点的id
        sqlite3_int64 insertNodeTable(std::string label, std::string attribute) {
            std::string insert_content; // 插入的内容
            insert_content = " (\"" + label + "\", \"" + attribute + "\") ";
            std::string insert_column; // 插入的列名
            insert_column = " (" + binding_info->node_label_alias + binding_info->node_attribute_alias + ") ";
            std::string sql; // 插入的SQL语句
            sql = "INSERT INTO " + binding_info->node_table + insert_column + "VALUES" + insert_content + ";";
            char *errMsg = 0;
            int rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0);
            if (rc != SQLITE_OK) {
                std::cerr << "Error: " << sqlite3_errmsg(db) << std::endl;
                return -1;
            } else {
                return getNodeIdByLabel(label);
            }
        }

        // 根据结点id，删除NodeTable中对应的结点
        // id为结点id
        // 删除失败，返回-1；删除成功，返回0
        int deleteNodeTable(sqlite3_int64 id) {
            std::string sql;
            sql = "DELETE FROM " + binding_info->node_table + " WHERE id = " + std::to_string(id);
            int rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0);
            if (rc != SQLITE_OK) {
                std::cerr << "Error: " << sqlite3_errmsg(db) << std::endl;
                return -1;
            }
            return 0;
        }

        // 更新结点id的label和attribute
        // type为更新结点表的类型：
        //      * 0：表示只更新label
        //      * 1：表示只更新attribute
        //      * 2：表示更新label和attribute
        //  成功返回0；失败返回-1
        int updateNodeTable(sqlite3_int64 id, std::string label, std::string attribute, int type) {
            std::string update_column;
            switch (type)
            {
                case UPDATE_LABEL: {
                    update_column = binding_info->node_label_alias + " = \"" + label + "\"";
                    break;
                }
                case UPDATE_ATTRIBUTE: {
                    update_column = binding_info->node_attribute_alias + " = \"" + attribute + "\"";
                    break;
                }
                case UPDATE_LABEL_ATTRIBUTE: {
                    std::string update_label = binding_info->node_label_alias + " = \"" + label + "\"";
                    std::string update_attribute = binding_info->node_attribute_alias + " = \"" + attribute + "\"";
                    update_column = update_label + ", " + update_attribute;
                    break;
                }
                default: {
                    std::cerr << "Incorrect type to update node table." << std::endl;
                    return -1;
                }
                return 0;
            }
            std::string sql;
            sql = "UPDATE " + binding_info->node_table + " SET " + update_column + " WHERE id = " + std::to_string(id);
            int rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0);
            if (rc != SQLITE_OK) {
                std::cerr << "Error: " << sqlite3_errmsg(db) << std::endl;
                return -1;
            }
            return 0;
        }

        // 向边表中插入一条边。插入成功，返回边的id；否则返回-1
        sqlite3_int64 insertEdgeTable(sqlite3_int64 from_node, sqlite3_int64 to_node, std::string label, std::string attribute) {
            std::string insert_column, insert_content, sql;
            insert_column = " (" + binding_info->from_node_alias + ", " + binding_info->to_node_alias + ", " +
                            binding_info->edge_label_alias + ", " + binding_info->edge_attribute_alias + ") ";
            insert_content = " (" + std::to_string(from_node) + ", " + std::to_string(to_node) + ", \"" + label +
                            "\", \"" + attribute + "\") ";
            sql = "INSERT INTO " + binding_info->edge_table + insert_column + "VALUES" + insert_content + ";";
            int rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0);
            if (rc != SQLITE_OK) {
                std::cerr << "Error: " << sqlite3_errmsg(db) << std::endl;
                return -1;
            } else {
                return getEdgeIdByLabel(label);
            }
        }

        // 根据边的id，在边表中删除这条边。删除成功，返回0；否则返回-1
        int deleteEdgeTable(sqlite3_int64 id) {
            std::string sql;
            sql = "DELETE FROM " + binding_info->edge_table + " WHERE id = " + std::to_string(id);
            int rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0);
            if (rc != SQLITE_OK) {
                std::cerr << "Error: " << sqlite3_errmsg(db) << std::endl;
                return -1;
            }
            return 0;
        }

        /*下面两个函数用来更新边表。由于边表的列比较多，所以进行了重载*/

        // 更新边表的from_node和to_node
        int updateEdgeTable(sqlite3_int64 id, sqlite3_int64 from_node, sqlite3_int64 to_node, int type) {
            std::string update_column;
            switch (type)
            {
                case UPDATE_FROM: {
                    update_column = binding_info->from_node_alias + " = " + std::to_string(from_node);
                    break;
                }
                case UPDATE_TO: {
                    update_column = binding_info->to_node_alias + " = " + std::to_string(to_node);
                    break;
                }
                case UPDATE_FROM_TO: {
                    std::string update_from = binding_info->from_node_alias + " = " + std::to_string(from_node);
                    std::string update_to = binding_info->to_node_alias + " = " + std::to_string(to_node);
                    update_column = update_from + ", " + update_to;
                    break;
                }
                default: {
                    std::cerr << "Incorrect type to update node table." << std::endl;
                    return -1;
                }
                return 0;
            }
            std::string sql;
            sql = "UPDATE " + binding_info->edge_table + " SET " + update_column + " WHERE id = " + std::to_string(id);
            int rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0);
            if (rc != SQLITE_OK) {
                std::cerr << "Error: " << sqlite3_errmsg(db) << std::endl;
                return -1;
            }
            return 0;
        }

        // 更新边表的label和attribute
        int updateEdgeTable(sqlite3_int64 id, std::string label, std::string attribute, int type) {
            std::string update_column;
            switch (type)
            {
                case UPDATE_LABEL: {
                    update_column = binding_info->edge_label_alias + " = \"" + label + "\"";
                    break;
                }
                case UPDATE_ATTRIBUTE: {
                    update_column = binding_info->edge_attribute_alias + " = \"" + attribute + "\"";
                    break;
                }
                case UPDATE_LABEL_ATTRIBUTE: {
                    std::string update_from = binding_info->edge_label_alias + " = \"" + label + "\"";
                    std::string update_to = binding_info->edge_attribute_alias + " = \"" + attribute + "\"";
                    update_column = update_from + ", " + update_to;
                    break;
                }
                default: {
                    std::cerr << "Incorrect type to update node table." << std::endl;
                    return -1;
                }
                return 0;
            }
            std::string sql;
            sql = "UPDATE " + binding_info->edge_table + " SET " + update_column + " WHERE id = " + std::to_string(id);
            int rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0);
            if (rc != SQLITE_OK) {
                std::cerr << "Error: " << sqlite3_errmsg(db) << std::endl;
                return -1;
            }
            return 0;
        }
    
    public:
        // Delete defaute constructor
        Graph() = delete;

        // Temporary graph constructor
        Graph(sqlite3 *db, sqlite3_int64 id, std::string graph_label): db(db), graph_id(id), graph_label(graph_label), persistence(0){
            nodeMap = new NodeMap();
            edgeMap = new EdgeMap();
        }

        // Persistence graph constructor
        Graph(sqlite3 *db, sqlite3_int64 id, std::string graph_label, BindingInfo *binding_info)
        : db(db), graph_id(id), graph_label(graph_label), persistence(1), binding_info(binding_info) {
            nodeMap = new NodeMap();
            edgeMap = new EdgeMap();
        }

        // Return number of nodes
        unsigned int getNNode() {
            return nodeMap->getNNode();
        }

        // Return number of edges
        unsigned int getNEdge() {
            return edgeMap->getNEdge();
        }
        
        // Add a node to nodeMap by id.
        int addNode(sqlite3_int64 id) {
            return nodeMap->insert(id, 0);
        }

        // Add a node to nodeMap by node's label and attribute. Binding node table will be updated.
        int addNode(std::string label, std::string attribute) {

        }

        // If node id exists, return 1.
        int sameAddNode(sqlite3_int64 id) {
            return nodeMap->insert(id, 1);
        }

        // Remove node id and its edges, also update its adj nodes.
        int removeNode(sqlite3_int64 id) {
            Node *node = nodeMap->find(id);

            if (!node) return 0;

            for (sqlite3_int64 i : node->inEdge) {
                Edge *e = edgeMap->find(i);
                sqlite3_int64 iNode = e->fromNode;
                Node *n = nodeMap->find(iNode);
                n->outNode.erase(id);
                n->outEdge.erase(i);
                edgeMap->remove(i);
            }

            for (sqlite3_int64 i : node->outEdge) {
                Edge *e = edgeMap->find(id);
                sqlite3_int64 iNode = e->toNode;
                Node *n = nodeMap->find(iNode);
                n->inNode.erase(id);
                n->inEdge.erase(i);
                edgeMap->remove(i);
            }
            
            return nodeMap->remove(id);
        }

        int addEdge(sqlite3_int64 id, sqlite3_int64 from, sqlite3_int64 to) {
            // Check if from-node exists
            if (nodeMap->find(from) == nullptr) {
                std::cout<<"In-node doesn't exist.\n";
                return 0;
            }
            // Check if to-node exists
            if (nodeMap->find(to) == nullptr) {
                std::cout<<"Out-node doesn't exist.\n";
                return 0;
            }

            // Add edge
            safeAddEdge(id, from, to);
        }

        // No check for from-node and to-node before add edge.
        int safeAddEdge(sqlite3_int64 id, sqlite3_int64 from, sqlite3_int64 to) {
            if (edgeMap->insert(id, from, to)) {
                // add attributes of from-node and to-node
                Node *from_node = nodeMap->find(from);
                Node *to_node = nodeMap->find(to);
                from_node->outEdge.insert(to);
                from_node->outEdge.insert(id);
                to_node->inNode.insert(from);
                to_node->inEdge.insert(id);
                return 1;
            } else {
                return 0;
            }
        }

        // Remove edge id and update related in-node and out-node.
        int removeEdge(sqlite3_int64 id) {
            Edge *edge = edgeMap->find(id);
            if (!edge) return 0;
            Node *from = nodeMap->find(edge->fromNode);
            Node *to = nodeMap->find(edge->toNode);
            from->outNode.erase(to->iNode);
            from->outEdge.erase(id);
            to->inNode.erase(from->iNode);
            to->inEdge.erase(id);
            return edgeMap->remove(id);
        }

        // Return a vector containing all nodes' pointer
        std::vector<Node*> nodeList() {
            std::vector<Node*>list;
            for (auto it = nodeMap->map.begin(); it != nodeMap->map.end(0); it++) {
                Node *n = it->second;
                list.push_back(n);
            }
            return list;
        }
};

#endif