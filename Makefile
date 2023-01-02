CC = gcc

CFLAGS = -g -Wall -pthread

EXECUTABLES = web_server web_client writer

all: ${EXECUTABLES}

web_server: web_server.o connection.o llist.o
	${CC} ${CFLAGS} -o web_server web_server.o connection.o llist.o

web_client: web_client.o connection.o
	${CC} ${CFLAGS} -o web_client -pthread web_client.o connection.o

writer: writer.o 
	${CC} ${CFLAGS} -o writer writer.o

clean:
	rm -rf *.o ${EXECUTABLES} web_server.o connection.o llist.o writer.o
