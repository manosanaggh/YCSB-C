CC=g++
CFLAGS=-std=c++17 -g -Wall -I./
LDFLAGS= -pthread -ltbb -lhiredis -lrt
SUBDIRS=core db redis
SUBSRCS=$(wildcard core/*.cc) $(wildcard db/*.cc) cs647/assignment3/ti.cpp cs647/assignment3/ti_api.cpp cs647/assignment3/tb.cpp cs647/assignment3/tb_api.cpp cs647/assignment3/tl_api.cpp cs647/assignment3/tl.cpp
OBJECTS=$(SUBSRCS:.cc=.o)
EXEC=ycsbc

all: $(SUBDIRS) $(EXEC)

$(SUBDIRS):
	$(MAKE) -C $@

$(EXEC): $(wildcard *.cc) $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) ../rocksdb/librocksdb.a /lib/x86_64-linux-gnu/libz.so -o $@

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done
	$(RM) $(EXEC)

.PHONY: $(SUBDIRS) $(EXEC)

