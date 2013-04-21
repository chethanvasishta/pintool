CC = g++

all: compile
	$(CC) obj/*.o -o pintool 
compile:
	find -name '*.cpp' -print0 | xargs -0 $(CC) -c $(CPPFLAGS) && mkdir -p obj && mv *.o ./obj/

#debug
debug: compiled
	$(CC) obj/*.o -o pintool 
compiled:
	find -name '*.cpp' -print0 | xargs -0 $(CC) -g -c $(CPPFLAGS) && mkdir -p obj && mv *.o ./obj/

#clean
clean:
	rm -rf obj pintool 
