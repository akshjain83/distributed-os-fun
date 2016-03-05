gchase: gchase.cpp libmap.a goldchase.h
	g++ gchase.cpp -o gchase -L. -lmap -lpanel -lncurses -lpthread -lrt -g

libmap.a: Screen.o Map.o
	ar -r libmap.a Screen.o Map.o

Screen.o: Screen.cpp
	g++ -c Screen.cpp

Map.o: Map.cpp
	g++ -c Map.cpp

clean:
	rm -f Screen.o Map.o libmap.a gchase
