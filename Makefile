CXX = g++
CXXFLAGS = -std=c++14 -O3 -Wall -Wextra -Iinclude
SRCDIR = src
INCDIR = include
BINDIR = bin

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
TEST_SOURCES = tests/benchmark.cpp $(filter-out $(SRCDIR)/main.cpp, $(SOURCES))

TARGET = $(BINDIR)/urbantis.exe
TEST_TARGET = $(BINDIR)/test_suite.exe

all: $(TARGET) $(TEST_TARGET)

$(TARGET): $(OBJECTS) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TEST_TARGET): $(TEST_SOURCES) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -f $(SRCDIR)/*.o $(BINDIR)/*.exe

.PHONY: all clean
