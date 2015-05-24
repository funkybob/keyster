
PROG=keyster

all: $(PROG)

clean:
	rm $(PROG)

$(PROG): src/main.c
	cc -Ofast -I sglib-1.0.4/ -lc -luv -o $(PROG) src/main.c
