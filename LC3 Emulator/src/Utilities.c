#include "Utilities.h"

HANDLE hStdin = INVALID_HANDLE_VALUE;

uint16_t signExtend(uint16_t x, uint16_t bitCount)
{
    if ((x >> (bitCount - 1)) & 1)
        x |= (0xFFFF << bitCount);
    return x;
}

void updateFlags(uint16_t r)
{
    if (reg[r] == 0)
    {
        reg[R_COND] = FL_ZRO;
    }
    else if (reg[r] >> 15)
    {
        reg[R_COND] = FL_NEG;
    }
    else
    {
        reg[R_COND] = FL_POS;
    }
}

uint16_t swap16(uint16_t x)
{
    return (x << 8) | (x >> 8);
}

void readImageFile(FILE* file)
{
    uint16_t origin;
    fread(&origin, sizeof(origin), 1, file);

    /* Little Endian Check */
    int check = 1;
    if (*(char*)&check == 1)
    {
        origin = swap16(origin);
    }
    else check = 0;

    uint16_t maxRead = UINT16_MAX - origin;
    uint16_t* p = memory + origin;
    size_t read = fread(p, sizeof(uint16_t), maxRead, file);

    while (read-- > 0 && check)
    {
        *p = swap16(*p);
        ++p;
    }
}

uint16_t readImage(const uint8_t* imagePath)
{
    FILE* file = NULL;
    fopen_s(&file, imagePath, "rb");
    if (!file) return 0;
    readImageFile(file);
    fclose(file);
    return 1;
}

void memWrite(uint16_t address, uint16_t val)
{
    memory[address] = val;
}

uint16_t memRead(uint16_t address)
{
    if (address == MR_KBSR)
    {
        if (checkKey())
        {
            memory[MR_KBSR] = (1 << 15);
            memory[MR_KBDR] = getchar();
        }
        else
        {
            memory[MR_KBDR] = 0;
        }
    }
    return memory[address];
}

int checkKey()
{
    return WaitForSingleObject(hStdin, 1000) == WAIT_OBJECT_0 && _kbhit();
}

void disableInputBuffering()
{
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hStdin, &fdwOldMode);
    fdwMode = fdwOldMode
        ^ ENABLE_ECHO_INPUT
        ^ ENABLE_LINE_INPUT;
    SetConsoleMode(hStdin, fdwMode);
    FlushConsoleInputBuffer(hStdin);
}

void restoreInputBuffering()
{
    SetConsoleMode(hStdin, fdwOldMode);
}

void handle_interrupt(int signal)
{
    restoreInputBuffering();
    printf("\n");
    exit(-2);
}