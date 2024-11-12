#include"graph.h"

class GraphManager {
private:
    // Construct function is private to avoid create an instance outside.
    GraphManager() {}

public:
    static GraphManager& getGraphManager() {
        static GraphManager graphManager;
        return graphManager;
    }

    GraphManager(const GraphManager&) = delete;
    GraphManager& operator=(const GraphManager&) = delete;
};