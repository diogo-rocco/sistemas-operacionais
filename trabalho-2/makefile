all: gerenciador.out 
	./gerenciador.out > output.txt

gerenciador.out: gerenciador.c
	gcc -Wall gerenciador.c -o gerenciador.out

clean:
	rm gerenciador.out test.out

test: test.out
	./test.out

test.out: test.c
	gcc -Wall test.c -o test.out