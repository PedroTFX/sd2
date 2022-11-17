TARGET := example
EXTENSION := c
CC := gcc

INCLUDEDIR := include
OBJDIR := object
SRCDIR := source
BINDIR := binary
LIBDIR := lib
ASMDIR := asm

SRCFILES := $(shell find $(SRCDIR) -type f \( -iname "*.$(EXTENSION)" \) -exec basename \{} \;)
HEADERFILES := $(shell find $(INCLUDEDIR) -type f \( -iname "*.h" \) -exec basename \{} \;)
OBJFILES := $(SRCFILES:%.$(EXTENSION)=%.o)

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
	$(CC) $(DEBUGFLAGS) -c $(SRCDIR)/network_client.c -o $(OBJDIR)/network_client.o -I $(INCLUDEDIR)

client_stub.o:
	$(CC) $(DEBUGFLAGS) -c $(SRCDIR)/client_stub.c -o $(OBJDIR)/client_stub.o -I $(INCLUDEDIR)

client-lib.o: data.o entry.o sdmessage.pb-c.o util.o network_client.o client_stub.o
	ld -r $(OBJDIR)/data.o $(OBJDIR)/entry.o $(OBJDIR)/sdmessage.pb-c.o $(OBJDIR)/util.o $(OBJDIR)/network_client.o $(OBJDIR)/client_stub.o -o $(LIBDIR)/client-lib.o -I $(INCLUDEDIR)

tree-client: client-lib.o
	$(CC) $(DEBUGFLAGS) $(SRCDIR)/tree_client.c -o $(BINDIR)/tree-client $(LIBDIR)/client-lib.o -I $(INCLUDEDIR) -I/usr/include/ -L/usr/include -lprotobuf-c -lpthread

ccclient_run: tree-client
	./$(BINDIR)/tree-client 127.0.0.1:1337

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
	$(CC) $(DEBUGFLAGS) -c $(SRCDIR)/tree_skel.c -o $(OBJDIR)/tree_skel.o -I $(INCLUDEDIR)

tree-server: data.o entry.o tree.o sdmessage.pb-c.o util.o network_server.o tree_skel.o
	$(CC) $(DEBUGFLAGS) $(SRCDIR)/tree_server.c -o $(BINDIR)/tree-server $(OBJDIR)/tree_skel.o $(OBJDIR)/network_server.o $(OBJDIR)/util.o $(OBJDIR)/sdmessage.pb-c.o $(OBJDIR)/tree.o $(OBJDIR)/entry.o $(OBJDIR)/data.o -I $(INCLUDEDIR) -I/usr/include/ -L/usr/include -lprotobuf-c -lpthread

ssserver_run: tree-server
	./$(BINDIR)/tree-server 1337 5

sserver_valgrind: tree-server
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all $(BINDIR)/tree-server 1337 3
