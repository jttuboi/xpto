all:
	gcc -o bin/b1 -Wall -Iinclude/ src/*.c

run:
	./bin/b1

clean:
	rm *~ bin/* include/*~ src/*~ 2> /dev/null
