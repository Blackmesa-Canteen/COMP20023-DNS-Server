# Created by Xiaotian Li on May 2, 2021
# A make file for Project 2
CC=clang
COPT=-Wall -Wpedantic -g

dns_svr: main.o message_handler.o my_response_handler.o network_handler.o utils.o
	$(CC) -o dns_svr main.o message_handler.o my_response_handler.o network_handler.o utils.o

main.o: main.c my_response_handler.h message_handler.h network_handler.h utils.h
	$(CC) -c -Wall main.c

my_response_handler.o: my_response_handler.c my_response_handler.h message_handler.h
	$(CC) -c -Wall my_response_handler.c

message_handler.o: message_handler.c message_handler.h
	$(CC) -c -Wall message_handler.c

network_handler.o: network_handler.c network_handler.h
	$(CC) -c -Wall network_handler.c

utils.o: utils.c utils.h
	$(CC) -c -Wall utils.c

%.o: %.c %.h
	$(CC) -c $< $(COPT) -g

format:
	clang-format -i *.c *.h

clean:
	rm -f dns_svr dns_svr.log *.o