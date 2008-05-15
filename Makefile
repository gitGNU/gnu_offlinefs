LDFLAGS= -ldb_cxx -lfuse
CXXFLAGS= -O0 -Wall -g

offlinefs: fs.o main.o database.o fsdb.o fsnodes/node.o fsnodes/file.o fsnodes/directory.o fsnodes/symlink.o medium.o source.o util.o
	g++ $(LDFLAGS) -o $@ $^

offmedia: tools/offmedia.o database.o
	g++ $(LDFLAGS) -o $@ $^

%.o: %.cxx
	g++ $(CXXFLAGS) -o $@ -c $<
