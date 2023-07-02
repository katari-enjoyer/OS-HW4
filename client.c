#include <stdio.h>   
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_BOOKS 8
#define MAX_SHELVES 8
#define MAX_ROWS 8
#define BUFFER_SIZE 64

void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

int main(int argc, char *argv[])
{
    int index;
    int client_socket;
    struct sockaddr_in server_addr;
    unsigned short server_port;
    int recv_msg_size;
    char *server_ip;
    int student_row;
    int shelves_count;
    int books_count;

    if (argc != 6)
    {
       fprintf(stderr, "Usage: %s <Server IP> <Server Port> <Student row> <Shelves count> <books count>\n", argv[0]);
       exit(1);
    }

    server_ip = argv[1];
    server_port = atoi(argv[2]);
    student_row = atoi(argv[3]);
    shelves_count = atoi(argv[4]);
    books_count = atoi(argv[5]);
    if (shelves_count > MAX_SHELVES) {
        fprintf(stderr, "Max shelves: %d\n", MAX_SHELVES);
        exit(1);
    }
    if (books_count > MAX_BOOKS) {
        fprintf(stderr, "Max books: %d\n", MAX_BOOKS);
        exit(1);
    }

    if ((client_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) DieWithError("socket() failed");
    memset(&server_addr, 0, sizeof(server_addr));  
    server_addr.sin_family      = AF_INET;        
    server_addr.sin_addr.s_addr = inet_addr(server_ip); 
    server_addr.sin_port        = htons(server_port);

    int buffer[BUFFER_SIZE + 2];
    buffer[0] = student_row;
    int data_len = shelves_count * books_count;
    buffer[1] = data_len;
    printf("Sorting row %d\n", student_row);
    int ptr = 2;
    for (int i = 0; i < shelves_count; i++) {
        for (int j = 0; j < books_count; j++) {
            buffer[ptr++] = 100*student_row + 10 * i + j;
        }
    }
    printf("My Sub-catalog:\n");
    for (int i = 2; i < ptr; i++) {
        printf("Book #%d\n", buffer[i]);
    }
    sendto(client_socket, buffer, sizeof(buffer), 0, (struct sockaddr*) &server_addr, sizeof(server_addr));
    close(client_socket);
    return 0;
}