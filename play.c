#include "animations.c"
#include <unistd.h>
#include <stdio.h>
#include <string.h>

typedef struct
{
    char *name;
    int hp;
    int wonFights;
} player_t;

typedef enum
{
    op_Addition,
    op_Subtraction,
    op_Multiplication,
} operator_t;

typedef enum
{
    mode_Default,
    mode_Long,
    mode_LongComma,
} mode_t;

typedef enum
{
    NotSet,
    LeftWon,
    Tie,
    RightWon,
} fightResult_t;

typedef struct
{
    int a;
    int b;
    int n;
    bool leftError;
    bool rightError;
    char *leftRes;
    char *rightRes;
    float progress;
    char *arg;
    fightResult_t result;
} values_t;

FILE *popen(const char *command, const char *type);

int pclose(FILE *stream);

char *toBase9(int i, bool start);
bool hasError(char *c);
int countChar(char *s, char c);
void printTable();
void printField(player_t left, player_t right, char *arg);
bool streq(char *a, char *b);
char *foo(char *command);
int playSound(char *filename);
void playIntro(long tick, bool update, bool draw);
void playRound1Review(long tick, values_t *v, bool update, bool draw);
void playRound2Review(long tick, values_t *v, bool update, bool draw);
void updateOperator(long *tick, long start, long end, operator_t op, mode_t mode, values_t *v);
void drawOperator(values_t v);

enum
{
    SCREENSHAKE1 = 70,
    INTRO = 140,
    FIGHT = 220,
    ADDITION = 340,
    REVIEW1 = 1500,
    ADDITIONCOMMA = 2000,
    REVIEW2 = 7000,
    SUBTRACTION = 10000,
    REVIEW3 = 11500,
    BATTLE = 9999999999999
} phase;

label_t lLeft;
label_t vs;
label_t lRight;
label_t youWin;
label_t itsATie;

player_t left;
player_t right;
long coolDown = -1;
int speed = 1;
int speedTick;

void initAll(char **argv)
{
    init();
   
   
   // clearScreen();
    clearBuffer();
    toggleCursorVisibility();
    left.name = argv[1];
    left.hp = 25;
    left.wonFights = 0;
    right.name = argv[2];
    right.hp = 25;
    right.wonFights = 0;

    lLeft.c = argv[1];
    lLeft.pos.x = 0;
    lLeft.pos.y = size.y / 2 - 1;

    vs.c = "vs";
    vs.pos.x = size.x / 2 - 1;
    vs.pos.y = 0;

    lRight.c = argv[2];
    lRight.pos.x = size.x - 1;
    lRight.pos.y = size.y / 2 + 1;

    youWin.c = "__/\\\\\\________/\\\\\\_______/\\\\\\\\\\_______/\\\\\\________/\\\\\\____________/\\\\\\______________/\\\\\\__/\\\\\\\\\\\\\\\\\\\\\\__/\\\\\\\\\\_____/\\\\\\_____/\\\\\\____________\n__\\///\\\\\\____/\\\\\\/______/\\\\\\///\\\\\\____\\/\\\\\\_______\\/\\\\\\___________\\/\\\\\\_____________\\/\\\\\\_\\/////\\\\\\///__\\/\\\\\\\\\\\\___\\/\\\\\\___/\\\\\\\\\\\\\\_________\n_____\\///\\\\\\/\\\\\\/______/\\\\\\/__\\///\\\\\\__\\/\\\\\\_______\\/\\\\\\___________\\/\\\\\\_____________\\/\\\\\\_____\\/\\\\\\_____\\/\\\\\\/\\\\\\__\\/\\\\\\__/\\\\\\\\\\\\\\\\\\_______\n________\\///\\\\\\/_______/\\\\\\______\\//\\\\\\_\\/\\\\\\_______\\/\\\\\\___________\\//\\\\\\____/\\\\\\____/\\\\\\______\\/\\\\\\_____\\/\\\\\\//\\\\\\_\\/\\\\\\_\\//\\\\\\\\\\\\\\_______\n___________\\/\\\\\\_______\\/\\\\\\_______\\/\\\\\\_\\/\\\\\\_______\\/\\\\\\____________\\//\\\\\\__/\\\\\\\\\\__/\\\\\\_______\\/\\\\\\_____\\/\\\\\\\\//\\\\\\\\/\\\\\\__\\//\\\\\\\\\\_______\n____________\\/\\\\\\_______\\//\\\\\\______/\\\\\\__\\/\\\\\\_______\\/\\\\\\_____________\\//\\\\\\/\\\\\\/\\\\\\/\\\\\\________\\/\\\\\\_____\\/\\\\\\_\\//\\\\\\/\\\\\\___\\//\\\\\\_______\n_____________\\/\\\\\\________\\///\\\\\\__/\\\\\\____\\//\\\\\\______/\\\\\\_______________\\//\\\\\\\\\\\\//\\\\\\\\\\_________\\/\\\\\\_____\\/\\\\\\__\\//\\\\\\\\\\\\____\\///_______\n______________\\/\\\\\\__________\\///\\\\\\\\\\/______\\///\\\\\\\\\\\\\\\\\\/_________________\\//\\\\\\__\\//\\\\\\_______/\\\\\\\\\\\\\\\\\\\\\\_\\/\\\\\\___\\//\\\\\\\\\\_____/\\\\\\_____\n_______________\\///_____________\\/////__________\\/////////____________________\\///____\\///_______\\///////////__\\///_____\\/////_____\\///_____";
    youWin.animation = None;

    itsATie.c = "__/\\\\\\\\\\\\\\\\\\\\\\__/\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\__/\\\\\\\\_____/\\\\\\\\\\\\\\\\\\\\\\_________________/\\\\\\\\\\\\\\\\\\_______________/\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\__/\\\\\\\\\\\\\\\\\\\\\\__/\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\_____/\\\\\\____________\n__\\/////\\\\\\///__\\///////\\\\\\/////__\\///\\\\___/\\\\\\/////////\\\\\\_____________/\\\\\\\\\\\\\\\\\\\\\\\\\\____________\\///////\\\\\\/////__\\/////\\\\\\///__\\/\\\\\\///////////____/\\\\\\\\\\\\\\_________\n_______\\/\\\\\\___________\\/\\\\\\________/\\\\/___\\//\\\\\\______\\///_____________/\\\\\\/////////\\\\\\_________________\\/\\\\\\___________\\/\\\\\\_____\\/\\\\\\______________/\\\\\\\\\\\\\\\\\\_______\n________\\/\\\\\\___________\\/\\\\\\_______\\//______\\////\\\\\\___________________\\/\\\\\\_______\\/\\\\\\_________________\\/\\\\\\___________\\/\\\\\\_____\\/\\\\\\\\\\\\\\\\\\\\\\_____\\//\\\\\\\\\\\\\\_______\n_________\\/\\\\\\___________\\/\\\\\\___________________\\////\\\\\\________________\\/\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\_________________\\/\\\\\\___________\\/\\\\\\_____\\/\\\\\\///////_______\\//\\\\\\\\\\_______\n__________\\/\\\\\\___________\\/\\\\\\______________________\\////\\\\\\_____________\\/\\\\\\/////////\\\\\\_________________\\/\\\\\\___________\\/\\\\\\_____\\/\\\\\\_______________\\//\\\\\\_______\n___________\\/\\\\\\___________\\/\\\\\\_______________/\\\\\\______\\//\\\\\\____________\\/\\\\\\_______\\/\\\\\\_________________\\/\\\\\\___________\\/\\\\\\_____\\/\\\\\\________________\\///_______\n_________/\\\\\\\\\\\\\\\\\\\\\\_______\\/\\\\\\______________\\///\\\\\\\\\\\\\\\\\\\\\\/_____________\\/\\\\\\_______\\/\\\\\\_________________\\/\\\\\\________/\\\\\\\\\\\\\\\\\\\\\\_\\/\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\_____/\\\\\\_____\n_________\\///////////________\\///_________________\\///////////_______________\\///________\\///__________________\\///________\\///////////__\\///////////////_____\\///_____";
    itsATie.animation = None;
}

values_t initV()
{
    values_t result;
    result.a = 0;
    result.arg = NULL;
    result.b = 0;
    result.leftError = false;
    result.leftRes = NULL;
    result.n = 1;
    result.progress = 0;
    result.result = NotSet;
    result.rightError = false;
    result.rightRes = NULL;
    return result;
}

int main(int argc, char **argv)
{
    if (argc != 3)
        return -1;
    initAll(argv);

    label_t fight;
    fight.c = "__/\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\__/\\\\\\\\\\\\\\\\\\\\\\_____/\\\\\\\\\\\\\\\\\\\\\\\\__/\\\\\\________/\\\\\\__/\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\_____/\\\\\\____________\n__\\/\\\\\\///////////__\\/////\\\\\\///____/\\\\\\//////////__\\/\\\\\\_______\\/\\\\\\_\\///////\\\\\\/////____/\\\\\\\\\\\\\\_________\n___\\/\\\\\\_________________\\/\\\\\\______/\\\\\\_____________\\/\\\\\\_______\\/\\\\\\_______\\/\\\\\\________/\\\\\\\\\\\\\\\\\\_______\n____\\/\\\\\\\\\\\\\\\\\\\\\\_________\\/\\\\\\_____\\/\\\\\\____/\\\\\\\\\\\\\\_\\/\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\_______\\/\\\\\\_______\\//\\\\\\\\\\\\\\_______\n_____\\/\\\\\\///////__________\\/\\\\\\_____\\/\\\\\\___\\/////\\\\\\_\\/\\\\\\/////////\\\\\\_______\\/\\\\\\________\\//\\\\\\\\\\_______\n______\\/\\\\\\_________________\\/\\\\\\_____\\/\\\\\\_______\\/\\\\\\_\\/\\\\\\_______\\/\\\\\\_______\\/\\\\\\_________\\//\\\\\\_______\n_______\\/\\\\\\_________________\\/\\\\\\_____\\/\\\\\\_______\\/\\\\\\_\\/\\\\\\_______\\/\\\\\\_______\\/\\\\\\__________\\///_______\n________\\/\\\\\\______________/\\\\\\\\\\\\\\\\\\\\\\_\\//\\\\\\\\\\\\\\\\\\\\\\\\/__\\/\\\\\\_______\\/\\\\\\_______\\/\\\\\\___________/\\\\\\_____\n_________\\///______________\\///////////___\\////////////____\\///________\\///________\\///___________\\///_____";
    //  label_t *fights = splitLabelHorizontally(fight);

    label_t addition;
    addition.c = "_____/\\\\\\\\\\\\\\\\\\_____/\\\\\\\\\\\\\\\\\\\\\\\\_____/\\\\\\\\\\\\\\\\\\\\\\\\_____/\\\\\\\\\\\\\\\\\\\\\\__/\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\__/\\\\\\\\\\\\\\\\\\\\\\_______/\\\\\\\\\\_______/\\\\\\\\\\_____/\\\\\\_____/\\\\\\____________\n____/\\\\\\\\\\\\\\\\\\\\\\\\\\__\\/\\\\\\////////\\\\\\__\\/\\\\\\////////\\\\\\__\\/////\\\\\\///__\\///////\\\\\\/////__\\/////\\\\\\///______/\\\\\\///\\\\\\____\\/\\\\\\\\\\\\___\\/\\\\\\___/\\\\\\\\\\\\\\_________\n____/\\\\\\/////////\\\\\\_\\/\\\\\\______\\//\\\\\\_\\/\\\\\\______\\//\\\\\\_____\\/\\\\\\___________\\/\\\\\\___________\\/\\\\\\_______/\\\\\\/__\\///\\\\\\__\\/\\\\\\/\\\\\\__\\/\\\\\\__/\\\\\\\\\\\\\\\\\\_______\n____\\/\\\\\\_______\\/\\\\\\_\\/\\\\\\_______\\/\\\\\\_\\/\\\\\\_______\\/\\\\\\_____\\/\\\\\\___________\\/\\\\\\___________\\/\\\\\\______/\\\\\\______\\//\\\\\\_\\/\\\\\\//\\\\\\_\\/\\\\\\_\\//\\\\\\\\\\\\\\_______\n_____\\/\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\_\\/\\\\\\_______\\/\\\\\\_\\/\\\\\\_______\\/\\\\\\_____\\/\\\\\\___________\\/\\\\\\___________\\/\\\\\\_____\\/\\\\\\_______\\/\\\\\\_\\/\\\\\\\\//\\\\\\\\/\\\\\\__\\//\\\\\\\\\\_______\n______\\/\\\\\\/////////\\\\\\_\\/\\\\\\_______\\/\\\\\\_\\/\\\\\\_______\\/\\\\\\_____\\/\\\\\\___________\\/\\\\\\___________\\/\\\\\\_____\\//\\\\\\______/\\\\\\__\\/\\\\\\_\\//\\\\\\/\\\\\\___\\//\\\\\\_______\n_______\\/\\\\\\_______\\/\\\\\\_\\/\\\\\\_______/\\\\\\__\\/\\\\\\_______/\\\\\\______\\/\\\\\\___________\\/\\\\\\___________\\/\\\\\\______\\///\\\\\\__/\\\\\\____\\/\\\\\\__\\//\\\\\\\\\\\\____\\///_______\n________\\/\\\\\\_______\\/\\\\\\_\\/\\\\\\\\\\\\\\\\\\\\\\\\/___\\/\\\\\\\\\\\\\\\\\\\\\\\\/____/\\\\\\\\\\\\\\\\\\\\\\_______\\/\\\\\\________/\\\\\\\\\\\\\\\\\\\\\\____\\///\\\\\\\\\\/_____\\/\\\\\\___\\//\\\\\\\\\\_____/\\\\\\_____\n_________\\///________\\///__\\////////////_____\\////////////_____\\///////////________\\///________\\///////////_______\\/////_______\\///_____\\/////_____\\///_____";
    //label_t *additions = splitLabelHorizontally(addition);

    long tick = 0;
    long updateTick = 0;
    bool draw = true;
    bool update = true;
    values_t v = initV();
    while (true)
    {
        update = coolDown == -1;
        if (updateTick < INTRO)
        {
            playIntro(updateTick, update, draw);
        }

        if (updateTick == SCREENSHAKE1)
            screenshake();

        if (updateTick == INTRO)
            screenshake();
        if (updateTick >= INTRO && updateTick < FIGHT)
        {
            printLabelCentered(fight);
            //printAnimatedLabelToCenter(fights, 10);
        }
        if (updateTick + 4 == INTRO)
            playSound("fight.wav");
        if (updateTick >= FIGHT && updateTick < ADDITION)
        {
            printField(left, right, "");
            printLabelCentered(addition);
            //            printAnimatedLabelToCenter(additions, 10);
        }
        if (updateTick - 3 == FIGHT)
            playSound("roundOne.wav");
        if (updateTick >= ADDITION && updateTick < REVIEW1)
        {
            if (coolDown == -1)
                updateOperator(&updateTick, ADDITION, REVIEW1, op_Addition, mode_Long, &v);
            drawOperator(v);
        }

        if (updateTick >= REVIEW1 && updateTick < ADDITIONCOMMA)
        {
            playRound1Review(updateTick, &v, update, draw);
        }
        if (updateTick == ADDITIONCOMMA)
        {
            coolDown = -1;
            v = initV();
        }
        if (updateTick >= ADDITIONCOMMA && updateTick < REVIEW2)
        {
            speed = 500;
            speedTick = 0;
            if (coolDown == -1)
                updateOperator(&updateTick, ADDITIONCOMMA, REVIEW2, op_Addition, mode_LongComma, &v);
            drawOperator(v);
        }
        if (updateTick >= REVIEW2 && updateTick < SUBTRACTION)
        {
            playRound2Review(updateTick, &v, update, draw);
        }
        if (updateTick == SUBTRACTION)
            v = initV();
        if (updateTick >= SUBTRACTION && updateTick < REVIEW3)
        {
            if (coolDown == -1)
                updateOperator(&updateTick, SUBTRACTION, REVIEW3, op_Subtraction, mode_LongComma, &v);
            drawOperator(v);
        }
        drawAnimations(tick);
        bool quiet = false;
        if(!quiet) //QUIET MODE!!!
        drawBackBuffer(tick);

        if (coolDown == tick)
            coolDown = -1;
        if (speedTick > speed)
        {
            tick++;
            speedTick = 0;
            if (coolDown == -1)
                updateTick++;
        }
        speedTick++;
        if(!quiet) //QUIET MODE!!!
        
        usleep(16000);
    }
}

void playIntro(long tick, bool update, bool draw)
{
    if (update)
    {
        if (lLeft.pos.x < size.x / 2)
            lLeft.pos.x += 2;
        if (vs.pos.y != size.y / 2)
            vs.pos.y += 1;
        if (lRight.pos.x > size.x / 2)
            lRight.pos.x -= 2;
    }
    if (draw)
    {
        printLabel(lLeft);
        printLabel(vs);
        printLabel(lRight);
        if (tick == 10)
            createExplosion(size.x / 2, size.y / 2, 10, 50, 10);
    }
}

void setCoolDown(int duration, long tick)
{
    coolDown = tick + duration;
}

void updateValues(values_t *v)
{
    if (rand() % 20 == 0)
        v->a *= -1;
    if (rand() % 20 == 0)
        v->b *= -1;
    if (v->a > 0)
        v->a += v->n;
    else
        v->a -= v->n;
    if (v->a >= 1000 || v->a <= -1000)
    {
        v->b += v->b;
        if (v->b >= 0)
            v->b += 1;
        else
            v->b -= 1;

        v->a = 0;
        v->n += 1;
    }
}

void updateOperator(long *tick, long start, long end, operator_t op, mode_t mode, values_t *v)
{
    long t = *tick;
    updateValues(v);
    v->progress = (float)(t - start) / (end - start);

    v->arg = calloc(401, sizeof(char));
    char *as = toBase9(v->a, true);
    char *bs = toBase9(v->a, true);
    char *au = toBase9(abs(v->a), true);
    char *bu = toBase9(abs(v->a), true);

    switch (op)
    {
    case op_Addition:
        switch (mode)
        {
        case mode_Default:
            sprintf(v->arg, "%s + %s", as, bs);
            break;
        case mode_Long:
            sprintf(v->arg, "%s%s%s%s + %s%s%s%s", as, bu, au, bu, bs, bu, bu, au);
            break;
        case mode_LongComma:
            sprintf(v->arg, "%s,%s%s%s + %s,%s%s%s", as, bu, au, bu, bs, bu, bu, au);
            break;
        }
        break;
    case op_Subtraction:
        switch (mode)
        {
        case mode_Default:
            sprintf(v->arg, "%s - %s", as, bs);
            break;
        case mode_Long:
            sprintf(v->arg, "%s%s%s%s - %s%s%s%s", as, bu, au, bu, bs, bu, bu, au);
            break;
        case mode_LongComma:
            sprintf(v->arg, "%s,%s%s%s - %s,%s%s%s", as, bu, au, bu, bs, bu, bu, au);
            break;
        }
        break;
    case op_Multiplication:
        switch (mode)
        {
        case mode_Default:
            sprintf(v->arg, "%s \"*\" %s", as, bs);
            break;
        case mode_Long:
            sprintf(v->arg, "%s%s%s%s \"*\" %s%s%s%s", as, bu, au, bu, bs, bu, bu, au);
            break;
        case mode_LongComma:
            sprintf(v->arg, "%s,%s%s%s \"*\" %s,%s%s%s", as, bu, au, bu, bs, bu, bu, au);
            break;
        }
        break;
    }
    char s[100];
    sprintf(s, "%s %s", right.name, v->arg);
    v->rightRes = foo(s);
    v->rightError = hasError(v->rightRes);
    sprintf(s, "%s %s", left.name, v->arg);
    v->leftRes = foo(s);
    v->leftError = hasError(v->leftRes);

    if (left.hp < 0 || right.hp < 0)
    {
        *tick = end;
        return;
    }
    if (v->leftError)
    {
        left.hp--;
        createExplosion(size.x / 4 - 12, 14, 5, 50, t);
    }
    if (v->rightError)
    {
        right.hp--;
        createExplosion(3 * size.x / 4 - 12, 14, 5, 50, t);
    }

    if (!streq(v->leftRes, v->rightRes) || v->leftError || v->rightError)
    {
        setCoolDown(50, t);
        if (!(v->leftError || v->rightError))
        {
            left.hp--;
            right.hp--;
        }
    }
}

void drawOperator(values_t v)
{
    printProgressBar(v.progress, 6, 3, size.x - 12, 2);
    printField(left, right, v.arg);
    setCursor(size.x / 4, size.y / 2 + 5);
    if(v.leftRes != NULL)
    print(v.leftRes);
    setCursor(3 * size.x / 4, size.y / 2 + 5);
    if(v.rightRes != NULL)
    print(v.rightRes);

    if (!streq(v.leftRes, v.rightRes))
    {
        drawBackground(6, 6, size.x - 6, size.y - 6, RED);
    }
}
/*
void playOperatorLong(long *tick, bool update, bool draw, bool lastUpdate, operator_t op)
{
    static int ai = 0;
    static int bi = 0;
    static int n = 1;

    long t = *tick;
    if (update)
        updateAdditionArgs(&ai, &bi, &n);
    float progress = (float)(t - ADDITION) / (REVIEW1 - ADDITION);
    char arg[200];
    char *as = toBase9(ai, true);
    char *bs = toBase9(bi, true);
    char *au = toBase9(abs(ai), true);
    char *bu = toBase9(abs(bi), true);
    switch (op)
    {
    case op_Addition:
        sprintf(arg, "%s%s%s%s + %s%s%s%s", as, bu, au, bu, bs, bu, bu, au);
        break;
    case op_Subtraction:
        sprintf(arg, "%s%s%s%s - %s%s%s%s", as, bu, au, bu, bs, bu, bu, au);
        break;
    case op_Multiplication:
        sprintf(arg, "%s%s%s%s \"*\" %s%s%s%s", as, bu, au, bu, bs, bu, bu, au);
        break;
    }
    char s[100];
    sprintf(s, "%s %s", left.name, arg);
    char *leftRes = foo(s);
    bool leftHasError = hasError(leftRes);
    sprintf(s, "%s %s", right.name, arg);
    char *rightRes = foo(s);
    bool rightHasError = hasError(rightRes);

    if (left.hp < 0 || right.hp < 0)
    {
        *tick = REVIEW1;
        return;
    }

    if (update)
    {
        if (leftHasError)
        {
            left.hp--;
            createExplosion(size.x / 4 - 12, 14, 5, 50, t);
        }
        if (rightHasError)
        {
            right.hp--;
            createExplosion(3 * size.x / 4 - 12, 14, 5, 50, t);
        }

        if (!streq(leftRes, rightRes) || leftHasError || rightHasError)
        {
            setCoolDown(50, t);
            if (!(leftHasError || rightHasError))
            {
                drawBackground(6, 6, size.x - 6, size.y - 6, RED);
                left.hp--;
                right.hp--;
            }
        }
        else
            setBackground(BLACK);
    }
    if (draw)
    {
        printProgressBar(progress, 6, 3, size.x - 12, 2);
        printField(left, right, arg);
        setCursor(size.x / 4, size.y / 2 + 5);
        print(leftRes);
        setCursor(3 * size.x / 4, size.y / 2 + 5);
        print(rightRes);

        if (!streq(leftRes, rightRes))
        {
            if (leftAtFaults)
                drawBackground(6, 6, size.x / 2 + 1, size.y - 6, RED);
            else
                drawBackground(size.x / 2 + 1, 6, size.x - 6, size.y - 6, RED);
        }
    }
}

void playOperatorLongComma(long *tick, bool update, bool draw, bool lastUpdate, operator_t op)
{
    fightResult = NotSet;
    static int ai = 0;
    static int bi = 0;
    static int n = 1;
    static bool leftAtFaults = false;

    long t = *tick;
    if (update)
        updateAdditionArgs(&ai, &bi, &n);
    float progress = (float)(t - ADDITION) / (REVIEW1 - ADDITION);
    char arg[200];
    char *as = toBase9(ai, true);
    char *bs = toBase9(bi, true);
    char *au = toBase9(abs(ai), true);
    char *bu = toBase9(abs(bi), true);
    switch (op)
    {
    case op_Addition:
        sprintf(arg, "%s,%s%s%s + %s,%s%s%s", as, bu, au, bu, bs, bu, bu, au);
        break;
    case op_Subtraction:
        sprintf(arg, "%s,%s%s%s - %s,%s%s%s", as, bu, au, bu, bs, bu, bu, au);
        break;
    case op_Multiplication:
        sprintf(arg, "%s,%s%s%s \"*\" %s,%s%s%s", as, bu, au, bu, bs, bu, bu, au);
        break;
    }
    char s[100];
    sprintf(s, "%s %s", left.name, arg);
    char *leftRes = foo(s);
    bool leftHasError = hasError(leftRes);
    sprintf(s, "%s %s", right.name, arg);
    char *rightRes = foo(s);
    bool rightHasError = hasError(rightRes);

    if (update)
    {
        if (leftHasError)
        {
            left.hp--;
            createExplosion(size.x / 4 - 12, 14, 5, 50, t);
        }
        if (rightHasError)
        {
            right.hp--;
            createExplosion(3 * size.x / 4 - 12, 14, 5, 50, t);
        }

        if (!streq(leftRes, rightRes) || leftHasError || rightHasError)
        {
            setCoolDown(50, t);
            if (!(leftHasError || rightHasError))
            {
                leftAtFaults = rand() % 2 == 0;
                if (leftAtFaults)
                {
                    drawBackground(6, 6, size.x / 2 + 1, size.y - 6, RED);
                    left.hp--;
                }
                else
                {
                    drawBackground(size.x / 2 + 1, 6, size.x - 6, size.y - 6, RED);
                    right.hp--;
                }
            }
        }
        else
            setBackground(BLACK);
    }
    if (draw)
    {
        printProgressBar(progress, 6, 3, size.x - 12, 2);
        printField(left, right, arg);
        setCursor(size.x / 4, size.y / 2 + 5);
        print(leftRes);
        setCursor(3 * size.x / 4, size.y / 2 + 5);
        print(rightRes);

        if (!streq(leftRes, rightRes))
        {
            if (leftAtFaults)
                drawBackground(6, 6, size.x / 2 + 1, size.y - 6, RED);
            else
                drawBackground(size.x / 2 + 1, 6, size.x - 6, size.y - 6, RED);
        }
    }
}
*/
void playRound1Review(long tick, values_t *v, bool update, bool draw)
{
    if (update)
    {
        if ((v->result == NotSet))
        {
            v->result = Tie;
            if (left.hp < right.hp)
            {
                v->result = RightWon;
                right.wonFights++;
                createConfetti(3 * size.x / 4, size.y / 2, 120, tick, -0.5f, 0.5f);
            }
            else if (right.hp < left.hp)
            {
                v->result = LeftWon;
                left.wonFights++;
                createConfetti(size.x / 4, size.y / 2, 120, tick, 0.5f, 0.5f);
            }
            right.hp = 25;
            left.hp = 25;
        }
    }
    if (draw)
    {
        label_t finished;
        finished.c = ".#####....####...##..##..##..##..#####.............##............######..######..##..##..######...####...##..##..######..#####..\n.##..##..##..##..##..##..###.##..##..##...........###............##........##....###.##....##....##......##..##..##......##..##.\n.#####...##..##..##..##..##.###..##..##............##............####......##....##.###....##.....####...######..####....##..##.\n.##..##..##..##..##..##..##..##..##..##............##............##........##....##..##....##........##..##..##..##......##..##.\n.##..##...####....####...##..##..#####...........######..........##......######..##..##..######...####...##..##..######..#####..";
        pos_t lSize = labelSize(finished);
        pos_t rSize = labelSize(youWin);
        finished.pos.x = size.x / 2 - lSize.x / 2;
        finished.pos.y = size.y / 2 - lSize.y - rSize.y;
        finished.animation = None;
        printLabel(finished);
        if (v->result != Tie)
        {
            printLabelCentered(youWin);
            setCursor(size.x / 2 - 5, size.y / 2 + 6);
            if (v->result == LeftWon)
                printBlink(left.name);
            else
                printBlink(right.name);
        }
        else
            printLabelCentered(itsATie);
    }
}

void playRound2Review(long tick, values_t *v, bool update, bool draw)
{
    if (update)
    {
        if ((v->result == NotSet))
        {
            v->result = Tie;
            if (left.hp < right.hp)
            {
                v->result = RightWon;
                right.wonFights++;
                createConfetti(3 * size.x / 4, size.y / 2, 120, tick, -0.5f, 0.5f);
            }
            else if (right.hp < left.hp)
            {
                v->result = LeftWon;
                left.wonFights++;
                createConfetti(size.x / 4, size.y / 2, 120, tick, 0.5f, 0.5f);
            }
            right.hp = 25;
            left.hp = 25;
        }
    }
    if (draw)
    {
        label_t finished;
        finished.c = ".#####....####...##..##..##..##..#####............####...........######..######..##..##..######...####...##..##..######..#####..\n.##..##..##..##..##..##..###.##..##..##..............##..........##........##....###.##....##....##......##..##..##......##..##.\n.#####...##..##..##..##..##.###..##..##...........####...........####......##....##.###....##.....####...######..####....##..##.\n.##..##..##..##..##..##..##..##..##..##..........##..............##........##....##..##....##........##..##..##..##......##..##.\n.##..##...####....####...##..##..#####...........######..........##......######..##..##..######...####...##..##..######..#####..";
        pos_t lSize = labelSize(finished);
        pos_t rSize = labelSize(youWin);
        finished.pos.x = size.x / 2 - lSize.x / 2;
        finished.pos.y = size.y / 2 - lSize.y - rSize.y;
        finished.animation = None;
        printLabel(finished);
        if (v->result != Tie)
        {
            printLabelCentered(youWin);
            setCursor(size.x / 2 - 5, size.y / 2 + 6);
            if (v->result == LeftWon)
                printBlink(left.name);
            else
                printBlink(right.name);
        }
        else
            printLabelCentered(itsATie);
    }
}

void printField(player_t left, player_t right, char *arg)
{
    int margin = 6;
    setCursor(size.x / 4 - 2 * margin, margin + 2);
    setForeground(TURQUOISE);
    if(left.name != NULL)
    print(left.name);
    print(" ");
    setForeground(GREEN);
    if(arg != NULL)
    print(arg);
    setForeground(WHITE);
    int hpBarSize = left.hp + 5;
    setCursor(size.x / 2 - hpBarSize, margin + 1);
    print("HP: ");
    setForeground(RED);
    for (int i = 0; i < left.hp; i++)
        //todo: print("♥");
        print("@");
    setForeground(WHITE);
    setCursor(3 * size.x / 4 - 2 * margin, margin + 2);
    setForeground(TURQUOISE);
    if(right.name != NULL)
    print(right.name);
    print(" ");
    setForeground(GREEN);
    if(arg != NULL)
    print(arg);
    setForeground(WHITE);
    hpBarSize = right.hp + 5;
    setCursor(size.x - hpBarSize - margin, margin + 1);
    print("HP: ");
    setForeground(RED);
    for (int i = 0; i < right.hp; i++)
        //print("♥");
        print("@");
    setForeground(WHITE);
    for (int x = margin; x < size.x - margin; x++)
    {
        setCursor(x, margin);
        print("=");
        setCursor(x, margin + 4);
        print("=");
        setCursor(x, size.y - margin - 1);
        print("=");
    }
    for (int y = margin; y < size.y - margin; y++)
    {
        setCursor(margin, y);
        print("||");
        setCursor(size.x - margin - 1, y);
        print("||");
        setCursor(size.x / 2, y);
        print("||");
    }
}

bool streq(char *a, char *b)
{
    if(a == NULL && b == NULL) return true;
    if(a == NULL || b == NULL) return false;
    if (strlen(a) != strlen(b))
        return false;
    for (int i = 0; a[i]; i++)
    {
        if (a[i] != b[i])
            return false;
    }
    return true;
}

char *foo(char *command)
{
    FILE *fp;
    char path[200];
    char *result = calloc(200, sizeof(char));
    /* Open the command for reading. */
    fp = popen(command, "r");
    if (fp == NULL)
    {
        printf("Failed to run command\n");
        exit(1);
    }

    /* Read the output a line at a time - output it. */
    if (fgets(path, sizeof(path), fp) != NULL)
    {
        int i = 0;
        for (; path[i] != '\n' && path[i] != 0 && i < 200; i++)
            result[i] = path[i];
        result[i] = 0;
    }
    setCursor(0, 0);
    //print("no problems in foooooooo!");
    /* close */
    pclose(fp);

    return result;
}

int playSound(char *filename)
{
    return 0;
    char command[256];
    int status;

    /* create command to execute */
    sprintf(command, "aplay -c 1 -q -t wav %s", filename);

    /* play sound */
    status = system(command);

    return status;
}

void printTable()
{
    int i, j;

    for (i = 0; i < 10; i++)
    {
        for (j = 1; j < 11; j++)
            printf("\033[%03dm %03d\033[m", i * 10 + j - 1, i * 10 + j - 1);
        printf("\n");
    }
    printf("\n---\n");
    for (i = 101; i < 108; i++)
        printf("\033[%03dm %03d\033[m", i, i);
    printf("\n");
}

/*
array_t* foo()
{
    array_t *result = malloc(sizeof(array_t));
    result->array = malloc(45);
    result->isSigned = false;
    return a;
}
*/

char *toBase9(int i, bool start)
{
    if (i == 0)
        return "0";
    int digit = abs(i % 9);
    char *res = toBase9(i / 9, false);
    int end = 2;
    if (start && i < 0)
        end++;
    char *s = calloc(strlen(res) + end, sizeof(char));
    if (start && i < 0)
        sprintf(s, "-%s%i", res, digit);
    else
        sprintf(s, "%s%i", res, digit);
    return s;
}

bool hasError(char *s)
{
    int comma = 0;
    int minus = 0;

    for (int i = 0; s[i]; i++)
    {
        switch (s[i])
        {
        case ',':
            comma++;
            if (comma > 1)
                return true;
            break;
        case '-':
            if (i != 0)
                return true;
            minus++;
            if (minus > 1)
                return true;
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
            break;
        default:
            return true;
        }
    }
    return false;
}

int countChar(char *s, char c)
{
    int result = 0;
    for (int i = 0; s[i]; i++)
        if (s[i] == c)
            result++;
    return result;
}