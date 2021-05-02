# Created by Xiaotian Li on May 2, 2021
# A make file for Project 2
dns_svr: main.o message_handler.o my_response_handler.o network_handler.o utils.o
	gcc -o dns_svr main.o message_handler.o my_response_handler.o network_handler.o utils.o

main.o: main.c my_response_handler.h message_handler.h network_handler.h utils.h
	gcc -c -Wall main.c

my_response_handler.o: my_response_handler.c my_response_handler.h message_handler.h
	gcc -c -Wall my_response_handler.c

message_handler.o: message_handler.c message_handler.h
	gcc -c -Wall message_handler.c

network_handler.o: network_handler.c network_handler.h
	gcc -c -Wall network_handler.c

utils.o: utils.c utils.h
	gcc -c -Wall utils.c

clean:
	rm -f dns_svr dns_svr.log *.o