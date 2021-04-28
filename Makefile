#compiler used
CC = gcc

CFLAGS = -Wall -Wextra -Wshadow -Werror -Wformat=2 -g

#executable name
EXECUTABLE = ndn

$(EXECUTABLE):
	@clear
	$(CC) $(CFLAGS) *.c -o $(EXECUTABLE)

clean:
	@clear
	rm $(EXECUTABLE) *.txt

EXE_MAC := ./ndn 192.168.1.14
memory0:
		clear
		@leaks -fullContent -atExit -- $(EXE_MAC) 50000

memory1:
		clear
		@leaks -fullContent -atExit -- $(EXE_MAC) 50001

memory2:
		clear
		@leaks -fullContent -atExit -- $(EXE_MAC) 50002

memory3:
		clear
		@leaks -fullContent -atExit -- $(EXE_MAC) 50003

memory4:
		clear
		@leaks -fullContent -atExit -- $(EXE_MAC) 50004

memory5:
		clear
		@leaks -fullContent -atExit -- $(EXE_MAC) 50005

memory6:
		clear
		@leaks -fullContent -atExit -- $(EXE_MAC) 50006

memory7:
		clear
		@leaks -fullContent -atExit -- $(EXE_MAC) 50007

memory8:
		clear
		@leaks -fullContent -atExit -- $(EXE_MAC) 50008

memory9:
		clear
		@leaks -fullContent -atExit -- $(EXE_MAC) 50009

EXE_WINDOWS := ./ndn 10.0.2.15

valgrind0:
		clear
		valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrinf0-out.txt $(EXE_WINDOWS) 50000

valgrind1:
		clear
		valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrinf1-out.txt $(EXE_WINDOWS) 50001 

valgrind2:
		clear
		valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrinf2-out.txt $(EXE_WINDOWS) 50002

valgrind3:
		clear
		valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrinf3-out.txt $(EXE_WINDOWS) 50003 

valgrind4:
		clear
		valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrinf4-out.txt $(EXE_WINDOWS) 50004 

valgrind5:
		clear
		valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrinf5-out.txt $(EXE_WINDOWS) 50005 

valgrind6:
		clear
		valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrinf6-out.txt $(EXE_WINDOWS) 50006

valgrind7:
		clear
		valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrinf7-out.txt $(EXE_WINDOWS) 50007

valgrind8:
		clear
		valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrinf8-out.txt $(EXE_WINDOWS) 50008

valgrind9:
		clear
		valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrinf9-out.txt $(EXE_WINDOWS) 50009