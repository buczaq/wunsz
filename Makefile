snek: snek.cpp
	gcc -o snek snek.cpp -lncurses -pthread -std=c++11 -lstdc++
	./snek
