#include <stdbool.h>

enum SpecialCommands
{
    cmd_END = -1,
};

typedef struct
{
    char *cmd;
    int index;
} command_t;

typedef struct
{
    int x, y;
} pos_t;

typedef enum
{
    BLACK = 0,
    RED = 1,
    GREEN = 2,
    ORANGE = 3,
    BLUE = 4,
    PURPLE = 5,
    TURQUOISE = 6,
    WHITE = 7,
} consoleColor_t;

void flashTick(long tick);
pos_t getSize();
void clearScreen();
void clearBuffer();
void setCursor(int x, int y);
void moveCursor(int offset);
void setForeground(consoleColor_t c);
void setBackground(consoleColor_t c);
void print(char *c);
