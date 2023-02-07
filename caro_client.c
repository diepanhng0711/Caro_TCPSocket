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
#define SERVER_ADDR "127.0.0.1"
#define PORT 5501
#define BUFF_SIZE 1024
#define MAX_SIZE 256

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

void addMove(char playerSymbol, int x, int y) {
    board[x][y] = playerSymbol;
}

int main() {
    int client_fd;
    struct sockaddr_in server_addr;
    int bytes_sent, bytes_received;
    char recv_data[BUFF_SIZE], send_data[BUFF_SIZE];
    char reply[MAX_SIZE];
    int playerPosition;

    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    memset((char *) &server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr) <= 0) {
        perror("ERROR inet_pton");
        exit(1);
    }

    if (connect(client_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("ERROR connecting");
        exit(1);
    } else {
        printf("Connected to server %s at port %d ...\n", SERVER_ADDR, PORT);
    }

    //Receive symbol of player
    bytes_received = recv(client_fd, recv_data, sizeof(recv_data), 0);
    char playerSymbol = recv_data[0];
    printf("You are player %c\n\n", playerSymbol);

    if (playerSymbol == 'X') {
        playerPosition = 0;
    } else {
        playerPosition = 1;
    }

    //init the caro board for the game
    initCaroBoard();
    printCaroBoard();

    int x, y;
    char c;
    while (1) {
        memset(recv_data, 0, BUFF_SIZE);
        memset(send_data, 0, BUFF_SIZE);

        //Wait for the server to notify the player's turn
        bytes_received = recv(client_fd, recv_data, sizeof(recv_data), 0);
        printf("check %s\n", recv_data);
        if (bytes_received <= 0) {
            perror("ERROR reading from socket");
            exit(1);
        }
        do {
            printf("Enter move x(%d -> %d) _ y(%c -> %c)): \n", 0, BOARD_SIZE, 97, (97 + BOARD_SIZE));
            scanf("%d %c", &x, &c);
        } while ((x < 0 || x >= BOARD_SIZE) || ((int) c < 97 || (int) c >= (BOARD_SIZE + 97)));
        
        y = (int) c - 97;

        char buffer[BUFF_SIZE];
        memset(buffer, 0, BUFF_SIZE);
        sprintf(buffer, "%d %c", x, c);
        // int n = write(client_fd, buffer, strlen(buffer));
        bytes_sent = send(client_fd, buffer, strlen(buffer), 0);
        if (bytes_sent <= 0) {
            perror("ERROR writing to socket");
            exit(1);
        }

        addMove(playerSymbol, x, y);
        printCaroBoard();

        memset(recv_data, 0, BUFF_SIZE);
        memset(send_data, 0, BUFF_SIZE);
        memset(buffer, 0, BUFF_SIZE);
    }

    //Send endgame message to server
    char buffer[BUFF_SIZE];
    memset(buffer, 0, BUFF_SIZE);
    sprintf(buffer, "ENDGAME");
    int n = write(client_fd, buffer, strlen(buffer));
    if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }
    close(client_fd);
    return 0;
}