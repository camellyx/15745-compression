all: stacksplitting.so

CXXFLAGS = -rdynamic $(shell llvm-config --cxxflags) -g -O0

stacksplitting.o: stacksplitting.cc

%.so: %.o
	$(CXX) -dylib -flat_namespace -shared $^ -o $@

run: stacksplitting.so
	opt -load ./stacksplitting.so -split-stack test-m2r.bc

clean:
	rm -f *.o *~ *.so

debug:
	gdb --args opt -load ./stacksplitting.so -split-stack test-m2r.bc

.PHONY: clean all run
