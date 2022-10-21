TARGET := example
EXTENSION := cpp
CC := gcc

INCLUDEDIR := include
OBJDIR := obj
SRCDIR := source
BINDIR := bin
LIBDIR := lib
ASMDIR := asm

SRCFILES := $(shell find $(SRCDIR) -type f \( -iname "*.$(EXTENSION)" \) -exec basename \{} \;)
HEADERFILES := $(shell find $(INCLUDEDIR) -type f \( -iname "*.h" \) -exec basename \{} \;)
OBJFILES := $(SRCFILES:%.$(EXTENSION)=%.o)

MACROS := MAKE
BASEFLAGS := $(addprefix -D ,$(MACROS))
DEBUGFLAGS := $(BASEFLAGS) -g -Wall
RELEASEFLAGS := $(BASEFLAGS) -O2

FLAGS :=
A :=

test_all: test_data_run test_entry_run test_tree_run test_serial_run

debug: FLAGS = $(DEBUGFLAGS)
debug: clean setup $(TARGET)

release: FLAGS = $(RELEASEFLAGS)
release: clean setup $(TARGET)

#build target
$(TARGET): $(OBJFILES)
	$(CC) $(FLAGS) $(addprefix $(OBJDIR)/,$^) -o $(addprefix $(BINDIR)/,$@)


#compile object files
%.o: $(SRCDIR)/%.$(EXTENSION)
	$(CC) $(FLAGS) -o $(addprefix $(OBJDIR)/,$@) -c $^ -I $(INCLUDEDIR)
%.o: $(SRCDIR)/**/%.$(EXTENSION)
	$(CC) $(FLAGS) -o $(addprefix $(OBJDIR)/,$@) -c $^ -I $(INCLUDEDIR)


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


#print makefile info
info:
	@echo TARGET = $(TARGET)
	@echo EXTENSION = $(EXTENSION)
	@echo INCLUDEDIR = $(INCLUDEDIR)
	@echo OBJDIR = $(OBJDIR)
	@echo SRCDIR = $(SRCDIR)
	@echo BINDIR = $(BINDIR)
	@echo LIBDIR = $(LIBDIR)
	@echo ASMDIR = $(ASMDIR)
	@echo SRCFILES = $(SRCFILES)
	@echo HEADERFILES = $(HEADERFILES)
	@echo MACROS = $(MACROS)
	@echo DEBUGFLAGS = $(DEBUGFLAGS)
	@echo RELEASEFLAGS = $(RELEASEFLAGS)
	@echo CC = $(CC)


#give execution permissions
permissions:
	chmod a+x $(BINDIR)/$(TARGET)


#make assembly files
%.s: $(SRCDIR)/%.$(EXTENSION)
	$(CC) $(FLAGS) -o $(addprefix $(ASMDIR)/,$@) -S $^ -I $(INCLUDEDIR)
%.s: $(SRCDIR)/**/%.$(EXTENSION)
	$(CC) $(FLAGS) -o $(addprefix $(ASMDIR)/,$@) -S $^ -I $(INCLUDEDIR)


#create delete files
c:
	mkdir -p $(shell dirname $(SRCDIR)/$(A).$(EXTENSION))
	touch $(SRCDIR)/$(A).$(EXTENSION)
	mkdir -p $(shell dirname $(INCLUDEDIR)/$(A).$(EXTENSION))
	touch $(INCLUDEDIR)/$(A).h
r:
	rm $(SRCDIR)/$(A).$(EXTENSION)
	rm $(INCLUDEDIR)/$(A).h


#exec with std args
exec:
	@$(BINDIR)/$(TARGET) $$(cat args.txt)


#run valgrind
valgrind_data:
	valgrind --leak-check=full --track-origins=yes $(BINDIR)/test_data
valgrind_entry:
	valgrind --leak-check=full --track-origins=yes $(BINDIR)/test_entry
valgrind_tree:
	valgrind --leak-check=full --track-origins=yes $(BINDIR)/test_tree
valgrind_serial:
	valgrind --leak-check=full --track-origins=yes $(BINDIR)/test_serialization
valgrind_all: valgrind_data valgrind_entry valgrind_tree valgrind_serial


#get todo list
todo:
	@grep -R TODO -n | tr -s ' ' | grep -v makefile

data:
	$(CC) $(DEBUGFLAGS) -c $(SRCDIR)/data.c -o $(OBJDIR)/data.o -I $(INCLUDEDIR)

entry:
	$(CC) $(DEBUGFLAGS) -c $(SRCDIR)/entry.c -o $(OBJDIR)/entry.o -I $(INCLUDEDIR)

tree:
	$(CC) $(DEBUGFLAGS) -c $(SRCDIR)/tree.c -o $(OBJDIR)/tree.o -I $(INCLUDEDIR)

serialization:
	$(CC) $(DEBUGFLAGS) -c $(SRCDIR)/serialization.c -o $(OBJDIR)/serialization.o -I $(INCLUDEDIR)

proto:
	protoc --c_out=. sdmessage.proto
	mv sdmessage.pb-c.c source
	mv sdmessage.pb-c.h include
	$(CC) $(DEBUGFLAGS) -c $(SRCDIR)/sdmessage.pb-c.c -o $(OBJDIR)/sdmessage.pb-c.o -I $(INCLUDEDIR) -I/usr/include/ -I/usr/include/protobuf-c -L/usr/include -L/usr/include/protobuf-c -L/usr/lib -lprotobuf-c


tree_skel: tree
	$(CC) $(DEBUGFLAGS) -c $(SRCDIR)/tree_skel.c -o $(OBJDIR)/tree_skel.o -I $(INCLUDEDIR)

network_server: tree_skel
	$(CC) $(DEBUGFLAGS) -c $(SRCDIR)/network_server.c -o $(OBJDIR)/network_server.o -I $(INCLUDEDIR)
	$(CC) $(DEBUGFLAGS) $(SRCDIR)/network_server.c -o $(BINDIR)/network_server $(OBJDIR)/tree_skel.o $(OBJDIR)/sdmessage.pb-c.o $(OBJDIR)/tree.o $(OBJDIR)/entry.o $(OBJDIR)/data.o -I $(INCLUDEDIR) -I/usr/include/ -I/usr/include/protobuf-c -L/usr/include -L/usr/include/protobuf-c -L/usr/lib -lprotobuf-c

network_client:
	$(CC) $(DEBUGFLAGS) -o $(OBJDIR)/network_client.o -c $(SRCDIR)/network_client.c -I $(INCLUDEDIR)
	$(CC) $(DEBUGFLAGS) $(SRCDIR)/network_client.c -I $(INCLUDEDIR) -o network_client

tree_server: network_server
	$(CC) $(DEBUGFLAGS) $(SRCDIR)/tree_server.c -I $(INCLUDEDIR)

# Tests
test_data: data
	$(CC) $(DEBUGFLAGS) tests/test_data.c -o $(BINDIR)/test_data $(OBJDIR)/data.o -I $(INCLUDEDIR)

test_entry: data entry
	$(CC) $(DEBUGFLAGS) tests/test_entry.c -o $(BINDIR)/test_entry $(OBJDIR)/data.o $(OBJDIR)/entry.o -I $(INCLUDEDIR)

test_tree: data entry tree
	$(CC) $(DEBUGFLAGS) tests/test_tree.c -o $(BINDIR)/test_tree $(OBJDIR)/data.o $(OBJDIR)/entry.o $(OBJDIR)/tree.o -I $(INCLUDEDIR)

test_serial: data entry tree serialization
	$(CC) $(DEBUGFLAGS) tests/test_serialization.c -o $(BINDIR)/test_serialization $(OBJDIR)/data.o $(OBJDIR)/entry.o $(OBJDIR)/tree.o $(OBJDIR)/serialization.o -I $(INCLUDEDIR)

test_data_run: test_data
	./$(BINDIR)/test_data

test_entry_run: test_entry
	./$(BINDIR)/test_entry

test_tree_run: test_tree
	./$(BINDIR)/test_tree

test_serial_run: test_serial
	./$(BINDIR)/test_serialization





t_c:
	gcc -g -Wall source/tree_client.c -o tree_client
	./tree_client 1.2.3.4:69

