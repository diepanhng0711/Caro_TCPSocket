#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 1024
#define MAX_SIZE 256

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

void addMove(char playerSymbol, int x, int y)
{
    board[x][y] = playerSymbol;
}

// FUNCTION USING WHEN PLAYING VERSUS BOT
// FUNCTION USING WHEN PLAYING VERSUS BOT

int scoreCheck(char player)
{
    int i, j, count;
    int total_score = 0;

    // Check rows
    for (i = 0; i < BOARD_SIZE; i++)
    {
        count = 0;
        for (j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] == player)
            {
                count++;
            }
            else
            {
                count = 0;
            }

            if (count == 5)
            {
                total_score += 100;
            }
            else if (count > 1)
            {
                total_score += count * 2;
            }
        }
    }

    // Check columns
    for (j = 0; j < BOARD_SIZE; j++)
    {
        count = 0;
        for (i = 0; i < BOARD_SIZE; i++)
        {
            if (board[i][j] == player)
            {
                count++;
            }
            else
            {
                count = 0;
            }

            if (count == 5)
            {
                total_score += 100;
            }
            else if (count > 1)
            {
                total_score += count * 2;
            }
        }
    }

    // Check diagonals
    for (i = 0; i < BOARD_SIZE - 4; i++)
    {
        for (j = 0; j < BOARD_SIZE - 4; j++)
        {
            count = 0;
            int k;
            for (k = 0; k < 5; k++)
            {
                if (board[i + k][j + k] == player)
                {
                    count++;
                }
                else
                {
                    count = 0;
                }

                if (count == 5)
                {
                    total_score += 100;
                }
                else if (count > 1)
                {
                    total_score += count * 2;
                }
            }
        }
    }

    // Check anti-diagonals
    for (i = 0; i < BOARD_SIZE - 4; i++)
    {
        for (j = BOARD_SIZE - 1; j >= 4; j--)
        {
            count = 0;
            int k;
            for (k = 0; k < 5; k++)
            {
                if (board[i + k][j - k] == player)
                {
                    count++;
                }
                else
                {
                    count = 0;
                }

                if (count == 5)
                {
                    total_score += 100;
                }
                else if (count > 1)
                {
                    total_score += count * 2;
                }
            }
        }
    }

    return total_score;
}

int minimax(int depth, char player)
{
    int i, j, score;
    int best_score = (player == 'O') ? -10000 : 10000;

    // Check if the game is over or the maximum depth has been reached
    if (depth == 0)
    {
        return scoreCheck(player);
    }

    // printf("Checking depth");
    for (i = 0; i < BOARD_SIZE; i++)
    {
        for (j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] == ' ')
            {
                board[i][j] = player;

                if (player == 'O')
                {
                    score = minimax(depth - 1, 'X');
                    if (score > best_score)
                    {
                        best_score = score;
                    }
                }
                else
                {
                    score = minimax(depth - 1, 'O');
                    if (score < best_score)
                    {
                        best_score = score;
                    }
                }
                board[i][j] = ' ';
            }
        }
    }

    return best_score;
}

void bot_move()
{
    int i, j;
    int best_score = -10000;
    int score;
    int row = -1;
    int col = -1;
    for (i = 0; i < BOARD_SIZE; i++)
    {
        for (j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] == ' ')
            {
                board[i][j] = 'O';
                score = minimax(5, 'X');
                board[i][j] = ' ';

                if (score > best_score)
                {
                    best_score = score;
                    row = i;
                    col = j;
                }
            }
        }
    }

    printf("ROW: %d, COL: %d", row, col);

    board[row][col] = 'O';
}

// MAIN
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: ./client PortNumber\n");
        return -1;
    }

    if (atoi(argv[1]) == 0)
    {
        printf("You've enter PvE mode! Please wait...!\n");
        initCaroBoard();

        while (1)
        {
            printCaroBoard();
            // Player's turn
            printf("Player's turn (X)\n");
            int x;
            char c;
            do
            {
                printf("Enter move x(%d -> %d) _ y(%c -> %c)): \n", 0, BOARD_SIZE, 97, (97 + BOARD_SIZE));
                scanf("%d %c", &x, &c);
            } while ((x < 0 || x >= BOARD_SIZE) || (((int)c < 97 || (int)c >= (BOARD_SIZE + 97))) || (board[x][(int)c - 97] != ' '));

            // Make the move
            board[x][(int)c - 97] = 'X';
            printCaroBoard();

            // Check if the game is over
            if (scoreCheck('X') >= 100)
            {
                printf("Player wins!\n");
                break;
            }

            // Bot's turn
            printf("Bot's turn (O)\n");
            bot_move();

            // Print the board
            printCaroBoard();

            // Check if the game is over
            if (scoreCheck('O') >= 100)
            {
                printf("Bot wins!\n");
                break;
            }
        }

        return 0;
    }

    int client_fd;
    struct sockaddr_in server_addr;
    int bytes_sent, bytes_received;
    char recv_data[BUFF_SIZE], send_data[BUFF_SIZE];
    char buffer[BUFF_SIZE];
    char reply[MAX_SIZE];
    int playerPosition;
    int opponentSymbol;

    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0)
    {
        perror("ERROR opening socket");
        exit(1);
    }
    printf("Client socket created successfully!\n");

    memset((char *)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    if (inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr) <= 0)
    {
        perror("ERROR inet_pton");
        exit(1);
    }

    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("ERROR connecting");
        exit(1);
    }
    else
    {
        printf("Connected to server %s at port %d ...\n", SERVER_ADDR, atoi(argv[1]));
    }

    // Receive symbol of player
    bytes_received = recv(client_fd, recv_data, sizeof(recv_data), 0);
    char playerSymbol = recv_data[0];

    if (playerSymbol == MSG_X)
    {
        playerPosition = 0;
        opponentSymbol = MSG_Y;
    }
    else
    {
        playerPosition = 1;
        opponentSymbol = MSG_X;
    }

    printf("You are player %d _ \"%c\"\n\n", playerPosition, playerSymbol);

    // Send OK to server
    send(client_fd, "OK", 2, 0);

    // init the caro board for the game
    initCaroBoard();
    printCaroBoard();

    int x, y;
    int opponentX;
    char opponentY;
    char c;
    while (1)
    {
        memset(recv_data, 0, BUFF_SIZE);
        memset(send_data, 0, BUFF_SIZE);

        // Wait for the server to notify the player's turn
        printf("Waiting for your turn..\n");
        bytes_received = recv(client_fd, recv_data, sizeof(recv_data), 0);
        printf("You are player: %d\n", playerPosition);
        if (bytes_received <= 0)
        {
            perror("ERROR reading from socket");
            exit(1);
        }

        if (strcmp(recv_data, MSG_WIN) == 0)
        {
            printf("You win the game\n");
            close(client_fd);
            return 0;
        }
        else if (strcmp(recv_data, MSG_LOSE) == 0)
        {
            printf("You lose the game\n");
            close(client_fd);
            return 0;
        }
        else if (strcmp(recv_data, MSG_DRAW) == 0)
        {
            printf("Draw game\n");
            close(client_fd);
            return 0;
        }
        else if (strcmp(recv_data, MSG_GO_FIRST) != 0)
        {
            sscanf(recv_data, "%d %c", &opponentX, &opponentY);
            printf("Opponent's move: %d %c\n", opponentX, opponentY);
            addMove(opponentSymbol, opponentX, (int)opponentY - 97);
            printCaroBoard();
        }

        do
        {
            printf("Enter move x(%d -> %d) _ y(%c -> %c)): \n", 0, BOARD_SIZE, 97, (97 + BOARD_SIZE));
            scanf("%d %c", &x, &c);
        } while ((x < 0 || x >= BOARD_SIZE) || (((int)c < 97 || (int)c >= (BOARD_SIZE + 97))) || (board[x][(int)c - 97] != ' '));

        if (x == 99)
        {
            memset(recv_data, 0, BUFF_SIZE);
            memset(send_data, 0, BUFF_SIZE);
            memset(buffer, 0, BUFF_SIZE);
            printf("You quit the game\n");
            close(client_fd);
            return 0;
        }

        y = (int)c - 97;

        memset(buffer, 0, BUFF_SIZE);
        sprintf(buffer, "%d %c", x, c);
        bytes_sent = send(client_fd, buffer, strlen(buffer), 0);
        if (bytes_sent <= 0)
        {
            perror("ERROR writing to socket");
            exit(1);
        }

        addMove(playerSymbol, x, y);
        printCaroBoard();

        memset(recv_data, 0, BUFF_SIZE);
        memset(send_data, 0, BUFF_SIZE);
        memset(buffer, 0, BUFF_SIZE);
    }

    // Send endgame message to server
    memset(buffer, 0, BUFF_SIZE);
    sprintf(buffer, "ENDGAME");
    int n = write(client_fd, buffer, strlen(buffer));
    if (n < 0)
    {
        perror("ERROR writing to socket");
        exit(1);
    }
    close(client_fd);
    return 0;
}