bench=test3

all: stacksplitting.so

CXXFLAGS = -rdynamic $(shell llvm-config --cxxflags) -g -O0

stacksplitting.o: stacksplitting.cc

%.so: %.o
	$(CXX) -dylib -flat_namespace -shared $^ -o $@

run: stacksplitting.so
	opt -load ./stacksplitting.so -split-stack $(bench)-m2r.bc -o $(bench)-out.bc
	opt -std-compile-opts $(bench)-out.bc -o $(bench)-final.bc
	llvm-dis $(bench)-out.bc
	llc $(bench)-final.bc
	gcc $(bench)-final.s -o $(bench)-final

clean:
	rm -f *.o *~ *.so

debug:
	gdb --args opt -load ./stacksplitting.so -split-stack $(bench)-m2r.bc -o $(bench)-out.bc

.PHONY: clean all run
