#compiler used
CC = gcc

CFLAGS = -Wall -std=c99 

#executable name
EXECUTABLE = ndn

$(EXECUTABLE):
	@clear
	$(CC) $(CFLAGS) *.c -o $(EXECUTABLE)

clean:
	@clear
	rm $(EXECUTABLE)

FORCE: