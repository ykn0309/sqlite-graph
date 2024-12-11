#ifndef TYPES_H
#define TYPES_H

#include<iostream>
#include<unordered_map>
#include<set>
#include<vector>
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
        friend class BFS;
        sqlite3 *db; // 图的数据库连接
        NodeMap *nodeMap; // 结点hash表
        EdgeMap *edgeMap; // 边hash表
        BindingInfo *binding_info; // 图和数据库中相关表的绑定信息

        // 通过结点的label查找结点id
        // label表示结点id
        // 找到对应结点，返回结点id；其他情况返回GRAPH_FAILED
        sqlite3_int64 getNodeIdByLabel(std::string label);

        // 通过label查找边id
        // label表示边id
        // 找到对应的边，返回边id；其他情况返回GRAPH_FAILED
        sqlite3_int64 getEdgeIdByLabel(std::string label);

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

#endif