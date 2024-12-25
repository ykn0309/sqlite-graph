CXX = g++
CXXFLAGS = -g -fPIC
LDFLAGS = -shared

TARGET = graph.so

INCLUDEDIR = src/include

# 所有源文件
SRCS = src/sqlite-graph.cpp src/graph.cpp src/algorithm.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -I $(INCLUDEDIR) $^ -o $@

# 编译规则
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I $(INCLUDEDIR) -c $< -o $@

# 清理规则
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
