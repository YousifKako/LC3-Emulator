#include <stdint.h>
#include <stdio.h>

#include <signal.h>
#include <Windows.h>
#include <conio.h>


enum // Registers
{
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC,     /* Program Counter */
    R_COND,
    R_COUNT   /* Total number of registers */
};

enum // Opcodes
{
    OP_BR = 0, /* branch */
    OP_ADD,    /* add  */
    OP_LD,     /* load */
    OP_ST,     /* store */
    OP_JSR,    /* jump register */
    OP_AND,    /* bitwise and */
    OP_LDR,    /* load register */
    OP_STR,    /* store register */
    OP_RTI,    /* unused */
    OP_NOT,    /* bitwise not */
    OP_LDI,    /* load indirect */
    OP_STI,    /* store indirect */
    OP_JMP,    /* jump */
    OP_RES,    /* reserved (unused) */
    OP_LEA,    /* load effective address */
    OP_TRAP    /* execute trap */
};

enum // Flag Upadtes
{
    FL_POS = 1 << 0, /* P */
    FL_ZRO = 1 << 1, /* Z */
    FL_NEG = 1 << 2, /* N */
};

enum // TRAP Routines
{
    TRAP_GETC = 0x20,  /* get character from keyboard, not echoed onto the terminal */
    TRAP_OUT = 0x21,   /* output a character */
    TRAP_PUTS = 0x22,  /* output a word string */
    TRAP_IN = 0x23,    /* get character from keyboard, echoed onto the terminal */
    TRAP_PUTSP = 0x24, /* output a byte string */
    TRAP_HALT = 0x25   /* halt the program */
};

enum // Memory Mapped Registers
{
    MR_KBSR = 0xFE00, /* keyboard status */
    MR_KBDR = 0xFE02  /* keyboard data */
};

uint16_t memory[UINT16_MAX];
uint16_t reg[R_COUNT];

uint16_t signExtend(uint16_t x, uint16_t bitCount);
void updateFlags(uint16_t r);
uint16_t swap16(uint16_t x);
void readImageFile(FILE* file);
uint16_t readImage(const char* imagePath);
int checkKey();

extern HANDLE hStdin;
DWORD fdwMode, fdwOldMode;

void disableInputBuffering();
void restoreInputBuffering();
void handle_interrupt(int signal);
void memWrite(uint16_t address, uint16_t val);
uint16_t memRead(uint16_t address);