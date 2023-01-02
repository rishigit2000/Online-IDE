#ifndef _COMMON_H_
#define _COMMON_H_

// basic socket definitions
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>

// for variadic function
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#define SERVER_PORT 18000

#define MAXLINE 8192
#define MAXSIZE 50
#define SA struct sockaddr

sem_t wrt,mutex1;

void err_n_die(const char *fmt, ...);
ssize_t readline(int fd, void *buf, size_t maxlen);
int open_server_connection();
int open_client_connection(char *hostname, int port);
void handle_request(int fd);
void *thread_function(void *arg);
void read_all(int fd);

#endif
