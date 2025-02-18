#ifndef GRAPH_H
#define GRAPH_H

#include<iostream>
#include<unordered_map>
#include<set>
#include<vector>
#include"defs.h"
#include"json.hpp"

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
    Edge() = delete; // 删除默认构造函数
    Edge(sqlite3_int64 id, sqlite3_int64 from, sqlite3_int64 to): iEdge(id), fromNode(from), toNode(to) {} 
};

struct NodeMap {
    std::unordered_map<sqlite3_int64, Node*>map;

    unsigned int nNode; // Number of nodes

    // Find node by id. If node exists, return a pointer to node. Else, return nullptr.
    Node* find(sqlite3_int64 id);
    
    /// @brief 插入结点
    /// @param id 插入结点的id
    /// @return 成功返回GRAPH_SUCCESS，失败返回GRAPH_FAILED
    int insert(sqlite3_int64 id);

    // Remove node. If success, return GRAPH_SUCCESS. Else, return GRAPH_FAILED.
    int remove(sqlite3_int64 id);

    // Return number of nodes
    unsigned int getNNode();
};

struct EdgeMap {
    std::unordered_map<sqlite3_int64, Edge*>map;

    unsigned int nEdge; // Number of nodes

    // Return number of edges
    unsigned int getNEdge();

    // Find edge by id
    Edge* find(sqlite3_int64 id);

    /// @brief 向EdgeMap中插入边
    /// @param id 插入边的id
    /// @param from 边的起始结点
    /// @param to 变得结束结点
    /// @return 
    int insert(sqlite3_int64 id, sqlite3_int64 from, sqlite3_int64 to);

    /// @brief 删除边id
    /// @param id 要删除边的id
    /// @return 删除成功，返回``GRAPH_SUCCESS``；删除失败，返回``GRAPH_FAILED``
    int remove(sqlite3_int64 id);
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
        friend class Cypher;
        friend class Parser;
        friend class BFS;
        friend class DFS;
        friend class Dijkstra;
        sqlite3 *db; // 图的数据库连接
        NodeMap *nodeMap; // 结点hash表
        EdgeMap *edgeMap; // 边hash表
        BindingInfo *binding_info; // 图和数据库中相关表的绑定信息

        // 向与Graph对象对应的NodeTable中插入一个结点
        // label和attribute是结点的标签和属性
        // 返回插入结点的id，如果失败，返回GRAPH_FAILED
        sqlite3_int64 addNodeTable(std::string label, std::string attribute);

        // 根据结点id，删除NodeTable中对应的结点
        // id为结点id
        // 删除失败，返回GRAPH_FAILED；删除成功，返回GRAPH_SUCCESS
        int removeNodeTable(sqlite3_int64 id);

        // 更新结点id的label和attribute
        // type为更新结点表的类型：
        //      * 0：表示只更新label
        //      * 1：表示只更新attribute
        //      * 2：表示更新label和attribute
        //  成功返回GRAPH_SUCCESS；失败返回GRAPH_FAILED
        int updateNodeTable(sqlite3_int64 id, std::string label, std::string attribute, int type);

        // 向边表中插入一条边。插入成功，返回边的id；否则返回GRAPH_FAILED
        sqlite3_int64 addEdgeTable(sqlite3_int64 from_node, sqlite3_int64 to_node, std::string label, std::string attribute);

        // 根据边的id，在边表中删除这条边。删除成功，返回``GRAPH_SUCCESS``；否则返回``GRAPH_FAILED``
        int removeEdgeTable(sqlite3_int64 id);

        /*下面两个函数用来更新边表。由于边表的列比较多，所以进行了重载*/

        // 更新边表的from_node和to_node
        int updateEdgeTable(sqlite3_int64 id, sqlite3_int64 from_node, sqlite3_int64 to_node, int type);

        // 更新边表的label和attribute
        int updateEdgeTable(sqlite3_int64 id, std::string label, std::string attribute, int type);

        /// @brief 根据图对象的binding_info，把结点和边的数据导入到图数据结构中
        /// @return 成功，返回``GRAPH_SUCCESS``，否则返回``GRAPH_FAILED``
        int loadGraphFromTable();
    
    public:
        // 删除默认构造函数
        Graph() = delete;

        // 构造函数
        Graph(sqlite3 *db, BindingInfo *binding_info): db(db), binding_info(binding_info) {
            nodeMap = new NodeMap();
            edgeMap = new EdgeMap();
            if (loadGraphFromTable() != GRAPH_SUCCESS) {
                std::cerr << "Error: load graph from table failed." << std::endl;
            }
        }

        // 析构函数
        ~Graph() {
            delete binding_info;
            delete nodeMap;
            delete edgeMap;
        }

        // Return number of nodes
        unsigned int getNNode();

        // Return number of edges
        unsigned int getNEdge();

        // 通过结点的label查找结点id
        // label表示结点id
        // 找到对应结点，返回结点id；其他情况返回GRAPH_FAILED
        sqlite3_int64 getNodeIdByLabel(std::string label);

        // 通过label查找边id
        // label表示边id
        // 找到对应的边，返回边id；其他情况返回GRAPH_FAILED
        sqlite3_int64 getEdgeIdByLabel(std::string label);

        /// @brief 根据id获取结点label
        /// @param id 结点id
        /// @return 结点label
        std::string getNodeLabelById(sqlite3_int64 id);

        /// @brief 根据id获取边的label
        /// @param id 边id
        /// @return 边label
        std::string getEdgeLabelById(sqlite3_int64 id);

        /// @brief 根据id获取结点的attribute
        /// @param id 结点id
        /// @return 结点attribute
        std::string getNodeAttributeById(sqlite3_int64 id);

        /// @brief 根据id获取边的attribute
        /// @param id 边id
        /// @return 边attribute
        std::string getEdgeAttributeById(sqlite3_int64 id);

        double getEdgeWeight(sqlite3_int64 id, std::string weight_alias);

        /// @brief 根据结点id添加结点。这个函数只有从结点表添加结点时被直接调用
        /// @param id 要添加结点的id
        /// @return 添加成功，返回``GRAPH_SUCCESS`，否则返回``GRAPH_FAILED``
        int addNode(sqlite3_int64 id);

        /// @brief Add a node to nodeMap by node's label and attribute. Binding node table will be updated.
        /// @param label 结点的label
        /// @param attribute 结点的attribute
        /// @return 添加成功，返回``GRAPH_SUCCESS``，否则返回``GRAPH_FAILED``
        int addNode(std::string label, std::string attribute);

        /// @brief 删除结点。同时会删除与结点关联的边，更新结点的邻接结点的信息并且更新数据表
        /// @param id 要删除结点的id
        /// @return 如果成功返回``GRAPH_SUCCESS``，否则返回``GRAPH_FAILED``
        int removeNode(sqlite3_int64 id);

        /// @brief 根据结点的label来删除结点
        /// @param label 要删除结点的label
        /// @return 如果成功返回``GRAPH_SUCCESS``，否则返回``GRAPH_FAILED``
        int removeNode(std::string label);

        /// @brief 在不检查from node和to node的情况下添加边（保证from node和to node都存在）
        /// @param id 添加边的id
        /// @param from 起始结点id
        /// @param to 结束结点id
        /// @return 成功返回``GRAPH_SUCCESS``，否则返回``GRAPH_FAILED``
        int AddEdgeWithoutCheckFromAndTo(sqlite3_int64 id, sqlite3_int64 from, sqlite3_int64 to);

        /// @brief 通过边、起始结点、结束结点的id添加边。这个函数只在通过读取边表添加边时直接使用
        /// @param id 边的id
        /// @param from 起始结点的id
        /// @param to 结束结点的id
        /// @return 成功返回``GRAPH_SUCCESS``，否则返回``GRAPH_FAILED``
        int addEdge(sqlite3_int64 id, sqlite3_int64 from, sqlite3_int64 to);

        /// @brief 通过边、起始结点、结束结点的label添加边
        /// @param label 边的label
        /// @param from_label 起始结点的label
        /// @param to_label 结束结点的label
        /// @return 成功返回``GRAPH_SUCCESS``，否则返回``GRAPH_FAILED``
        int addEdge(std::string label, std::string attribute, std::string from_label, std::string to_label);

        /// @brief 用id删除结点并更新与其关联的from node和to node
        /// @param id 要删除结点的id
        /// @return 成功返回``GRAPH_SUCCESS``，否则返回``GRAPH_FAILED``
        int removeEdge(sqlite3_int64 id);

        /// @brief 根据结点的label来删除边
        /// @param label 要删除边的label
        /// @return 如果成功返回``GRAPH_SUCCESS``，否则返回``GRAPH_FAILED``
        int removeEdge(std::string label);

        // Return a vector containing all nodes' pointer
        std::vector<Node*> nodeList();
};

using json = nlohmann::json;

Node* NodeMap::find(sqlite3_int64 id) {
        auto it = map.find(id);
        if (it != map.end()) {
            return it->second;
        } else {
            return nullptr;
        }
    }

int NodeMap::insert(sqlite3_int64 id) {
    if (id < 0) {
        std::cout<<"Node id can't smaller than 0.\n";
        return GRAPH_SUCCESS;
    }
    if (find(id) != nullptr) {
        std::cout<<"Node "<<id<<" already exists.\n";
        return GRAPH_FAILED;
    }
    Node *node = new Node(id);
    map[id] = node;
    nNode++;
    return GRAPH_SUCCESS;
}

int NodeMap::remove(sqlite3_int64 id) {
    if (id < 0) {
        std::cout<<"Node id can't smaller than 0.\n";
        return GRAPH_FAILED;
    }
    if (find(id) == nullptr) {
        std::cout<<"Node "<<id<<" doesn't exists.\n";
        return GRAPH_FAILED;
    } else {
        Node *node = map[id];
        map.erase(id);
        delete node;
        nNode--;
        return GRAPH_SUCCESS;
    }
}

unsigned int NodeMap::getNNode() {
    return nNode;
}

unsigned int EdgeMap::getNEdge() {
    return nEdge;
}

Edge* EdgeMap::find(sqlite3_int64 id) {
    std::unordered_map<sqlite3_int64, Edge*>::iterator it = map.find(id);
    if (it != map.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}

int EdgeMap::insert(sqlite3_int64 id, sqlite3_int64 from, sqlite3_int64 to) {
    // 确保edge id >= 0
    if (id < 0) {
        std::cout<<"Edge id can't smaller than 0.\n";
        return GRAPH_FAILED;
    }
    // Ensure from id and to id >= 0
    if (from < 0 || to < 0) {
        std::cout<<"Edge id can't smaller than 0.\n";
        return GRAPH_FAILED;
    }
    if (find(id) != nullptr) {
        std::cout<<"Edge "<<id<<" already exists.\n";
        return GRAPH_FAILED;
    } else {
        Edge *edge = new Edge(id, from, to); // Node in and Node out exist
        map[id] = edge;
        nEdge++;
        return GRAPH_SUCCESS;
    }
}

int EdgeMap::remove(sqlite3_int64 id) {
    // Ensure edge id >= 0
    if (id < 0) {
        std::cout<<"Edge id can't smaller than 0.\n";
        return GRAPH_FAILED;
    }
    if (find(id) == nullptr) {
        std::cout<<"Edge "<<id<<" doesn't exist.\n";
        return GRAPH_FAILED;
    } else {
        Edge *edge = map[id];
        map.erase(id);
        delete edge;
        nEdge--;
        return GRAPH_SUCCESS;
    }
}

sqlite3_int64 Graph::getNodeIdByLabel(std::string label) {
    sqlite3_stmt *stmt;
    std::string sql;
    sql = "SELECT id FROM " + binding_info->node_table + " WHERE " + binding_info->node_label_alias + " = \'" + label + "\';";
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "Error: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return GRAPH_FAILED;
    } else {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            sqlite3_int64 id = sqlite3_column_int(stmt, 0);
            sqlite3_finalize(stmt);
            return id;
        } else {
            sqlite3_finalize(stmt);
            return GRAPH_FAILED;
        }
    }
}

sqlite3_int64 Graph::getEdgeIdByLabel(std::string label) {
    sqlite3_stmt *stmt;
    std::string sql;
    sql = "SELECT id FROM " + binding_info->edge_table + " WHERE " + binding_info->edge_label_alias + " = \'" + label + "\';";
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "Error: " << sqlite3_errmsg(db) <<std::endl;
        sqlite3_finalize(stmt);
        return GRAPH_FAILED;
    } else {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            sqlite3_int64 id = sqlite3_column_int(stmt, 0);
            sqlite3_finalize(stmt);
            return id;
        } else {
            sqlite3_finalize(stmt);
            return GRAPH_FAILED;
        }
    }
}

std::string Graph::getNodeLabelById(sqlite3_int64 id) {
    sqlite3_stmt *stmt;
    std::string sql;
    sql = "SELECT " + binding_info->node_label_alias + " FROM " + binding_info->node_table + " WHERE id = " + std::to_string(id) + ";";
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "Error: " << sqlite3_errmsg(db) <<std::endl;
        sqlite3_finalize(stmt);
        return "ERROR";
    } else {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string label  = (const char*)sqlite3_column_text(stmt, 0);
            sqlite3_finalize(stmt);
            return label;
        } else {
            sqlite3_finalize(stmt);
            return "ERROR";
        }
    }
}

std::string Graph::getEdgeLabelById(sqlite3_int64 id) {
    sqlite3_stmt *stmt;
    std::string sql;
    sql = "SELECT " + binding_info->edge_label_alias + " FROM " + binding_info->edge_table + " WHERE id = " + std::to_string(id) + ";";
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "Error: " << sqlite3_errmsg(db) <<std::endl;
        sqlite3_finalize(stmt);
        return "ERROR";
    } else {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string label  = (const char*)sqlite3_column_text(stmt, 0);
            sqlite3_finalize(stmt);
            return label;
        } else {
            sqlite3_finalize(stmt);
            return "ERROR";
        }
    }
}

std::string Graph::getNodeAttributeById(sqlite3_int64 id) {
    sqlite3_stmt *stmt;
    std::string sql;
    sql = "SELECT " + binding_info->node_attribute_alias + " FROM " + binding_info->node_table + " WHERE id = " + std::to_string(id) + ";";
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "Error: " << sqlite3_errmsg(db) <<std::endl;
        sqlite3_finalize(stmt);
        return "ERROR";
    } else {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string attribute  = (const char*)sqlite3_column_text(stmt, 0);
            sqlite3_finalize(stmt);
            return attribute;
        } else {
            sqlite3_finalize(stmt);
            return "ERROR";
        }
    }
}

std::string Graph::getEdgeAttributeById(sqlite3_int64 id) {
    sqlite3_stmt *stmt;
    std::string sql;
    sql = "SELECT " + binding_info->edge_attribute_alias + " FROM " + binding_info->edge_table + " WHERE id = " + std::to_string(id) + ";";
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "Error: " << sqlite3_errmsg(db) <<std::endl;
        sqlite3_finalize(stmt);
        return "ERROR";
    } else {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string attribute  = (const char*)sqlite3_column_text(stmt, 0);
            sqlite3_finalize(stmt);
            return attribute;
        } else {
            sqlite3_finalize(stmt);
            return "ERROR";
        }
    }
}

double Graph::getEdgeWeight(sqlite3_int64 id, std::string weight_alias) {
    std::string attribute = getEdgeAttributeById(id);
    json data = json::parse(attribute);
    double weight = data[weight_alias];
    return weight;
}

sqlite3_int64 Graph::addNodeTable(std::string label, std::string attribute) {
    std::string insert_content; // 插入的内容
    insert_content = " (\'" + label + "\', \'" + attribute + "\') ";
    std::string insert_column; // 插入的列名
    insert_column = " (" + binding_info->node_label_alias + ", " + binding_info->node_attribute_alias + ") ";
    std::string sql; // 插入的SQL语句
    sql = "INSERT INTO " + binding_info->node_table + insert_column + "VALUES" + insert_content + ";";
    char *errMsg = 0;
    int rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "Error: " << sqlite3_errmsg(db) << std::endl;
        return GRAPH_FAILED;
    } else {
        return getNodeIdByLabel(label);
    }
}

int Graph::removeNodeTable(sqlite3_int64 id) {
    std::string sql;
    sql = "DELETE FROM " + binding_info->node_table + " WHERE id = " + std::to_string(id);
    int rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "Error: " << sqlite3_errmsg(db) << std::endl;
        return GRAPH_FAILED;
    }
    return GRAPH_SUCCESS;
}

int Graph::updateNodeTable(sqlite3_int64 id, std::string label, std::string attribute, int type) {
    std::string update_column;
    switch (type)
    {
        case UPDATE_LABEL: {
            update_column = binding_info->node_label_alias + " = \'" + label + "\'";
            break;
        }
        case UPDATE_ATTRIBUTE: {
            update_column = binding_info->node_attribute_alias + " = \'" + attribute + "\'";
            break;
        }
        case UPDATE_LABEL_ATTRIBUTE: {
            std::string update_label = binding_info->node_label_alias + " = \'" + label + "\'";
            std::string update_attribute = binding_info->node_attribute_alias + " = \'" + attribute + "\'";
            update_column = update_label + ", " + update_attribute;
            break;
        }
        default: {
            std::cerr << "Incorrect type to update node table." << std::endl;
            return GRAPH_FAILED;
        }
        return GRAPH_SUCCESS;
    }
    std::string sql;
    sql = "UPDATE " + binding_info->node_table + " SET " + update_column + " WHERE id = " + std::to_string(id);
    int rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "Error: " << sqlite3_errmsg(db) << std::endl;
        return GRAPH_FAILED;
    }
    return GRAPH_SUCCESS;
}

sqlite3_int64 Graph::addEdgeTable(sqlite3_int64 from_node, sqlite3_int64 to_node, std::string label, std::string attribute) {
    std::string insert_column, insert_content, sql;
    insert_column = " (" + binding_info->from_node_alias + ", " + binding_info->to_node_alias + ", " +
                    binding_info->edge_label_alias + ", " + binding_info->edge_attribute_alias + ") ";
    insert_content = " (" + std::to_string(from_node) + ", " + std::to_string(to_node) + ", \'" + label +
                    "\', \'" + attribute + "\') ";
    sql = "INSERT INTO " + binding_info->edge_table + insert_column + "VALUES" + insert_content + ";";
    int rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "Error: " << sqlite3_errmsg(db) << std::endl;
        return GRAPH_FAILED;
    } else {
        return getEdgeIdByLabel(label);
    }
}

int Graph::removeEdgeTable(sqlite3_int64 id) {
    std::string sql;
    sql = "DELETE FROM " + binding_info->edge_table + " WHERE id = " + std::to_string(id);
    int rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "Error: " << sqlite3_errmsg(db) << std::endl;
        return GRAPH_FAILED;
    }
    return GRAPH_SUCCESS;
}

int Graph::updateEdgeTable(sqlite3_int64 id, sqlite3_int64 from_node, sqlite3_int64 to_node, int type) {
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
            return GRAPH_FAILED;
        }
        return GRAPH_SUCCESS;
    }
    std::string sql;
    sql = "UPDATE " + binding_info->edge_table + " SET " + update_column + " WHERE id = " + std::to_string(id);
    int rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "Error: " << sqlite3_errmsg(db) << std::endl;
        return GRAPH_FAILED;
    }
    return GRAPH_SUCCESS;
}

int Graph::updateEdgeTable(sqlite3_int64 id, std::string label, std::string attribute, int type) {
    std::string update_column;
    switch (type)
    {
        case UPDATE_LABEL: {
            update_column = binding_info->edge_label_alias + " = \'" + label + "\'";
            break;
        }
        case UPDATE_ATTRIBUTE: {
            update_column = binding_info->edge_attribute_alias + " = \'" + attribute + "\'";
            break;
        }
        case UPDATE_LABEL_ATTRIBUTE: {
            std::string update_from = binding_info->edge_label_alias + " = \'" + label + "\'";
            std::string update_to = binding_info->edge_attribute_alias + " = \'" + attribute + "\'";
            update_column = update_from + ", " + update_to;
            break;
        }
        default: {
            std::cerr << "Incorrect type to update node table." << std::endl;
            return GRAPH_FAILED;
        }
        return GRAPH_SUCCESS;
    }
    std::string sql;
    sql = "UPDATE " + binding_info->edge_table + " SET " + update_column + " WHERE id = " + std::to_string(id);
    int rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "Error: " << sqlite3_errmsg(db) << std::endl;
        return GRAPH_FAILED;
    }
    return GRAPH_SUCCESS;
}

int Graph::loadGraphFromTable() {
    int rc = SQLITE_OK;
    std::string sql;
    sql = "SELECT id FROM " + binding_info->node_table + ";";
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error: Failed to prepare SQL." << std::endl;
        sqlite3_finalize(stmt);
        return GRAPH_FAILED;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        sqlite3_int64 iNode = sqlite3_column_int64(stmt, 0);
        if (addNode(iNode) != GRAPH_SUCCESS) {
            std::cerr << "Error: Failed to add node." << std::endl;
            sqlite3_finalize(stmt);
            return GRAPH_FAILED;
        }
    }

    std::string selected_column = "id, " + binding_info->from_node_alias + ", " + binding_info->to_node_alias;
    sql = "SELECT " + selected_column + " FROM " + binding_info->edge_table + ";";
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error: Failed to prepare SQL." << std::endl;
        sqlite3_finalize(stmt);
        return GRAPH_FAILED;
    }
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        sqlite3_int64 iEdge = sqlite3_column_int64(stmt, 0); // Edge id
        std::string from_label = (const char*)sqlite3_column_text(stmt, 1); // From-node lab
        std::string to_label = (const char*)sqlite3_column_text(stmt, 2); // To-node id

        sqlite3_int64 from_node = getNodeIdByLabel(from_label);
        sqlite3_int64 to_node = getNodeIdByLabel(to_label);

        // 添加边
        if (addEdge(iEdge, from_node, to_node) != GRAPH_SUCCESS) {
            std::cerr << "Error: Failed to add edge." << std::endl;
            sqlite3_finalize(stmt);
            return GRAPH_FAILED;
        }
    }
    sqlite3_finalize(stmt);
    return GRAPH_SUCCESS;
}

unsigned int Graph::getNNode() {
    return nodeMap->getNNode();
}

unsigned int Graph::getNEdge() {
    return edgeMap->getNEdge();
}

int Graph::addNode(sqlite3_int64 id) {
    return nodeMap->insert(id);
}

int Graph::addNode(std::string label, std::string attribute) {
    sqlite3_int64 id = addNodeTable(label, attribute);
    if (id == -1) {
        return GRAPH_FAILED; // 插入失败
    } else {
        return nodeMap->insert(id);
    }
}

int Graph::removeNode(sqlite3_int64 id) {
    Node *node = nodeMap->find(id);
    if (!node) return 0;
    while  (!node->inEdge.empty()) {
        sqlite3_int64 i = *node->inEdge.begin();
        Edge *e = edgeMap->find(i);
        sqlite3_int64 iNode = e->fromNode;
        Node *n = nodeMap->find(iNode);
        n->outNode.erase(id);
        n->outEdge.erase(i);
        removeEdge(i);
    }
    while (!node->outEdge.empty()) {
        sqlite3_int64 i = *node->inEdge.begin();
        Edge *e = edgeMap->find(id);
        sqlite3_int64 iNode = e->toNode;
        Node *n = nodeMap->find(iNode);
        n->inNode.erase(id);
        n->inEdge.erase(i);
        removeEdge(i);
    }
    if (nodeMap->remove(id) == GRAPH_SUCCESS) {
        return removeNodeTable(id); // 
    } else {
        return GRAPH_FAILED;
    }
}

int Graph::removeNode(std::string label) {
    sqlite3_int64 id = getNodeIdByLabel(label);
    return removeNode(id);
}

int Graph::AddEdgeWithoutCheckFromAndTo(sqlite3_int64 id, sqlite3_int64 from, sqlite3_int64 to) {
    if (edgeMap->insert(id, from, to) == GRAPH_SUCCESS) {
        // add attributes of from-node and to-node
        Node *from_node = nodeMap->find(from);
        Node *to_node = nodeMap->find(to);
        from_node->outNode.insert(to);
        from_node->outEdge.insert(id);
        to_node->inNode.insert(from);
        to_node->inEdge.insert(id);
        return GRAPH_SUCCESS;
    } else {
        return GRAPH_FAILED;
    }
}

int Graph::addEdge(sqlite3_int64 id, sqlite3_int64 from, sqlite3_int64 to) {
    // Check if from-node exists
    if (nodeMap->find(from) == nullptr) {
        std::cout<<"In-node doesn't exist.\n";
        return GRAPH_FAILED;
    }
    // Check if to-node exists
    if (nodeMap->find(to) == nullptr) {
        std::cout<<"Out-node doesn't exist.\n";
        return GRAPH_FAILED;
    }

    // Add edge
    return AddEdgeWithoutCheckFromAndTo(id, from, to);
}

int Graph::addEdge(std::string label, std::string attribute, std::string from_label, std::string to_label) {
    sqlite3_int64 from = getNodeIdByLabel(from_label);
    sqlite3_int64 to = getNodeIdByLabel(to_label);
    sqlite3_int64 id = addEdgeTable(from, to, label, attribute);
    if (from == GRAPH_FAILED || to == GRAPH_FAILED || id == GRAPH_FAILED) {
        return GRAPH_FAILED;
    }
    return addEdge(id, from, to);
}

int Graph::removeEdge(sqlite3_int64 id) {
    Edge *edge = edgeMap->find(id);
    if (!edge) return GRAPH_FAILED;
    Node *from = nodeMap->find(edge->fromNode);
    Node *to = nodeMap->find(edge->toNode);
    from->outNode.erase(to->iNode);
    from->outEdge.erase(id);
    to->inNode.erase(from->iNode);
    to->inEdge.erase(id);
    if (edgeMap->remove(id) == GRAPH_SUCCESS) {
        return removeEdgeTable(id);
    } else {
        return GRAPH_FAILED;
    }
}

int Graph::removeEdge(std::string label) {
    sqlite3_int64 id = getNodeIdByLabel(label);
    return removeEdge(id);
}

std::vector<Node*> Graph::nodeList() {
    std::vector<Node*>list;
    for (auto it = nodeMap->map.begin(); it != nodeMap->map.end(); it++) {
        Node *n = it->second;
        list.push_back(n);
    }
    return list;
}

#endif