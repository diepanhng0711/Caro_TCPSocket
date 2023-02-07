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
#define PORT 5501
#define BUFF_SIZE 1024
#define MAX_SIZE 256
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
    int i;
    int server_fd, client_fd;
    socklen_t client_len;
    struct sockaddr_in server_addr, client_addr;
    int bytes_sent, bytes_received;
    char recv_data[BUFF_SIZE], send_data[BUFF_SIZE];
    char buffer[BUFF_SIZE];
    char reply[MAX_SIZE];
    int num_clients = 0;
    int winner = 0;

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

    int ret = listen(server_fd, 10);
    if (ret < 0) {
        perror("ERROR on listen");
        exit(1);
    }

    client_len = sizeof(client_addr);

    printf("Waiting for clients to connect...\n");
    while (num_clients < MAX_CLIENTS) {
        client_fds[num_clients] = accept(server_fd, (struct sockaddr *) &client_addr, &client_len);
        if (client_fds[num_clients] < 0) {
            perror("ERROR on accept");
            exit(1);
        }
        printf("Client %d connected\n", num_clients + 1);
        num_clients++;
    }

    //NOtify each client their symbol
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (i == 0) {
            sprintf(send_data, "X");
        } else {
            sprintf(send_data, "O");
        }
        bytes_sent = send(client_fds[i], send_data, strlen(send_data), 0);
        if (bytes_sent < 0) {
            perror("ERROR writing to socket");
            exit(1);
        }
    }

    initCaroBoard();
    memset(buffer, 0, BUFF_SIZE);
    memset(reply, 0, MAX_SIZE);
    memset(recv_data, 0, BUFF_SIZE);
    memset(send_data, 0, BUFF_SIZE);

    //Enter the game
    while(1) {
        int i;
        for (i = 0; i < MAX_CLIENTS; i++) {
            memset(buffer, 0, BUFF_SIZE);
            if (i == 0) {
                sprintf(buffer, "%d", 0);
                printf("X's turn\n");
            } else {
                sprintf(buffer, "%d", 1);
                printf("O's turn\n");
            }
            bytes_sent = send(client_fds[i], buffer, strlen(buffer), 0);
            if (bytes_sent < 0) {
                perror("ERROR writing to socket");
                exit(1);
            } else {
                printf("Sent message: %s\n", buffer);
            }

            memset(buffer, 0, BUFF_SIZE);
            recv(client_fds[i], buffer, BUFF_SIZE - 1, 0);
            recv_data[bytes_received] = '\0';

            int x, y;
            sscanf(buffer, "%d %d", &x, &y);
            if (i == 0) {
                addMove('X', x, y);
            } else {
                addMove('O', x, y);
            }
            printf("Received message: %s\n", buffer);
            printf("Board after move:\n");
            printCaroBoard();

            if (winnerWinnerChickenDinner('X')) {
                winner = 'X';
                break;
            } else if (winnerWinnerChickenDinner('O')) {
                winner = 'O';
                break;
            }
        }

        if (winner) {
            int send_winner = send(client_fds[winner], "WIN", strlen("WIN"), 0);
            if (send_winner < 0) {
                perror("ERROR writing to socket");
                exit(1);
            }
            int send_loser = send(client_fds[(winner + 1) % 2], "LOSE!", strlen("LOSE"), 0);
            if (send_loser < 0) {
                perror("ERROR writing to socket");
                exit(1);
            }
            break;
        }

        memset(recv_data, 0, BUFF_SIZE);
        memset(send_data, 0, BUFF_SIZE);
        memset(buffer, 0, BUFF_SIZE);
    }

    //Close socket and display message
    for (i = 0; i < MAX_CLIENTS; i++) {
        close(client_fds[i]);
    }
    close(server_fd);
    return 0;
}