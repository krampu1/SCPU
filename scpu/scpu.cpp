//#define GRAPHICS

#include <stdio.h>
#include "../io/io.h"
#include <assert.h>
#include "scpu.h"
#include <math.h>
#include <SFML/Graphics.hpp>
#include <limits>
#include <strings.h>

#define Type_t int
#include "../stack/stack.h"

const size_t MEM_H = 200;

const size_t MEM_W = 320;

const size_t PIXEL_SIZE = 4;

const unsigned int FPS = 10;

const size_t REG_SIZE = 5;

struct Cpu {
    size_t program_size;
    char *program;
    size_t ip;
    Stack stack;
    Stack calls;
    int *REG;
    int *RAM;
};

void get_jmp_param(int *a, int *b, int *arg, Cpu *cpu);

int * get_ptr_arg(Cpu *cpu);

int init_cpu(Cpu *cpu, FILE* program_file);

void cpu_des(Cpu *cpu);

int do_cpu(Cpu *cpu);

void clean_window_buffer(sf::RenderWindow *window);

void window_pause(sf::RenderWindow *window, sf::VertexArray *pointmap);

int pop(Cpu *cpu);




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




#ifdef GRAPHICS
void clean_window_buffer(sf::RenderWindow *window) {
    assert(window != nullptr);

    if ((*window).isOpen()) {
        sf::Event event;
        while ((*window).pollEvent(event)) {
            if (event.type == sf::Event::Closed)
            (*window).close();
        }
    }
}

void window_pause(sf::RenderWindow *window, sf::VertexArray *pointmap) {
    assert(window   != nullptr);
    assert(pointmap != nullptr);

    while (window->isOpen()) {
        sf::Event event;
        while (window->pollEvent(event)) {
            if (event.type == sf::Event::Closed)
            window->close();
        }
        window->clear();
        window->draw(*pointmap);
        window->display(); //cringe
    }
}

#define CALC_W (a % (MEM_W * PIXEL_SIZE))

#define CALC_H (a / (MEM_W * PIXEL_SIZE)) % (MEM_H * PIXEL_SIZE)

void draw_window(sf::RenderWindow *window, Cpu *cpu, sf::VertexArray *pointmap) {
    assert(window   != nullptr);
    assert(cpu      != nullptr);
    assert(pointmap != nullptr);

    if (window->isOpen()) {
        for ( int a = 0; a < MEM_W * PIXEL_SIZE * MEM_H * PIXEL_SIZE; a++) {
            (*pointmap)[a].position = sf::Vector2f((float)(CALC_W), (float)(CALC_H));
            (*pointmap)[a].color = (sf::Color)(cpu->RAM[CALC_W / PIXEL_SIZE + CALC_H / PIXEL_SIZE * MEM_W]);
        }
        window->clear();
        window->draw(*pointmap);

        window->display();
    }
}
#endif

int do_cpu(Cpu *cpu) {
    assert(cpu != nullptr);
#ifdef GRAPHICS
    sf::RenderWindow window(sf::VideoMode(MEM_W * PIXEL_SIZE, MEM_H * PIXEL_SIZE), "Test Window");
    window.setFramerateLimit(10);
    sf::VertexArray pointmap(sf::Points, MEM_W * PIXEL_SIZE * MEM_H * PIXEL_SIZE);
#endif
    while(true) {
        bool end_flug = false;
        
        switch (cpu->program[cpu->ip++]) {
            #define DEF_CMD(CMD, NUM, ARG, CODE) case NUM: {CODE break;}

            #include "../commands/commands.h"

            #undef DEF_CMD

            default: {
                fprintf(stderr, "command_not_found ip:%Iu\n", cpu->ip - COMMAND_SIZE);
                return 1;
                break;
            }
        }

        if (end_flug) {
            break;
        }
#ifdef GRAPHICS
        clean_window_buffer(&window);
#endif

    }

#ifdef GRAPHICS
    window_pause(&window, &pointmap);
#endif
    printf("END PROGRAM");
    return 0;
}

void cpu_des(Cpu *cpu) {
    assert(cpu != nullptr);

    free(cpu->program);
}

int init_cpu(Cpu *cpu, FILE *program_file) {
    assert(program_file != nullptr);
    assert(cpu != nullptr);

    cpu->REG = (int *)calloc(REG_SIZE, sizeof(int));
    assert(cpu->REG != nullptr);

    cpu ->RAM = (int *)calloc(MEM_H * MEM_W, sizeof(int));
    assert(cpu->RAM != nullptr);

    char *file_type = (char *)calloc(2, sizeof(char));
    assert(file_type != nullptr);

    fread(file_type, sizeof(char), 2, program_file);

    if (file_type[0] != SIGN_FIRST || file_type[1] != SIGN_SECOND) {
        fprintf(stderr, "NOT COMPILE TYPE FILE");
        return 1;
    }

    char command_version = 0;
    fread(&command_version, sizeof(char), 1, program_file);

    if ((COMMAND_VERSION) != ((unsigned int)(unsigned char)command_version)) {
        fprintf(stderr, "unsupported version of commands");
        return 1;
    }

    fread((char *)(&(cpu->program_size)), 1, sizeof(size_t), program_file);

    fseek(program_file, 0, SEEK_SET);

    cpu->program = (char *) calloc(cpu->program_size, sizeof(char));
    assert(cpu->program != nullptr);

    fread(cpu->program, cpu->program_size, sizeof(char), program_file);

    cpu->ip = BEGIN_PROGRAM_PTR;

    stack_create(&(cpu->stack));

    stack_create(&(cpu->calls));

    memset((void *)cpu->REG, REG_SIZE, sizeof(int));

    memset((void *)cpu->RAM, MEM_W * MEM_H, sizeof(int));

    return 0;
}

void get_jmp_param(int *a, int *b, int *arg, Cpu *cpu) {
    assert(a   != nullptr);
    assert(b   != nullptr);
    assert(arg != nullptr);
    assert(cpu != nullptr);

    char cmd_param = cpu->program[(cpu->ip)++];
    char reg    = 0;
    int operand = 0;

    if (cmd_param & REG_MASK) {
        reg = cpu->program[(cpu->ip)++];
        if (reg < 1 || reg > 4) {
            fprintf(stderr, "error in reg number, number reg is %u in ip:%Iu", (unsigned int)reg, cpu->ip - COMMAND_AND_MASK_SIZE);
            assert(reg < 1 || reg > 4);
        }

        *arg = cpu->REG[(size_t)((unsigned char)reg)];
    }

    if (cmd_param & INT_MASK) {
        for (size_t i = 0; i < sizeof(int); i++) {
            ((char *)(&operand))[i] = cpu->program[(cpu->ip)++];
        }

        if (cmd_param & REG_MASK) {
            *arg += operand;
        }
        else {
            *arg = operand;
        }
    }

    if (cmd_param & MEM_MASK) {
        *arg = cpu->RAM[*arg];
    }

    *b = stack_pop(&cpu->stack);
    *a = stack_pop(&cpu->stack); 
}

int * get_ptr_arg(Cpu *cpu) {
    assert(cpu != nullptr);
    
    int operand = 0;
    int *ret    = 0;

    char cmd_param = cpu->program[(cpu->ip)++];

    if (cmd_param & REG_MASK) {
        ret = &cpu->REG[(size_t)((unsigned char)(cpu->program[(cpu->ip)++]))];
        operand = *ret;
    }

    if (cmd_param & INT_MASK) {
        operand = *((int *)(&cpu->program[cpu->ip]));
        cpu->ip += 4;

        if (cmd_param & REG_MASK) {
            operand += *ret;
        }

        cpu->REG[0] = operand;
        ret = &(cpu->REG[0]);
    }

    if (cmd_param & MEM_MASK) {
        ret = &(cpu->RAM[operand]);
    }

    return ret;
}

int pop(Cpu *cpu) {
    assert(cpu      != nullptr);

    if ((cpu->stack).size == 0) {
        fprintf(stderr, "pop error:stack size == 0, in ip %Iu\n", cpu->ip - COMMAND_SIZE);
    }
    else {
        return stack_pop(&(cpu->stack));
    }

    return 0;
}