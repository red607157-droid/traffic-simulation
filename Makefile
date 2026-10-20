CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread -Iinclude -MMD -MP
SRC      = $(shell find src -name '*.cpp') main.cpp
OBJ      = $(SRC:%.cpp=build/%.o)
TARGET   = traffic-sim

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@

build/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Chạy test: make test  (biên dịch toàn bộ src/ + tests/*.cpp, không cần main.cpp)
test: test-vehicle test-trafficlight test-simulation test-infrastructure

test-vehicle:
	$(CXX) -std=c++17 -Wall -Wextra -pthread -Iinclude -fsanitize=address,undefined $(shell find src -name '*.cpp') tests/test_vehicle.cpp -o test-vehicle
	./test-vehicle

test-trafficlight:
	$(CXX) -std=c++17 -Wall -Wextra -pthread -Iinclude -fsanitize=address,undefined $(shell find src -name '*.cpp') tests/test_trafficlight.cpp -o test-trafficlight
	./test-trafficlight

test-simulation:
	$(CXX) -std=c++17 -Wall -Wextra -pthread -Iinclude -fsanitize=address,undefined $(shell find src -name '*.cpp') tests/test_simulation.cpp -o test-simulation
	./test-simulation

test-infrastructure:
	$(CXX) -std=c++17 -Wall -Wextra -pthread -Iinclude -fsanitize=address,undefined $(shell find src -name '*.cpp') tests/test_infrastructure.cpp -o test-infrastructure
	./test-infrastructure

# Phase 11: thí nghiệm đa luồng độc lập (không dùng code simulator)
concurrency:
	$(CXX) -std=c++17 -Wall -Wextra -pthread experiments/concurrency_basics.cpp -o concurrency-basics
	./concurrency-basics

# Phase 11 + ThreadSanitizer: báo chính xác dòng có data race (phần 2). Chỉ chạy được trên Linux/Mac, không có trên MinGW.
concurrency-tsan:
	$(CXX) -std=c++17 -g -pthread -fsanitize=thread experiments/concurrency_basics.cpp -o concurrency-tsan
	./concurrency-tsan

# Phase 12-13: chạy test simulation (có đa luồng) dưới ThreadSanitizer. Chỉ Linux/Mac. Xem docs/concurrency.md
test-simulation-tsan:
	$(CXX) -std=c++17 -g -pthread -Iinclude -fsanitize=thread $(shell find src -name '*.cpp') tests/test_simulation.cpp -o test-simulation-tsan
	./test-simulation-tsan

# Demo riêng của Simulation (không dùng main.cpp)
demo:
	$(CXX) -std=c++17 -Wall -Wextra -pthread -Iinclude $(shell find src -name '*.cpp') demo/simulation_demo.cpp -o simulation-demo
	./simulation-demo

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf build $(TARGET) test-vehicle test-vehicle.d test-trafficlight test-trafficlight.d test-simulation test-simulation.d test-infrastructure test-infrastructure.d concurrency-basics concurrency-tsan test-simulation-tsan simulation-demo

-include $(OBJ:.o=.d)

.PHONY: all run clean test test-vehicle test-trafficlight test-simulation test-infrastructure concurrency concurrency-tsan test-simulation-tsan demo