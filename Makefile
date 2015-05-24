
PROG=keyster

all: $(PROG)

$(PROG): src/main.c
	cc -O3 -I sglib-1.0.4/ -lc -luv -o $(PROG) src/main.c
