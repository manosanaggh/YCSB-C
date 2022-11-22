CC=g++
CFLAGS=-std=c++17 -g -Wall -ldl -I./
LDFLAGS= -pthread -ltbb -lrt
SUBDIRS=core db
SUBSRCS=$(wildcard core/*.cc) $(wildcard db/*.cc) cs647/assignment3/ti.cpp cs647/assignment3/ti_api.cpp cs647/assignment3/tb.cpp cs647/assignment3/tb_api.cpp cs647/assignment3/tl_api.cpp cs647/assignment3/tl.cpp
OBJECTS=$(SUBSRCS:.cc=.o)
EXEC=ycsbc

all: $(SUBDIRS) $(EXEC)

$(SUBDIRS):
	$(MAKE) -C $@

$(EXEC): $(wildcard *.cc) $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) ../rocksdb/librocksdb.a /lib64/libz.so redis/hiredis/libhiredis.a -o $@

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done
	$(RM) $(EXEC)

.PHONY: $(SUBDIRS) $(EXEC)

