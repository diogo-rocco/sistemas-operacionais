all: escalonador.out 
	./escalonador.out 

escalonador.out: escalonador.c
	gcc -Wall escalonador.c -o escalonador.out

clean:
	rm escalonador.out test.out

test: test.out
	./test.out

test.out: test.c
	gcc -Wall test.c -o test.out