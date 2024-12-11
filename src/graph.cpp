#include<iostream>
#include<unordered_map>
#include<set>
#include<vector>
#include"defs.h"
#include"graph.h"

#ifdef __cplusplus
extern "C" {
#endif
#include"sqlite3.h"
#ifdef __cplusplus
}
#endif

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
    for (auto it = nodeMap->map.begin(); it != nodeMap->map.end(0); it++) {
        Node *n = it->second;
        list.push_back(n);
    }
    return list;
}