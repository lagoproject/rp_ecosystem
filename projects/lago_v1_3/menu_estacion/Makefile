CC = gcc
EXEC = menu
OBJS = tinygps.o menu.o bmp180.o
LDFLAGS = -lm -lpthread -li2c

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

%.$(o) : %.c
	$(CC) -c $< -o $@

.PHONY: clean
clean:
	rm -rf *.o $(EXEC)


