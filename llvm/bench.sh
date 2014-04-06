clang -O0 -emit-llvm -c test.c
opt -mem2reg test.bc -o test-m2r.bc
llvm-dis test-m2r.bc
