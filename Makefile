.PHONY: build

build: a.out
a.out: main.cpp
	$(CXX) $(CXXFLAGS) $<
