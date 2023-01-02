#include "connection.h"
#include "llist.h"
#include <strings.h>
#include <pthread.h>

#define THREAD_POOL_SIZE 20

pthread_t thread_pool[THREAD_POOL_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t condition_v = PTHREAD_COND_INITIALIZER;  


//handle multiple threads
void *thread_function(void *arg){
    while (1){
        pthread_mutex_lock(&mutex) ; 
        int *pclient = dequeue();
        if (pclient == NULL ){
            pthread_cond_wait(&condition_v  , &mutex) ; 
            pclient = dequeue () ; 
        }
        pthread_mutex_unlock(&mutex ) ; 
        if (pclient != NULL){
            int conn_fd = *pclient ; 
            free(pclient)   ; 
            
            handle_request(conn_fd);
            close(conn_fd);
            printf("Closed the connection.\n");
        }
    }
}

int main(int argc, char *argv[])
{
    sem_init(&wrt,0,1);
    sem_init(&mutex1,0,1);
    int listen_fd, conn_fd;

    if ((listen_fd = open_server_connection()) < 0)
    {
        err_n_die("open_server_connection error.");
    }

    for (int i=0;i<THREAD_POOL_SIZE;i++){
        pthread_create(&thread_pool[i],NULL,thread_function,NULL);
    }

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        //put connection decsription ina linked list

        // accept blocks untill an incoming connection arrives
        // it returns a "file descriptor" to the connection.
        printf("waiting for a connection on port %d\n", SERVER_PORT);
        if ((conn_fd = accept(listen_fd, (SA *) &client_addr, (socklen_t *) &addr_len)) < 0) {
            err_n_die("accept error.");
        }
        printf("Connected!\n");
        fflush(stdout);

        int *pclient = malloc(sizeof(int));
        *pclient = conn_fd;
        pthread_mutex_lock(&mutex) ; 
        enqueue(pclient);
        pthread_cond_signal(&condition_v ) ; 
        pthread_mutex_unlock(&mutex); 
        
    }
}

