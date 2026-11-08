CXX := g++
CPPFLAGS := $(shell pkg-config --cflags opencv4)
LDLIBS := $(shell pkg-config --libs opencv4) -pthread
COMMON_FLAGS := -std=c++17 -Wall -Wextra -Wpedantic -pthread
REF_FLAGS := $(COMMON_FLAGS) -O0
OPT_FLAGS := $(COMMON_FLAGS) -O3 -ftree-vectorize

.PHONY: all clean
all: sobel_original sobel_optimized validate

sobel_original: benchmark_original.o reference.o common.o input.o
	$(CXX) $^ -o $@ $(LDLIBS)

sobel_optimized: benchmark_optimized.o optimized.o common.o input.o
	$(CXX) $^ -o $@ $(LDLIBS)

validate: validate.o reference.o optimized.o common.o input.o
	$(CXX) $^ -o $@ $(LDLIBS)

benchmark_original.o: benchmark.cpp image_processing.h input.h
	$(CXX) $(CPPFLAGS) $(REF_FLAGS) -DORIGINAL_BUILD -c $< -o $@

benchmark_optimized.o: benchmark.cpp image_processing.h input.h
	$(CXX) $(CPPFLAGS) $(OPT_FLAGS) -c $< -o $@

reference.o: reference.cpp image_processing.h
	$(CXX) $(CPPFLAGS) $(REF_FLAGS) -c $< -o $@

optimized.o: optimized.cpp image_processing.h
	$(CXX) $(CPPFLAGS) $(OPT_FLAGS) -c $< -o $@

common.o: common.cpp image_processing.h
	$(CXX) $(CPPFLAGS) $(OPT_FLAGS) -c $< -o $@

input.o: input.cpp input.h
	$(CXX) $(CPPFLAGS) $(OPT_FLAGS) -c $< -o $@

validate.o: validate.cpp image_processing.h input.h
	$(CXX) $(CPPFLAGS) $(OPT_FLAGS) -c $< -o $@

clean:
	rm -f *.o sobel_original sobel_optimized validate
