#compiler used
CC = gcc

CFLAGS = -Wall

#executable name
EXECUTABLE = ndn

$(EXECUTABLE):
	@clear
	$(CC) $(CFLAGS) *.c -o $(EXECUTABLE)

clean:
	@clear
	rm $(EXECUTABLE)