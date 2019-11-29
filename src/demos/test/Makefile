LIBS := -L /shared/projects/proto/lib -lproto
.PHONY: all
all: libtest.so Makefile

libtest.so: test.cc
	clang++ -fPIC -std=c++17 -c -o test.o test.cc -I /shared/projects/proto/src
	clang++ -fPIC -shared -o libtest.so test.o $(LIBS)

.PHONY: clean
clean:
	rm test.so test.o
