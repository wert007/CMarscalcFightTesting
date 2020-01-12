//#include "ansiescapecodes.h"
#include "ansiescapecodes.c"
#include <string.h>
double sin(double number);
float sqrtf(float x);
char *ltoa(long N, char *str, int base);
int max(int a, int b)
{
    if (a > b)
        return a;
    return b;
}

typedef enum
{
    SinWave,
    None,
} animation_t;

typedef struct
{
    pos_t pos;
    char *c;
    animation_t animation;
} label_t;

typedef struct
{
    float xPos;
    float yPos;
    float xVel;
    float yVel;
    consoleColor_t color;
    char character;
} particle_t;

typedef enum
{
    Explosion,
    Confetti
} particleCreatorType_t;

typedef struct
{
    pos_t initPos;
    long start;
    long end;
    particle_t *particles;
    int particleCount;
    particleCreatorType_t type;
} particleCreator_t;

void addParticleCreator(particleCreator_t p);
void printLabel(label_t l);

char *getNToMLines(char *c, int n, int m)
{
    int i = 0;
    while (n != 0)
    {
        if (c[i] == '\n')
            n--;
        i++;
    }
    int start = i;
    while (m != 0)
    {
        if (c[i] == '\n')
            m--;
        i++;
    }
    int end = i;
    char *result = calloc(end - start + 1, sizeof(char));
    for (int j = start; j < end; j++)
    {
        result[j - start] = c[j];
    }
    return result;
}

pos_t labelSize(label_t l)
{
    int x = 0;
    int maxX = 0;
    int y = 0;
    for (int i = 0; l.c[i]; i++)
    {
        if (l.c[i] == '\n')
        {
            y++;
            if (x > maxX)
                maxX = x;
            x = 0;
        }
        x++;
    }
    if (x > maxX)
        maxX = x;
    pos_t result;
    result.x = maxX;
    result.y = y;
    return result;
}

bool screenshakeActive = false;
particleCreator_t *particleCreators = NULL;
int particleCreatorCount = 0;

label_t *splitLabelHorizontally(label_t l)
{
    pos_t lSize = labelSize(l);
    if (lSize.y <= 1)
        return NULL;
    label_t *result = calloc(2, sizeof(label_t));
    label_t top;
    top.animation = None;
    top.pos = l.pos;
    top.pos.y -= lSize.y / 4;
    top.pos.x = -lSize.x;
    top.c = getNToMLines(l.c, 0, lSize.y / 2);
    result[0] = top;
    label_t bottom;
    bottom.animation = None;
    bottom.pos = l.pos;
    top.pos.x = size.x;
    bottom.pos.y += lSize.y / 4;
    bottom.c = getNToMLines(l.c, lSize.y / 2, lSize.y / 2);
    result[1] = bottom;
    return result;
}

//            createConfetti(3 * size.x / 4, size.y / 2, 120, tick, -0.5f, 0.5f);
void createConfetti(int x, int y, int duration, long tick, float xDir, float yDir)
{
    particleCreator_t result;
    result.type = Confetti;
    result.initPos.x = x;
    result.initPos.y = y;
    result.end = tick + duration;
    result.start = tick;
    result.particleCount = 1000;

    result.particles = calloc(result.particleCount, sizeof(particle_t));
    for (int i = 0; i < result.particleCount; i++)
    {
        particle_t p;
        p.xPos = x;
        p.yPos = y;
        p.character = '*';
        p.color = WHITE;
        p.xVel = xDir + (rand() % 20 - 40) / 100.0f;
        p.yVel = yDir + (rand() % 20 - 40) / 100.0f;
        // float length = sqrtf(p.xVel * p.xVel + p.yVel * p.yVel);
        // if (length > 1)
        // {
        //     p.xVel /= length;
        //     p.yVel /= length;
        // }

        result.particles[i] = p;
    }

    addParticleCreator(result);
}

void createExplosion(int x, int y, int radius, int duration, long tick)
{
    particleCreator_t result;
    result.type = Explosion;
    result.initPos.x = x;
    result.initPos.y = y;
    result.end = tick + duration;
    result.start = tick;
    result.particleCount = radius * duration;
    result.particles = calloc(result.particleCount, sizeof(particle_t));
    for (int i = 0; i < result.particleCount; i++)
    {
        particle_t p;
        p.xPos = x;
        p.yPos = y;
        p.character = 'x';
        p.color = RED;
        if (rand() % 4 == 0)
            p.color = ORANGE;
        p.xVel = (rand() % 200 - 100) / 100.0f;
        p.yVel = (rand() % 200 - 100) / 100.0f;
        float length = sqrtf(p.xVel * p.xVel + p.yVel * p.yVel);
        if (length > 1)
        {
            p.xVel /= length;
            p.yVel /= length;
        }

        result.particles[i] = p;
    }

    addParticleCreator(result);
}

void addParticleCreator(particleCreator_t p)
{
    particleCreatorCount++;
    particleCreators = realloc(particleCreators, particleCreatorCount * sizeof(particleCreator_t));
    particleCreators[particleCreatorCount - 1] = p;
}

void printAt(int x, int y, consoleColor_t color, char c)
{
    if (x < 0 || x >= size.x || y < 0 || y >= size.y)
        return;
    setCursor(x, y);
    setForeground(color);
    printChar(c);
    setForeground(WHITE);
}

void drawParticle(particle_t *particle)
{
    if (particle->character == 0)
        return;
    particle->xPos += particle->xVel;
    particle->yPos += particle->yVel;

    printAt((int)(particle->xPos + 0.5f), (int)(particle->yPos + 0.5f), particle->color, particle->character);
}

bool dragParticle(particle_t *particle, float drag, float minSpeed)
{
    if (particle->character == 0)
        return true;
    if (particle->xVel > 0)
    {
        particle->xVel -= drag;
        if (particle->xVel <= 0)
            particle->xVel = 0;
    }
    else if (particle->xVel < 0)
    {
        particle->xVel += drag;
        if (particle->xVel >= 0)
            particle->xVel = 0;
    }
    if (particle->yVel > 0)
    {
        particle->yVel -= drag;
        if (particle->yVel <= 0)
            particle->yVel = 0;
    }
    else if (particle->yVel < 0)
    {
        particle->yVel += drag;
        if (particle->yVel >= 0)
            particle->yVel = 0;
    }
    if (particle->xVel * particle->xVel + particle->yVel * particle->yVel < minSpeed * minSpeed)
    {
        particle->character = 0;
        return true;
    }
    return false;
}

void drawConfetti(particleCreator_t confetti, int tick)
{
    for (int i = 0; i < confetti.particleCount; i++)
    {
        drawParticle(confetti.particles + i);
        if (dragParticle(confetti.particles + i, 0.002f, 0.5f))
        {
            particle_t p;
            p.xPos = confetti.initPos.x;
            p.yPos = confetti.initPos.y;
            p.character = '*';
            p.color = WHITE;
            p.xVel = confetti.particles[i].xVel + (rand() % 20 - 40) / 100.0f;
            p.yVel = confetti.particles[i].yVel + (rand() % 20 - 40) / 100.0f;
            confetti.particles[i] = p;
        }
    }
}

void drawExplosion(particleCreator_t explosion, int tick)
{
    for (int i = 0; i < explosion.particleCount; i++)
    {
        drawParticle(explosion.particles + i);
        dragParticle(explosion.particles + i, 0.002f, 0.5f);
    }
}

void printProgressBar(float progress, int xPos, int yPos, int width, int height)
{
    // setCursor(xPos, yPos);
    // char s[100];
    // sprintf(s, "width = %d\n", (int)(width * progress));
    // print(s);
    for (int x = xPos; x < (int)(width * progress) + xPos; x++)
    {
        for (int y = yPos; y < height + yPos; y++)
        {
            setCursor(x - y + yPos, y);
            printChar('/');
        }
    }
}

void printAnimatedLabelToCenter(label_t *labels, int speed)
{
    label_t * top = labels;
    label_t * bottom = labels + 1;
    pos_t tSize = labelSize(*top);
    pos_t bSize = labelSize(*bottom);
    top->pos.y = size.y / 2 - tSize.y / 2 - 1;
    printLabel(*top);
    top->pos.x += speed;
    if (top->pos.x >= size.x / 2 - tSize.x / 2)
        top->pos.x = size.x / 2 - tSize.x / 2;
    bottom->pos.y = size.y / 2 + bSize.y / 2 - 1;
    printLabel(*bottom);
    bottom->pos.x -= speed;

    if (top->pos.x <= size.x / 2 - bSize.x / 2)
        bottom->pos.x = size.x / 2 - bSize.x / 2;
}

void printLabelCentered(label_t l)
{
    pos_t lSize = labelSize(l);
    setCursor(size.x / 2 - lSize.x / 2, size.y / 2 - lSize.y / 2);
    int n = 0;
    int x = 0;
    for (int i = 0; l.c[i]; i++)
    {
        x++;
        int xOff = 0;
        int yOff = 0;
        moveCursorRel(xOff, yOff);
        if (l.c[i] == '\n')
        {
            setCursor(size.x / 2 - lSize.x / 2, size.y / 2 - lSize.y / 2 + ++n);
            x = 0;
        }
        else
            printChar(l.c[i]);
        moveCursorRel(-xOff, -yOff);
    }
}

void printLabel(label_t l)
{
    pos_t lSize = labelSize(l);
    setCursor(l.pos.x, l.pos.y);
    int n = 0;
    for (int i = 0; l.c[i]; i++)
    {
        int y = i / lSize.x;
        int xOff = 0;
        int yOff = 0;
        if (l.animation == SinWave)
        {
            yOff = sin(y * 3.14);
        }
        moveCursorRel(xOff, yOff);
        if (l.c[i] == '\n')
            setCursor(l.pos.x, l.pos.y + (++n));
        else
            printChar(l.c[i]);
    }
}

void screenshake()
{
    screenshakeActive = !screenshakeActive;
    if (!screenshakeActive)
    {
        screenOffset.x = 0;
        screenOffset.y = 0;
    }
}

void screenshakeTick()
{
    if (!screenshakeActive)
        return;
    screenOffset.x = rand() % 3 - 1;
    screenOffset.y = rand() % 3 - 1;
}

void drawAnimations(long tick)
{
    screenshakeTick();
    for (int i = 0; i < particleCreatorCount; i++)
    {
        if (particleCreators[i].start <= tick && particleCreators[i].end > tick)
        {
            int progress = tick - particleCreators[i].start;
            switch (particleCreators[i].type)
            {
            case Explosion:
                drawExplosion(particleCreators[i], progress);
                break;
            case Confetti:
                drawConfetti(particleCreators[i], progress);
                break;
            }
        }
    }
}