TARGET := example
EXTENSION := c
CC := gcc

INCLUDEDIR := include
LIBINCLUDEDIR := /usr/include/zookeeper
OBJDIR := object
SRCDIR := source
BINDIR := binary
LIBDIR := lib
ASMDIR := asm

MACROS := MAKE
BASEFLAGS := $(addprefix -D ,$(MACROS))
DEBUGFLAGS := $(BASEFLAGS) -g -Wall
RELEASEFLAGS := $(BASEFLAGS) -O2

run: clean setup tree-server tree-client

#clean directory
clean:
	rm -rf $(OBJDIR)
	rm -rf $(BINDIR)
	rm -rf $(LIBDIR)

#setup directory
setup:
	mkdir -p $(OBJDIR)
	mkdir -p $(BINDIR)
	mkdir -p $(LIBDIR)

# Shared
data.o:
	$(CC) $(DEBUGFLAGS) -c $(SRCDIR)/data.c -o $(OBJDIR)/data.o -I $(INCLUDEDIR)

entry.o:
	$(CC) $(DEBUGFLAGS) -c $(SRCDIR)/entry.c -o $(OBJDIR)/entry.o -I $(INCLUDEDIR)

sdmessage.pb-c.o:
	protoc --c_out=. sdmessage.proto
	mv sdmessage.pb-c.c source
	mv sdmessage.pb-c.h include
	$(CC) $(DEBUGFLAGS) -c $(SRCDIR)/sdmessage.pb-c.c -o $(OBJDIR)/sdmessage.pb-c.o -I $(INCLUDEDIR)

util.o:
	$(CC) $(DEBUGFLAGS) -c $(SRCDIR)/util.c -o $(OBJDIR)/util.o -I $(INCLUDEDIR)

# Client


network_client.o:
	$(CC) $(DEBUGFLAGS) -c $(SRCDIR)/network_client.c -o $(OBJDIR)/network_client.o -I $(INCLUDEDIR) -I $(LIBINCLUDEDIR)

client_zookeeper.o:
	$(CC) $(DEBUGFLAGS) -c $(SRCDIR)/client_zookeeper.c -o $(OBJDIR)/client_zookeeper.o -I $(INCLUDEDIR) -I $(LIBINCLUDEDIR)

client_stub.o:
	$(CC) $(DEBUGFLAGS) -c $(SRCDIR)/client_stub.c -o $(OBJDIR)/client_stub.o -I $(INCLUDEDIR) -I $(LIBINCLUDEDIR)

client-lib.o: data.o entry.o sdmessage.pb-c.o util.o network_client.o client_zookeeper.o client_stub.o sort.o
	ld -r $(OBJDIR)/data.o $(OBJDIR)/entry.o $(OBJDIR)/sdmessage.pb-c.o $(OBJDIR)/util.o $(OBJDIR)/network_client.o $(OBJDIR)/client_zookeeper.o $(OBJDIR)/client_stub.o $(OBJDIR)/bubble_sort.o -o $(LIBDIR)/client-lib.o -I $(INCLUDEDIR) -I $(LIBINCLUDEDIR)

tree-client: client-lib.o
	$(CC) $(DEBUGFLAGS) $(SRCDIR)/tree_client.c -o $(BINDIR)/tree-client $(LIBDIR)/client-lib.o -I $(INCLUDEDIR) -I $(LIBINCLUDEDIR) -I/usr/include/ -L/usr/include -lprotobuf-c -lpthread -lzookeeper_mt

ccclient_run: tree-client
	./$(BINDIR)/tree-client 127.0.0.1:2181

f1cclient_run: tree-client
	./$(BINDIR)/tree-client 127.0.0.1:1337 < tests/del01.txt

f2cclient_run: tree-client
	./$(BINDIR)/tree-client 127.0.0.1:1337 < tests/del02.txt

f3cclient_run: tree-client
	./$(BINDIR)/tree-client 127.0.0.1:1337 < tests/del03.txt

f4cclient_run: tree-client
	./$(BINDIR)/tree-client 127.0.0.1:1337 < tests/del01-expected.txt

f5cclient_run: tree-client
	./$(BINDIR)/tree-client 127.0.0.1:1337 < tests/del01-put.txt

f6cclient_run: tree-client
	./$(BINDIR)/tree-client 127.0.0.1:1337 < tests/del01-del.txt

cclient_valgrind: tree-client
	valgrind --leak-check=full --track-origins=yes $(BINDIR)/tree-client 127.0.0.1:1337 < ./tests/del01-del.txt

2cclient_valgrind: tree-client
	valgrind --leak-check=full --track-origins=yes $(BINDIR)/tree-client 127.0.0.1:1337 < ./tests/del01.txt

3cclient_valgrind: tree-client
	valgrind --leak-check=full --track-origins=yes $(BINDIR)/tree-client 127.0.0.1:1337

# Server
tree.o:
	$(CC) $(DEBUGFLAGS) -c $(SRCDIR)/tree.c -o $(OBJDIR)/tree.o -I $(INCLUDEDIR)

network_server.o:
	$(CC) $(DEBUGFLAGS) -c $(SRCDIR)/network_server.c -o $(OBJDIR)/network_server.o -I $(INCLUDEDIR)

tree_skel.o:
	$(CC) $(DEBUGFLAGS) -c $(SRCDIR)/tree_skel.c -o $(OBJDIR)/tree_skel.o -I $(INCLUDEDIR) -I $(LIBINCLUDEDIR)

tree-server: tree.o network_server.o tree_skel.o client-lib.o
	$(CC) $(DEBUGFLAGS) $(SRCDIR)/zookeeper.c $(SRCDIR)/tree_server.c -o $(BINDIR)/tree-server $(OBJDIR)/tree.o $(OBJDIR)/network_server.o $(OBJDIR)/tree_skel.o $(LIBDIR)/client-lib.o -I $(INCLUDEDIR) -I/usr/include/ -I$(LIBINCLUDEDIR) -L/usr/include -lprotobuf-c -lpthread -lzookeeper_mt

sss1erver_run: tree-server
	./$(BINDIR)/tree-server 1337 127.0.0.1:2181
sss2erver_run: tree-server
	./$(BINDIR)/tree-server 1338 127.0.0.1:2181
sss3erver_run: tree-server
	./$(BINDIR)/tree-server 1339 127.0.0.1:2181
sss4erver_run: tree-server
	./$(BINDIR)/tree-server 1340 127.0.0.1:2181

ssserver_run2: tree-server
	./$(BINDIR)/tree-server $(args) 127.0.0.1:2181

sserver_valgrind: tree-server
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all $(BINDIR)/tree-server 1337 3

zoo: compile_zoo

compile_zoo:
	gcc ./ZooKeeper/examples/zoo.c -o ./ZooKeeper/examples/zoo -lzookeeper_mt
	gcc ./ZooKeeper/examples/zchildmaker.c -o ./ZooKeeper/examples/zchildmaker -lzookeeper_mt
	gcc ./ZooKeeper/examples/zchildwatcher.c -o ./ZooKeeper/examples/zchildwatcher -lzookeeper_mt
	gcc ./ZooKeeper/examples/zdataupdater.c -o ./ZooKeeper/examples/zdataupdater -lzookeeper_mt
	gcc ./ZooKeeper/examples/zdataupdater.c -o ./ZooKeeper/examples/zdataupdater -lzookeeper_mt
	gcc ./ZooKeeper/examples/zdatawatcher.c -o ./ZooKeeper/examples/zdatawatcher -lzookeeper_mt

zoo_cw:
	./ZooKeeper/examples/zchildwatcher 127.0.0.1:2181

zoo_dw:
	./ZooKeeper/examples/zdatawatcher 127.0.0.1:2181

zoo_cm:
	./ZooKeeper/examples/zchildmaker 127.0.0.1:2181

zoo_du:
	./ZooKeeper/examples/zdataupdater 127.0.0.1:2181

zoo_z:
	./ZooKeeper/examples/zoo 127.0.0.1:2181


sort.o:
	$(CC) $(DEBUGFLAGS) -c $(SRCDIR)/bubble_sort.c -o $(OBJDIR)/bubble_sort.o -I $(INCLUDEDIR) -I$(LIBINCLUDEDIR) -lzookeeper_mt


chmod:
	chmod +x ./ZooKeeper/bin/*
