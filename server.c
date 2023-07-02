#include <stdio.h>     
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>   
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MAXPENDING 8
#define MAX_BOOKS 8
#define MAX_SHELVES 8
#define MAX_ROWS 8
#define BUFFER_SIZE 64

pthread_mutex_t mutex;
int* catalog;
int book_ptr = 0;
int connections = 0;
int rows_count;
int shelves_count;
int books_count;

typedef struct thread_args {
    int socket;
} thread_args;

void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}


void *handleClient(void *args) {
    struct sockaddr_in client_addr;
    int client_length;
    int socket;
    pthread_detach(pthread_self());
    socket = ((thread_args*)args)->socket;
    free(args);

    for (;;) {
        int student_id = 0;
        int buffer[BUFFER_SIZE + 2];
        client_length = sizeof(client_addr);
        recvfrom(socket, buffer, sizeof(buffer), 0, (struct sockaddr*) &client_addr, &client_length);
        pthread_mutex_lock(&mutex);
        connections++;
        student_id = buffer[0];
        int data_len = buffer[1];
        for (int i = 2; i < data_len + 2; i++) {
            catalog[book_ptr++] = buffer[i];
        }
        printf("Got sub-catalog from student #%d\n", student_id);
        if (connections >= rows_count) {
            printf("Finished catalog:\n");
            for (int i = 0; i < rows_count * shelves_count * books_count; i++) {
                printf("Book: %d\n", catalog[i]);
            }
        }
        pthread_mutex_unlock(&mutex);
    }
        
}

int createSocket(unsigned short server_port) {
    int server_socket;
    struct sockaddr_in server_addr;

    if ((server_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) DieWithError("socket() failed");
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;              
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    server_addr.sin_port = htons(server_port);

    if (bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) DieWithError("bind() failed");
    printf("Open socket on %s:%d\n", inet_ntoa(server_addr.sin_addr), server_port);
    return server_socket;
}

int main(int argc, char *argv[])
{
    unsigned short student_port;
    int server_socket;
    
    pthread_t threadId;
    pthread_mutex_init(&mutex, NULL);
    if (argc != 5)
    {
        fprintf(stderr, "Usage:  %s <Port for students> <rows count> <shelves count> <books count>\n", argv[0]);
        exit(1);
    }

    student_port = atoi(argv[1]);
    rows_count = atoi(argv[2]);
    shelves_count = atoi(argv[3]);
    books_count = atoi(argv[4]);
    if (rows_count > MAX_ROWS) {
        fprintf(stderr, "Max rows: %d\n", MAX_ROWS);
        exit(1);
    }
    if (shelves_count > MAX_SHELVES) {
        fprintf(stderr, "Max shelves: %d\n", MAX_SHELVES);
        exit(1);
    }
    if (books_count > MAX_BOOKS) {
        fprintf(stderr, "Max books: %d\n", MAX_BOOKS);
        exit(1);
    }

    catalog = (int*)malloc(sizeof(int) * rows_count * shelves_count * books_count);

    server_socket = createSocket(student_port);
    thread_args *args = (thread_args*) malloc(sizeof(thread_args));
    args->socket = server_socket;
    if (pthread_create(&threadId, NULL, handleClient, (void*) args) != 0) DieWithError("pthread_create() failed");
    for (;;) {
        sleep(1);
    }
    free(catalog);
    pthread_mutex_destroy(&mutex);
    return 0;
}