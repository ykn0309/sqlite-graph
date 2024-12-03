#include"graph.h"
#ifdef __cplusplus
extern "C" {
#endif
#include"sqlite3.h"
#ifdef __cplusplus
}
#endif

class GraphManager {
private:
    Graph *graph;
    // 把构造函数私有，保证只有一个实例
    GraphManager() {}

public:
    static GraphManager& getGraphManager() {
        static GraphManager graphManager;
        return graphManager;
    }

    // 删除拷贝构造函数和复制构造函数
    GraphManager(const GraphManager&) = delete;
    GraphManager& operator=(const GraphManager&) = delete;

    ~GraphManager() {
        delete graph;
    }

    Graph* newGraph(sqlite3 *db, BindingInfo *bi) {
        if (graph != nullptr) {
            delete graph;
        }
        graph = new Graph(db, bi);
    }
};