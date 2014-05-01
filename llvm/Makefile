bench=vanilla

all: stacksplitting.so

CXXFLAGS = -rdynamic $(shell llvm-config --cxxflags) -g -O0

stacksplitting.o: stacksplitting.cc

%.so: %.o
	$(CXX) -dylib -flat_namespace -shared $^ -o $@

run: stacksplitting.so $(bench).c
	clang -march=x86-64 -O0 -emit-llvm -c $(bench).c
	opt -mem2reg $(bench).bc -o $(bench)-m2r.bc
	#llvm-dis $(bench)-m2r.bc
	opt -load ./stacksplitting.so -split-stack $(bench)-m2r.bc -o $(bench)-out.bc
	opt -std-compile-opts $(bench)-out.bc -o $(bench)-split.bc
	opt -std-compile-opts $(bench)-m2r.bc -o $(bench)-ref.bc
	#llvm-dis $(bench)-out.bc
	#llc $(bench)-split.bc
	clang -march=x86-64 -o $(bench)-ref $(bench)-ref.bc
	clang -march=x86-64 -o $(bench)-split $(bench)-split.bc

clean:
	rm -f *.o *~ *.so

debug:
	gdb --args opt -load ./stacksplitting.so -split-stack $(bench)-m2r.bc -o $(bench)-out.bc

.PHONY: clean all run
