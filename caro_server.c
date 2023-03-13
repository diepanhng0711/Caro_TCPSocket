#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <string.h>
#include <sys/stat.h>

// DEFINE ENVIROMENT VARIABLES FOR PROGRAMS
#define BOARD_SIZE 15
#define BUFF_SIZE 1024
#define MAX_SIZE 256
#define MAX_CLIENTS 2

// DEFINE MESSAGE TYPE
#define MSG_GO_FIRST "LADY FIRST"
#define MSG_PLAYER1 "X"
#define MSG_PLAYER2 "O"
#define MSG_X 'X'
#define MSG_Y 'O'
#define MSG_INVALID "INVALID"
#define MSG_READY "OK"
#define MSG_WIN "WIN"
#define MSG_LOSE "LOSE"
#define MSG_DRAW "DRAW"

char board[BOARD_SIZE][BOARD_SIZE];
int client_fds[MAX_CLIENTS];
int current_turn = 0;

void initCaroBoard()
{
    int i, j;
    for (i = 0; i < BOARD_SIZE; i++)
    {
        for (j = 0; j < BOARD_SIZE; j++)
        {
            board[i][j] = ' ';
        }
    }
}

void printCaroBoard()
{
    int i, j;
    printf("+ ");
    for (i = 0; i < BOARD_SIZE; i++)
    {
        printf("%c ", 97 + i);
    }
    printf("+\n+");
    for (i = 0; i < BOARD_SIZE; i++)
    {
        printf("--");
    }
    printf("-+\n");
    for (i = 0; i < BOARD_SIZE; i++)
    {
        printf("||");
        for (j = 0; j < BOARD_SIZE; j++)
        {
            printf("%c|", board[i][j]);
        }
        printf("| %d\n+", i);
        for (j = 0; j < BOARD_SIZE; j++)
        {
            printf("%s", "--");
        }
        printf("-+\n");
    }
}

void addMove(char playerSymbol, int x, int y)
{
    board[x][y] = playerSymbol;
}

int winnerWinnerChickenDinner(char playerSymbol)
{
    // Checking if a player win the game
    int i, j;

    // Check horizontal
    for (i = 0; i < BOARD_SIZE; i++)
    {
        for (j = 0; j < BOARD_SIZE - 4; j++)
        {
            if (board[i][j] == playerSymbol && board[i][j + 1] == playerSymbol && board[i][j + 2] == playerSymbol && board[i][j + 3] == playerSymbol && board[i][j + 4] == playerSymbol)
            {
                return 1;
            }
        }
    }

    // Check vertical
    for (i = 0; i < BOARD_SIZE - 4; i++)
    {
        for (j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] == playerSymbol && board[i + 1][j] == playerSymbol && board[i + 2][j] == playerSymbol && board[i + 3][j] == playerSymbol && board[i + 4][j] == playerSymbol)
            {
                return 1;
            }
        }
    }

    // Check diagonal
    for (i = 0; i < BOARD_SIZE - 4; i++)
    {
        for (j = 0; j < BOARD_SIZE - 4; j++)
        {
            if (board[i][j] == playerSymbol && board[i + 1][j + 1] == playerSymbol && board[i + 2][j + 2] == playerSymbol && board[i + 3][j + 3] == playerSymbol && board[i + 4][j + 4] == playerSymbol)
            {
                return 1;
            }
        }
    }

    // Check anti-diagonal
    for (i = 0; i < BOARD_SIZE - 4; i++)
    {
        for (j = 4; j < BOARD_SIZE; j++)
        {
            if (board[i][j] == playerSymbol && board[i + 1][j - 1] == playerSymbol && board[i + 2][j - 2] == playerSymbol && board[i + 3][j - 3] == playerSymbol && board[i + 4][j - 4] == playerSymbol)
            {
                return 1;
            }
        }
    }

    return 0;
}

bool checkDraw()
{
    int i, j;
    for (i = 0; i < BOARD_SIZE; i++)
    {
        for (j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] == ' ')
            {
                return false;
            }
        }
    }
    return true;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: ./server PortNumber\n");
        return -1;
    }

    int i;
    int x, y;
    char c;
    int server_fd, client_fd;
    socklen_t client_len;
    struct sockaddr_in server_addr, client_addr;
    int bytes_sent, bytes_received;
    char recv_data[BUFF_SIZE], send_data[BUFF_SIZE];
    char buffer[BUFF_SIZE];
    char reply[MAX_SIZE];
    int num_clients = 0;
    int winner = -1;
    int prevX;
    char prevY;
    bool hasStarted = false;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("ERROR opening socket");
        exit(1);
    }

    memset((char *)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("ERROR on binding");
        exit(1);
    }

    int ret = listen(server_fd, 10);
    if (ret < 0)
    {
        perror("ERROR on listen");
        exit(1);
    }

    client_len = sizeof(client_addr);

    printf("You've created the server at PORT %s", argv[1]);
    printf("Waiting for clients to connect...\n");
    while (num_clients < MAX_CLIENTS)
    {
        client_fds[num_clients] = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fds[num_clients] < 0)
        {
            perror("ERROR on accept");
            exit(1);
        }
        printf("Client %d connected\n", num_clients + 1);
        num_clients++;
    }

    // Notify each client their symbol
    for (i = 0; i < MAX_CLIENTS; i++)
    {
        if (i == 0)
        {
            sprintf(send_data, MSG_PLAYER1);
        }
        else
        {
            sprintf(send_data, MSG_PLAYER2);
        }
        bytes_sent = send(client_fds[i], send_data, strlen(send_data), 0);
        if (bytes_sent < 0)
        {
            perror("ERROR writing to socket");
            exit(1);
        }
        // receive OK from each client
        bytes_received = recv(client_fds[i], recv_data, BUFF_SIZE, 0);
        if (bytes_received < 0)
        {
            perror("ERROR reading from socket");
            exit(1);
        }
        else
        {
            printf("Client %d: %s\n", i + 1, recv_data);
        }
    }

    initCaroBoard();
    memset(buffer, 0, BUFF_SIZE);
    memset(reply, 0, MAX_SIZE);
    memset(recv_data, 0, BUFF_SIZE);
    memset(send_data, 0, BUFF_SIZE);

    // Enter the game
    while (1)
    {
        int i;
        for (i = 0; i < MAX_CLIENTS; i++)
        {
            memset(buffer, 0, BUFF_SIZE);
            if (i == 0)
            {
                if (!hasStarted)
                {
                    sprintf(buffer, "%s", MSG_GO_FIRST);
                }
                else
                {
                    sprintf(buffer, "%d %c", prevX, prevY);
                }
                printf("X's turn\n");
            }
            else
            {
                sprintf(buffer, "%d %c", prevX, prevY);
                printf("O's turn\n");
            }
            bytes_sent = send(client_fds[i], buffer, strlen(buffer), 0);
            if (bytes_sent < 0)
            {
                perror("ERROR writing to socket");
                exit(1);
            }
            else
            {
                printf("Sent message: %s\n", buffer);
            }

            memset(buffer, 0, BUFF_SIZE);
            recv(client_fds[i], buffer, BUFF_SIZE - 1, 0);
            recv_data[bytes_received] = '\0';

            sscanf(buffer, "%d %c", &x, &c);
            prevX = x;
            prevY = c;
            y = (int)c - 97;

            if (i == 0)
            {
                board[x][y] = 'X';
            }
            else
            {
                board[x][y] = 'O';
            }
            printf("Received message: %s\n", buffer);
            printCaroBoard();

            if (winnerWinnerChickenDinner('X'))
            {
                winner = 0;
                printf("X wins!\n");
                break;
            }
            else if (winnerWinnerChickenDinner('O'))
            {
                winner = 1;
                printf("O wins!\n");
                break;
            }
            else if (checkDraw())
            {
                winner = -1;
                printf("Draw!\n");
                break;
            }
        }

        if (winner == 0 || winner == 1)
        {
            int send_winner = send(client_fds[winner], MSG_WIN, strlen(MSG_WIN), 0);
            printf("Player %d win", winner);
            if (send_winner < 0)
            {
                perror("ERROR writing to socket");
                exit(1);
            }
            int send_loser = send(client_fds[(winner + 1) % 2], MSG_LOSE, strlen(MSG_LOSE), 0);
            if (send_loser < 0)
            {
                perror("ERROR writing to socket");
                exit(1);
            }
            memset(recv_data, 0, BUFF_SIZE);
            memset(send_data, 0, BUFF_SIZE);
            memset(buffer, 0, BUFF_SIZE);
            break;
        }
        else if (winner == -1)
        {
            // send lose to each two player
            for (i = 0; i < MAX_CLIENTS; i++)
            {
                int bytes1_sent = send(client_fds[i], MSG_DRAW, strlen(MSG_DRAW), 0);
                if (bytes1_sent < 0)
                {
                    perror("ERROR writing to socket");
                    exit(1);
                }
                
            }
            memset(recv_data, 0, BUFF_SIZE);
            memset(send_data, 0, BUFF_SIZE);
            memset(buffer, 0, BUFF_SIZE);
            break;
        }
        if (!hasStarted)
        {
            hasStarted = true;
        }
    }

    // Close socket and display message
    for (i = 0; i < MAX_CLIENTS; i++)
    {
        close(client_fds[i]);
    }
    close(server_fd);
    return 0;
}