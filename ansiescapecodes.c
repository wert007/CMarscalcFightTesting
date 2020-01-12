#include "ansiescapecodes.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>

bool flashActivated = false;
bool cursorIsVisible = true;

pos_t screenOffset;

pos_t cursorPos;
pos_t size;

int backBufferSize = -1;
char *backBuffer;
command_t *commands;
int commandCount = 0;

void init()
{
    cursorPos.x = 0;
    cursorPos.y = 0;
    screenOffset.x = 0;
    screenOffset.y = 0;
    size = getSize();
    backBufferSize = size.x * size.y;
    backBuffer = (char *)calloc(backBufferSize + 1, sizeof(char));
}

void drawBackBuffer(long tick)
{
    flashTick(tick);
    clearScreen();
    // if(tick % 2 == 0)
    //     setBackground(TURQUOISE);
    // else
    //     setForeground(WHITE);
    // setCursor(screenOffset.x, screenOffset.y);
    for (int i = 0; i < backBufferSize; i++)
    {
        if (screenOffset.x != 0 || screenOffset.y != 0)
        {
            int x = i % size.x;
            int y = i / size.x;
            if (x + screenOffset.x < 0)
                continue;
            if (y + screenOffset.y < 0)
                continue;
            if (x + screenOffset.x >= size.x)
                continue;
            if (y + screenOffset.y >= size.y)
                continue;
        }
        for (int j = 0; j < commandCount; j++)
        {
            if (commands[j].index == i)
            {
                printf("%s", commands[j].cmd);
            }
        }
        printf("%c", backBuffer[i]);
    }
    clearBuffer();
}

int posToIndex(pos_t p)
{
    return p.x + p.y * size.x;
}

void printInt(int i)
{
    char s[50];
    sprintf(s, "%d", i);
    print(s);
}

void printChar(char c)
{
    int index = posToIndex(cursorPos);
    backBuffer[index] = c;
    moveCursor(1);
}

void print(char *c)
{
    for (int i = 0; c[i]; i++)
    {
        int index = posToIndex(cursorPos);
        backBuffer[index] = c[i];
        // printf("%c", c[i]);
        moveCursor(1);
    }
}

void moveCursor(int offset)
{
    cursorPos.x += offset;
    if (cursorPos.x >= size.x)
    {
        cursorPos.x = size.x - 1;
        //cursorPos.y += 1;
    }
    if (cursorPos.y >= size.y)
    {
        cursorPos.x = 0;
        cursorPos.y = 0;
    }
}

command_t createCommand(int cmd)
{
    command_t result;
    result.index = posToIndex(cursorPos);
    result.cmd = calloc(7, sizeof(char));
    if (cmd >= 0)
        sprintf(result.cmd, "\033[%03dm", cmd);
    else if (cmd == -1)
        sprintf(result.cmd, "\033[m");

    return result;
}

void addCommand(int cmd)
{
    commandCount++;
    commands = realloc(commands, sizeof(command_t) * commandCount);
    commands[commandCount - 1] = createCommand(cmd);
}

void printBetweenTags(int cmd, char *c)
{
    addCommand(cmd);
    print(c);
    addCommand(cmd_END);
}

void clearBuffer()
{
    for (int i = 0; i < backBufferSize; i++)
        backBuffer[i] = ' ';
    for(int i = 0; i < commandCount; i++)
        free(commands[i].cmd);
    free(commands);
    commands = NULL;
    commandCount = 0;
}

void clearScreen()
{
    printf("\033[2J\033[;H");
}

void toggleCursorVisibility()
{
    cursorIsVisible = !cursorIsVisible;
    if (cursorIsVisible)
        printf("\e[?25h");
    else
        printf("\e[?25l");
}

void printBold(char *c)
{
    printBetweenTags(1, c);
}

void printCursive(char *c)
{
    printBetweenTags(3, c);
}

void printUnderlined(char *c)
{
    printBetweenTags(4, c);
}

void printBlink(char *c)
{
    printBetweenTags(5, c);
}

void setFlashing(bool value)
{
    flashActivated = value;
}

void flashTick(long tick)
{
    if (!flashActivated)
        return;
    if (tick % 2 == 0)
        printf("\e[?5h");
    else
        printf("\e[?5l");
}

void printInvert(char *c)
{
    printBetweenTags(7, c);
}

void printStrike(char *c)
{
    printBetweenTags(9, c);
}

void setForeground(consoleColor_t c)
{
    addCommand(30 + c);
}

void setBackground(consoleColor_t c)
{
    addCommand(40 + c);
}

void drawBackground(int x0, int y0, int x1, int y1, consoleColor_t color)
{
    for(int y = y0; y < y1; y++)
    {
        setCursor(x0, y);
        setBackground(color);
        setCursor(x1, y);
        setBackground(BLACK);
    }
}

pos_t getSize()
{
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

    pos_t result;
    result.x = w.ws_col;
    result.y = w.ws_row;
    return result;
}

void setCursor(int x, int y)
{
    // printf("\033[%d;%dH", y, x);
    if(x >= 0 && x < size.x)
        cursorPos.x = x;
    if(y >= 0 && y < size.y)
        cursorPos.y = y;
}

void moveCursorRel(int x, int y)
{
    cursorPos.x += x;
    cursorPos.y += y;
    // if (y < 0)
    //     printf("\033[%dA", -y); // Move up X lines;
    // if (y > 0)
    //     printf("\033[%dB", y); // Move down X lines;
    // if (x < 0)
    //     printf("\033[%dD", -x); // Move left X column;
    // if (x > 0)
    //     printf("\033[%dC", x); // Move right X column;
}

pos_t getCursorPos()
{
    //HACKY
    system("/bin/stty raw");
    pos_t res;
    res.x = -1;
    res.y = -1;

    printf("\e[6n");
    if (getchar() != '\e')
        return res;
    if (getchar() != '\[')
        return res;
    int read = 2;
    char yres[20] = {};
    int i = 0;
    char cur = getchar();
    while (cur != ';')
    {
        yres[i] = cur;
        i++;
        cur = getchar();
    }
    int y = atoi(yres);
    read += i + 1;
    char xres[20] = {};
    i = 0;
    cur = getchar();
    while (cur != 'R')
    {
        xres[i] = cur;
        i++;
        cur = getchar();
    }
    int x = atoi(xres);
    // read += i + 2;
    // moveCursorRel(-read, 0);
    // for(int i = 0; i < read; i++)
    //     printf("%c", ' ');
    // moveCursorRel(-read, 0);

    //HACKY!
    system("/bin/stty cooked");
    res.x = x;
    res.y = y;
    cursorPos.x = x;
    cursorPos.y = y;
    return res;
}
