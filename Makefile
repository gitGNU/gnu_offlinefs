LDFLAGS= -ldb_cxx -lfuse
CXXFLAGS= -Wall -g


offlinefs:	main.o fsnodes.o fs.o database.o
	g++ $(LDFLAGS) -o $@ $^

%.o: %.cxx
	g++ $(CXXFLAGS) -c $<
