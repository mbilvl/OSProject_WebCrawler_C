#makefile for our crawler program
#variables
CC=gcc
CFLAG=-lcurl -ltidy -lpthread -Wall -I.

#tagets: Dependencies
	#actions
all: crawler
crawler: main.o bootup.o spider.o crawler.o crawl_frontier.o hash.o queue.o list.o state.o
	$(CC) -g main.o bootup.o spider.o crawler.o crawl_frontier.o hash.o queue.o list.o state.o -o crawler $(CFLAG) 
main.o: main.c
	$(CC) -g -c main.c
bootup.o: bootup.c
	$(CC) -g -c bootup.c
spider.o: spider.c
	$(CC) -g -c spider.c
crawler.o: crawler.c
	$(CC) -g -c crawler.c
crawl_frontier.o: crawl_frontier.c
	$(CC) -g -c crawl_frontier.c
hash.o: hash.c
	$(CC) -g -c hash.c
queue.o: queue.c
	$(CC) -g -c queue.c
list.o: list.c
	$(CC) -g -c list.c
state.o: state.c
	$(CC) -g -c state.c
#clean all the object files and crawler executable file.
clean:
	rm -f *.o crawler
