#compiler used
CC = gcc

CFLAGS = -Wall -Wextra -Werror -Wshadow -Wformat=2 -g

#executable name
EXECUTABLE = ndn

$(EXECUTABLE):
	@clear
	$(CC) $(CFLAGS) *.c -o $(EXECUTABLE)

clean:
	@clear
	rm $(EXECUTABLE)

EXE := ./ndn 192.168.1.7
memory0:
		clear
		@leaks -fullContent -atExit -- $(EXE) 50000

memory1:
		clear
		@leaks -fullContent -atExit -- $(EXE) 50001

memory2:
		clear
		@leaks -fullContent -atExit -- $(EXE) 50002

memory3:
		clear
		@leaks -fullContent -atExit -- $(EXE) 50003

memory4:
		clear
		@leaks -fullContent -atExit -- $(EXE) 50004

memory5:
		clear
		@leaks -fullContent -atExit -- $(EXE) 50005

memory6:
		clear
		@leaks -fullContent -atExit -- $(EXE) 50006

memory7:
		clear
		@leaks -fullContent -atExit -- $(EXE) 50007

memory8:
		clear
		@leaks -fullContent -atExit -- $(EXE) 50008

memory9:
		clear
		@leaks -fullContent -atExit -- $(EXE) 50009

