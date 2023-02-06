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
#define MAX_CLIENTS 2

char board[BOARD_SIZE][BOARD_SIZE];
int client_fds[MAX_CLIENTS];
int current_turn = 0;

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

void addMove(char playerSymbol, int x, int y) {
    board[x][y] = playerSymbol;
}

int winnerWinnerChickenDinner(char playerSymbol) {
    //Checking if a player win the game
    int i, j;

    //Check horizontal
    for (i = 0; i < BOARD_SIZE; i++) {
        for (j = 0; j < BOARD_SIZE - 4; j++) {
            if (board[i][j] == playerSymbol && board[i][j + 1] == playerSymbol && board[i][j + 2] == playerSymbol && board[i][j + 3] == playerSymbol && board[i][j + 4] == playerSymbol) {
                return 1;
            }
        }
    }

    //Check vertical
    for (i = 0; i < BOARD_SIZE - 4; i++) {
        for (j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == playerSymbol && board[i + 1][j] == playerSymbol && board[i + 2][j] == playerSymbol && board[i + 3][j] == playerSymbol && board[i + 4][j] == playerSymbol) {
                return 1;
            }
        }
    }

    //Check diagonal
    for (i = 0; i < BOARD_SIZE - 4; i++) {
        for (j = 0; j < BOARD_SIZE - 4; j++) {
            if (board[i][j] == playerSymbol && board[i + 1][j + 1] == playerSymbol && board[i + 2][j + 2] == playerSymbol && board[i + 3][j + 3] == playerSymbol && board[i + 4][j + 4] == playerSymbol) {
                return 1;
            }
        }
    }

    //Check anti-diagonal
    for (i = 0; i < BOARD_SIZE - 4; i++) {
        for (j = 4; j < BOARD_SIZE; j++) {
            if (board[i][j] == playerSymbol && board[i + 1][j - 1] == playerSymbol && board[i + 2][j - 2] == playerSymbol && board[i + 3][j - 3] == playerSymbol && board[i + 4][j - 4] == playerSymbol) {
                return 1;
            }
        }
    }

    return 0;
}

int main() {
    int server_fd, client_fd;
    socklen_t client_len;
    struct sockaddr_in server_addr, client_addr;
    int bytes_sent, bytes_received;
    char recv_data[BUFF_SIZE], send_data[BUFF_SIZE];
    char reply[MAX_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    memset((char *) &server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    listen(server_fd, 5);
    client_len = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_len);
    if (client_fd < 0) {
        perror("ERROR on accept");
        exit(1);
    }

    initCaroBoard();

    int n;
    char buffer[256];
    while (1) {
        memset(buffer, 0, 256);
        // n = read(client_fd, buffer, 255);
        bytes_received = recv(client_fd, recv_data, BUFF_SIZE - 1, 0);
        if (n < 0) {
            perror("ERROR reading from socket");
            exit(1);
        }
        if (bytes_received <= 0) {
            printf("\nConnection closed! END GAME!!!\n");
            break;
        }
        recv_data[bytes_received] = '\0';
        if (strcmp(recv_data, "ENDGAME") == 0) {
            sprintf(reply, "Goodbye Bae");
            bytes_sent = send(client_fd, reply, strlen(reply), 0);
        } else {
            printf("Received message: %s\n", recv_data);

            int x, y;
            sscanf(buffer, "%d %d", &x, &y);
            // board[x][y] = 'X';
            addMove('X', x, y);
        }
    }

    //Close socket and display message
    close(client_fd);
    close(server_fd);
    return 0;
}