CC=gcc
FLAGS=-Wall -O3

####
PROG=rp_gps
EXTRA=tinygps.c

all: $(PROG)

$(PROG): $(PROG).c $(EXTRA) 
	$(CC) $(FLAGS) -o $(PROG) $(PROG).c $(EXTRA) -lm 

clean:
	rm -f $(PROG)
