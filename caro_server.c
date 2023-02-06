#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<netdb.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/ioctl.h>
#include<sys/select.h> 
#include<sys/time.h>
#include<string.h>
#include<sys/stat.h>

//DEFINE ENVIROMENT VARIABLES FOR PROGRAMS
#define BOARD_SIZE 15
#define PORT 3434
#define BUFF_SIZE 1024
#define MAX_SIZE 1024

char board[BOARD_SIZE][BOARD_SIZE];

void initCaroBoard() {
    int i, j;
    for (i = 0; i < BOARD_SIZE; i++) {
        for (j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = ' ';
        }
    }
}

void printCaroBoard() {
    int i, j;
    printf("+ ");
    for (i = 0; i < BOARD_SIZE; i++) {
        printf("%c ", 97 + i); 
    }
    printf("+\n+");
    for (i = 0; i < BOARD_SIZE; i++) {
        printf("--");
    }
    printf("-+\n");
    for (i = 0; i < BOARD_SIZE; i++) {
        printf("||");
        for (j = 0; j < BOARD_SIZE; j++) {
            printf("%c|", board[i][j]);
        }
        printf("| %d\n+", i);
        for (j = 0; j < BOARD_SIZE; j++) {
            printf("%s", "--");
        }
        printf("-+\n");
        
    }

}

int main() {
    int sockfd, client_sockfd;
    socklen_t client_len;
    struct sockaddr_in server_addr, client_addr;
    int bytes_sent, bytes_received;
    char recv_data[BUFF_SIZE], send_data[BUFF_SIZE];
    char reply[MAX_SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    memset((char *) &server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    listen(sockfd, 5);
    client_len = sizeof(client_addr);
    client_sockfd = accept(sockfd, (struct sockaddr *) &client_addr, &client_len);
    if (client_sockfd < 0) {
        perror("ERROR on accept");
        exit(1);
    }

    initCaroBoard();
    printCaroBoard();

    int n;
    char buffer[256];
    while (1) {
        memset(buffer, 0, 256);
        // n = read(client_sockfd, buffer, 255);
        bytes_received = recv(client_sockfd, recv_data, BUFF_SIZE - 1, 0);
        if (n < 0) {
            perror("ERROR reading from socket");
            exit(1);
        }
        //Check if receive endgame message from client using recv
        // recv(client_sockfd, buffer, 255, MSG_PEEK);
        // if (strcmp(buffer, "ENDGAME") == 0) {
        //     break;
        // }
        if (bytes_received <= 0) {
            printf("\nConnection closed");
            break;
        }
        recv_data[bytes_received] = '\0';
        if (strcmp(recv_data, "ENDGAME") == 0) {
            sprintf(reply, "Goodbye Bae");
            bytes_sent = send(client_sockfd, reply, strlen(reply), 0);
        } else {
            printf("Received message: %s\n", buffer);

            int x, y;
            sscanf(buffer, "%d %d", &x, &y);
            board[x][y] = 'X';
        }

        // printCaroBoard();
    }

    //Close socket and display message
    printf("End game\n");
    close(client_sockfd);
    close(sockfd);
    return 0;
}