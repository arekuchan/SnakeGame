#define _XOPEN_SOURCE_EXTENDED
#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <locale.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <math.h>


#define ROW 20
#define COL 40
#define MAXTRAPS 20

// compile with -lncursesw

int field[ROW][COL], x, y, gY, head, tail, game, frogs, userIO, dir, score, highscore, hsFileNull, toggleTraps, numTraps, playing = 1;
FILE* highscoreData;

void loadHighscore() {
    highscoreData = fopen("Snake_Highscore.txt", "r");

    if (!highscoreData) {
        highscoreData = fopen("Snake_Highscore.txt", "w");

        if (!highscoreData) {
            hsFileNull = 1;
        }

        highscore = 0;
    } else {
        highscore = fgetc(highscoreData) - '0';
    }
}

void snakeInit() {
    srand(time(0));
    x = ROW / 2;
    y = COL / 2;
    gY = y;
    head = 5;
    tail = 1;
    game = 1;
    frogs = 0;
    dir = 'd';
    score = 0;
    toggleTraps = 1;
    numTraps = 5;
    playing = 1;

    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    scrollok(stdscr, TRUE);
    loadHighscore();

    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++) {
            field[i][j] = 0;
        }
    }

    for (int i = 0; i < head; i++) {
        gY++;
        field[x][gY - head] = i + 1;
    }

}

void delay(int requiredMS) {
    clock_t startTime = clock();
    int startTimeMs = (startTime / CLOCKS_PER_SEC) * 1000;
    clock_t intendedDelay = ((startTimeMs + requiredMS) / 1000) * CLOCKS_PER_SEC;
    while (clock() < intendedDelay);
}

void setFrog() {
    int x,y;

    x = ((rand() % 18)) + 1;
    y = ((rand() % 38)) + 1;

    if (!frogs && field[x][y] == 0) {
        field[x][y] = -1;
        frogs = 1;
    }
}

int kbhit() {
    int ch = getch();

    if (ch != ERR) {
        ungetch(ch);
        return 1;
    } else {
        return 0;
    }
}

int getch_noblock() {
    if (kbhit()) {
        return getch();
    } else {
        return -1;
    }
}

void updateFrogs(int x, int y) {
    if (field[x][y] == -1) {
        score++;
        frogs = 0;

        if (numTraps < MAXTRAPS) {
            numTraps++;
            toggleTraps = 1;
        }
    }
}

void updateSnakeLength(int x, int y) {
    if (field[x][y] == -1) {
        tail -= 2;
    }
}

int hitItself(int x, int y) {
    if (field[x][y] != 0 && field[x][y] != -1) {
        return 1;
    }

    return 0;
}

void updateHighscore(void) {
    if (!hsFileNull) {
        fclose(highscoreData);
    }

    remove("Snake_Highscore.txt");
    highscoreData = fopen("Snake_Highscore.txt", "w");

    if (highscoreData) {
        if (score > highscore) {
            fputc(score + '0', highscoreData);
        } else {
            fputc(highscore + '0', highscoreData);
        }

        fclose(highscoreData);
    } else {
        printw("isnull");
    }
}

int reasonableBounds(int x0, int y0) {
    int bound = 3;

    if (abs(x - x0) < bound || abs(y - y0) < bound) {
        return 0;
    }

    return 1;
}

void clearTraps() {
    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++) {
            if (field[i][j] == -2) {
                field[i][j] = 0;
            }
        }
    }
}

void spawnTraps() {
    int positions[numTraps][2], x, y;

    if (toggleTraps) {
        clearTraps();
        //Initialise trap positions
        for (int i = 0; i < numTraps; i++) {
            x = ((rand() % 18)) + 1;
            y = ((rand() % 38)) + 1;

            if (!reasonableBounds(x, y)) {
                i--;
                continue;
            }

            positions[i][0] = x;
            positions[i][1] = y;
        }

        //Update field grid
        for (int i = 0; i < numTraps; i++) {
            x = positions[i][0];
            y = positions[i][1];

            field[x][y] = -2;
        }

        toggleTraps = 0;
    }
}

void gameOver() {
    int input;

    usleep(200 * 1000);
    system("clear");
    game = 0;
    updateHighscore();

    printf("You lose! Your score was: %d\n", score);
    usleep(5000 * 500);
}



void checkBounds(int x, int y) {
    if (y >= COL - 1 || y <= -1) {
        gameOver();
    } else if (x <= 0 || x >= ROW - 1) {
        gameOver();
    }
}

int hitTrap(int x, int y) {
    if (field[x][y] == -2) {
        return 1;
    }

    return 0;
}

void moveSnake(int dir) {
    checkBounds(x, y);
    head++;
    switch (dir)
    {
    case 'w':
        x--;

        if (x == 0 || hitItself(x, y) ||
                hitTrap(x, y)) {
            gameOver();
        }

        updateFrogs(x, y);
        updateSnakeLength(x, y);
        break;

    case 'd':
        y++;

        if (y == COL - 1 || hitItself(x, y) ||
                hitTrap(x, y)) {
            gameOver();
        }

        updateFrogs(x, y);
        updateSnakeLength(x, y);
        break;

    case 's':
        x++;

        if (x == ROW - 1 || hitItself(x, y) ||
                hitTrap(x, y)) {
            gameOver();
        }

        updateFrogs(x, y);
        updateSnakeLength(x, y);
        break;

    case 'a':
        y--;

        if (y == 0 || hitItself(x, y) ||
                hitTrap(x, y)) {
            gameOver();
        }

        updateFrogs(x, y);
        updateSnakeLength(x, y);
        break;
    }

    field[x][y] = head;
}

void movement() {
    userIO = getch_noblock();
    userIO = tolower(userIO);

    if (userIO == 'w' || userIO == 'a' 
    || userIO == 's' || userIO == 'd') {
        dir = userIO;
    }

    moveSnake(dir);
}

int printCorners(int i, int j) {
    if (i == 0 && j == 0) {
        printw("╔");
        return 1;
    } else if (i == 0 && j == COL - 1) {
        printw("╗\n");
        return 1;
    } else if (i == ROW - 1 && j == 0) {
        printw("╚");
        return 1;
    } else if (i == ROW - 1 && j == COL - 1) {
        printw("╝");
        return 1;
    }

    return 0;
}

int printBorders(int i, int j) {
    if (i == 0 || i == ROW - 1) {
        if (j != 0 && j != COL - 1) {
            printw("═");
            return 1;
        }
    } else if (j == 0) {
        printw("║");
        return 1;
    } else if (j == COL - 1) {
        printw("║\n");
        return 1;
    }

    return 0;
}

void printSquare() {
    int cornerPrinted, boarderPrinted;
    move(0,0);
    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++) {
            cornerPrinted = printCorners(i, j);

            if (!cornerPrinted) {
                boarderPrinted = printBorders(i, j);
            }

            if (!cornerPrinted && !boarderPrinted) {
                if (field[i][j] == 0) {
                    printw(" ");
                } else if (field[i][j] > 0 && field[i][j] != head) {
                    printw("█");
                } else if (field[i][j] == head) {
                    printw("░");
                } else if (field[i][j] == -1) {
                    printw("O");
                } else if (field[i][j] == -2) {
                    printw("X");
                }
            }
        }
    }
}

void printScore() {
    printw("\nScore: %d\nHighscore: %d", score, highscore);
}


void removeTail() {
    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++) {
            if (field[i][j] == tail) {
                field[i][j] = 0;
            }
        }
    }

    tail++;
}

int main() {
    setlocale(LC_ALL, "");
    initscr();
    snakeInit();

    while(game) {
        printSquare();
        printScore();
        spawnTraps();
        setFrog();
        refresh();
        movement();
        removeTail();
        usleep(75 * 1000);
    }
}
