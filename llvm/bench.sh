clang -O0 -emit-llvm -c test.c
opt -mem2reg test.bc -o test-m2r.bc
llvm-dis test-m2r.bc
clang -O0 -emit-llvm -c test2.c
opt -mem2reg test2.bc -o test2-m2r.bc
llvm-dis test2-m2r.bc
clang -O0 -emit-llvm -c test3.c
opt -mem2reg test3.bc -o test3-m2r.bc
llvm-dis test3-m2r.bc
