#compiler used
CC = gcc

CFLAGS = -Wall -Wextra -Werror -Wshadow -Wformat=2

#executable name
EXECUTABLE = ndn

$(EXECUTABLE):
	@clear
	$(CC) $(CFLAGS) *.c -o $(EXECUTABLE)

clean:
	@clear
	rm $(EXECUTABLE)
