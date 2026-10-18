CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude -MMD -MP
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
test: test-vehicle test-simulation test-infrastructure

test-vehicle:
	$(CXX) -std=c++17 -Wall -Wextra -Iinclude -fsanitize=address,undefined $(shell find src -name '*.cpp') tests/test_vehicle.cpp -o test-vehicle
	./test-vehicle

test-simulation:
	$(CXX) -std=c++17 -Wall -Wextra -Iinclude -fsanitize=address,undefined $(shell find src -name '*.cpp') tests/test_simulation.cpp -o test-simulation
	./test-simulation

test-infrastructure:
	$(CXX) -std=c++17 -Wall -Wextra -Iinclude -fsanitize=address,undefined $(shell find src -name '*.cpp') tests/test_infrastructure.cpp -o test-infrastructure
	./test-infrastructure

# Phase 11: thí nghiệm đa luồng độc lập (không dùng code simulator)
concurrency:
	$(CXX) -std=c++17 -Wall -Wextra -pthread experiments/concurrency_basics.cpp -o concurrency-basics
	./concurrency-basics

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf build $(TARGET) test-vehicle test-vehicle.d test-simulation test-simulation.d test-infrastructure test-infrastructure.d concurrency-basics

-include $(OBJ:.o=.d)

.PHONY: all run clean test test-vehicle test-simulation test-infrastructure concurrency
