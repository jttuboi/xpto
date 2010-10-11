all:
	gcc -o bin/b1 -Wall -Iinclude/ src/*.c

run:
	./bin/b1

clean:
	ls *~ bin/* include/*~ src/*~ .*.swp src/.*.swo src/.*.swp  include/.*.swp 2> /dev/null | xargs rm -rf 
