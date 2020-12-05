#include "Utilities.h"


int main(uint16_t argc, const uint8_t* argv[])
{
    if (argc < 2)
    {
        printf("lc3 [image-file1] ...\n");
        exit(2);
    }
    if (!readImage(argv[1]))
    {
        printf("failed to load image: %s\n", argv[1]);
        exit(1);
    }

    signal(SIGINT, handle_interrupt);
    disableInputBuffering();

    enum { PC_START = 0x3000 };
    reg[R_PC] = PC_START;

    uint16_t running = 1;
    while (running)
    {
        uint16_t instr = memRead(reg[R_PC]++);
        uint16_t op = instr >> 12;

        switch (op)
        {
        case OP_ADD:
        {
            uint16_t r0 = (instr >> 9) & 0x7;
            uint16_t r1 = (instr >> 6) & 0x7;
            uint16_t imm_flag = (instr >> 5) & 0x1;

            if (imm_flag)
            {
                uint16_t imm5 = signExtend(instr & 0x1F, 5);
                reg[r0] = reg[r1] + imm5;
            }
            else
            {
                uint16_t r2 = instr & 0x7;
                reg[r0] = reg[r1] + reg[r2];
            }

            updateFlags(r0);
            break;
        }
        case OP_AND:
        {
            uint16_t r0 = (instr >> 9) & 0x7;
            uint16_t r1 = (instr >> 6) & 0x7;
            uint16_t imm_flag = (instr >> 5) & 0x1;

            if (imm_flag)
            {
                uint16_t imm5 = signExtend(instr & 0x1F, 5);
                reg[r0] = reg[r1] & imm5;
            }
            else
            {
                uint16_t r2 = instr & 0x7;
                reg[r0] = reg[r1] & reg[r2];
            }

            updateFlags(r0);
            break;
        }
        case OP_NOT:
        {
            uint16_t r0 = (instr >> 9) & 0x7;
            uint16_t r1 = (instr >> 6) & 0x7;

            reg[r0] = ~reg[r1];

            updateFlags(r0);
            break;
        }
        case OP_BR:
        {
            uint16_t pc_offset = signExtend(instr & 0x1FF, 9);
            uint16_t cond_flag = (instr >> 9) & 0x7;
            if (cond_flag & reg[R_COND])
            {
                reg[R_PC] += pc_offset;
            }
            break;
        }
        case OP_JMP:
        {
            uint16_t r1 = (instr >> 6) & 0x7;
            reg[R_PC] = reg[r1];
            break;
        }
        case OP_JSR:
        {
            reg[R_R7] = reg[R_PC];
            uint16_t imm_flag = (instr >> 11) & 0x1;

            if (imm_flag)
            {
                uint16_t pc_offset = signExtend(instr & 0x7FF, 11);
                reg[R_PC] += pc_offset;
            }
            else
            {
                uint16_t r1 = (instr >> 6) & 0x7;
                reg[R_PC] = reg[r1];
            }
            break;
        }
        case OP_LD:
        {
            uint16_t r0 = (instr >> 9) & 0x7;
            uint16_t pc_offset = signExtend(instr & 0x1FF, 9);
            reg[r0] = memRead(reg[R_PC] + pc_offset);

            updateFlags(r0);
            break;
        }
        case OP_LDI:
        {
            uint16_t r0 = (instr >> 9) & 0x7;
            uint16_t pc_offset = signExtend(instr & 0x1FF, 9);
            reg[r0] = memRead(memRead(reg[R_PC] + pc_offset));

            updateFlags(r0);
            break;
        }
        case OP_LDR:
        {
            uint16_t r0 = (instr >> 9) & 0x7;
            uint16_t r1 = (instr >> 6) & 0x7;
            uint16_t offset = signExtend(instr & 0x3F, 6);

            reg[r0] = memRead(reg[r1] + offset);

            updateFlags(r0);
            break;
        }
        case OP_LEA:
        {
            uint16_t r0 = (instr >> 9) & 0x7;
            uint16_t pc_offset = signExtend(instr & 0x1FF, 9);

            reg[r0] = reg[R_PC] + pc_offset;

            updateFlags(r0);
            break;
        }
        case OP_ST:
        {
            uint16_t r1 = (instr >> 9) & 0x7;
            uint16_t pc_offset = signExtend(instr & 0x1FF, 9);

            memWrite(reg[R_PC] + pc_offset, reg[r1]);
            break;
        }
        case OP_STI:
        {
            uint16_t r1 = (instr >> 9) & 0x7;
            uint16_t pc_offset = signExtend(instr & 0x1FF, 9);

            memWrite(memRead(reg[R_PC] + pc_offset), reg[r1]);
            break;
        }
        case OP_STR:
        {
            uint16_t r1 = (instr >> 9) & 0x7;
            uint16_t r2 = (instr >> 6) & 0x7;
            uint16_t offset = signExtend(instr & 0x3F, 6);

            memWrite(reg[r2] + offset, reg[r1]);
            break;
        }
        case OP_TRAP:
        {
            switch (instr & 0xFF)
            {
            case TRAP_GETC:
            {
                reg[R_R0] = (uint16_t)getchar();
                break;
            }
            case TRAP_OUT:
            {
                putc((char)reg[R_R0], stdout);
                fflush(stdout);
                break;
            }
            case TRAP_PUTS:
            {
                uint16_t* c = memory + reg[R_R0];
                while (*c)
                {
                    putc((char)*c, stdout);
                    ++c;
                }
                fflush(stdout);
                break;
            }
            case TRAP_IN:
            {
                printf("Enter a character: ");
                char c = getchar();
                putc(c, stdout);
                reg[R_R0] = (uint16_t)c;
                break;
            }
            case TRAP_PUTSP:
            {
                uint16_t* c = memory + reg[R_R0];
                while (*c)
                {
                    char char1 = (*c) & 0xFF;
                    putc(char1, stdout);
                    char char2 = *c >> 8;
                    if (char2) putc(char2, stdout);
                    ++c;
                }
                fflush(stdout);
                break;
            }
            case TRAP_HALT:
            {
                printf("HALT!");
                fflush(stdout);
                running = 0;
                break;
            }
            }
            break;
        }
        case OP_RES:
        case OP_RTI:
        default:
            abort();
            break;
        }
    }
    restoreInputBuffering();
}