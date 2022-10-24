#include <stdio.h>
#include "../io/io.h"
#include <assert.h>
#include "scpu.h"
#include <math.h>
#include <SFML/Graphics.hpp>
#include <limits>

#define Type_t int
#include "../stack/stack.h"

const int MEM_H = 200;

const int MEM_W = 320;

const int PIXEL_SIZE = 4;

const int FPS = 10;

struct Cpu {
    size_t program_size;
    char *program;
    size_t ip;
    Stack stack;
    Stack calls;
    int REG[5];
    int RAM[MEM_H * MEM_W];
};

void get_jmp_param(int *a, int *b, int *arg, Cpu *cpu);

int * get_ptr_arg(Cpu *cpu);

int init_cpu(Cpu *cpu, FILE* program_file);

void cpu_des(Cpu *cpu);

int do_cpu(Cpu *cpu);

int main(int argc, const char *argv[]) {
    const char *file_in_path = nullptr;

    get_infile_name_from_flug(&file_in_path, argc, argv);
    assert(file_in_path != nullptr);

    FILE *program_file = fopen(file_in_path, "rb");
    assert(program_file != nullptr);

    Cpu cpu = {0};

    if (init_cpu(&cpu, program_file)) {
        cpu_des(&cpu);
        return 0;
    }

    do_cpu(&cpu);

    cpu_des(&cpu);
}

int do_cpu(Cpu *cpu) {
    sf::RenderWindow window(sf::VideoMode(MEM_W * PIXEL_SIZE, MEM_H * PIXEL_SIZE), "Test Window");
    window.setFramerateLimit(10);
    sf::VertexArray pointmap(sf::Points, MEM_W * PIXEL_SIZE * MEM_H * PIXEL_SIZE);

    while(true) {
        switch (cpu->program[cpu->ip++]) {
            #define DEF_CMD(CMD, NUM, ARG, CODE) case NUM: {CODE}

            #include "../commands/commands.h"

            #undef DEF_CMD

            default: {
                printf("command_not_found ip:%d\n", cpu->ip-1);
                return 1;
                break;
            }
        }

        if (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                window.close();
            }
        }
    }

    if (window.isOpen()) {
        window.close();
    }
    return 0;
}

void cpu_des(Cpu *cpu) {
    free(cpu->program);
}

int init_cpu(Cpu *cpu, FILE *program_file) {
    assert(program_file != nullptr);

    char *file_type = (char *)calloc(2, sizeof(char));
    assert(file_type != nullptr);

    fread(file_type, sizeof(char), 2, program_file);

    if (file_type[0] != 'K' || file_type[1] != 'C') {
        printf("NOT COMPILE TYPE FILE");
        return 1;
    }

    char command_version = 0;
    fread(&command_version, sizeof(char), 1, program_file);

    if ((COMMAND_VERSION) != ((unsigned int)(unsigned char)command_version)) {
        printf("unsupported version of commands");
        return 1;
    }

    fread((char *)(&(cpu->program_size)), 4, 1, program_file);

    fseek(program_file, 0, SEEK_SET);

    cpu->program = (char *) calloc(cpu->program_size, sizeof(char));
    assert(cpu->program != nullptr);

    fread(cpu->program, cpu->program_size, sizeof(char), program_file);

    cpu->ip = BEGIN_PROGRAM_PTR;

    stack_create(&(cpu->stack));

    stack_create(&(cpu->calls));

    for (int i = 0; i < 5 ; i++) {
        cpu->REG[i] = 0;
    }

    for (size_t i = 0; i < MEM_W * MEM_H ; i++) {
        cpu->RAM[i] = 0;
    }

    return 0;
}

void get_jmp_param(int *a, int *b, int *arg, Cpu *cpu) {
    char cmd_param = cpu->program[(cpu->ip)++];
    char reg = 0;
    int inte = 0;

    if (cmd_param & REG_MASK) {
        reg = cpu->program[(cpu->ip)++];
        if (reg < 1 || reg > 4) {
            printf("error in reg number, number reg is %d in ip:%d", reg, cpu->ip - 2);
            assert(reg < 1 || reg > 4);
        }

        *arg = cpu->REG[reg];
    }

    if (cmd_param & INT_MASK) {
        for (size_t i = 0; i < sizeof(int); i++) {
            ((char *)(&inte))[i] = cpu->program[(cpu->ip)++];
        }

        if (cmd_param & REG_MASK) {
            *arg += inte;
        }
        else {
            *arg = inte;
        }
    }

    if (cmd_param & MEM_MASK) {
        *arg = cpu->RAM[*arg];
    }

    *b = stack_pop(&cpu->stack);
    *a = stack_pop(&cpu->stack); 
}

int * get_ptr_arg(Cpu *cpu) {
    int inte = 0;
    int *ret = 0;

    char cmd_param = cpu->program[(cpu->ip)++];

    if (cmd_param & REG_MASK) {
        ret = &cpu->REG[cpu->program[(cpu->ip)++]];
        inte = *ret;
    }

    if (cmd_param & INT_MASK) {
        inte = *((int *)(&cpu->program[cpu->ip]));
        cpu->ip += 4;

        if (cmd_param & REG_MASK) {
            inte += *ret;
        }

        cpu->REG[0] = inte;
        ret = &(cpu->REG[0]);
    }

    if (cmd_param & MEM_MASK) {
        ret = &(cpu->RAM[inte]);
    }

    return ret;
}