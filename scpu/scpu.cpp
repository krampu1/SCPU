//#define GRAPHICS

//#pragma GCC diagnostic ignored "-Weffc++"

#include <stdio.h>
#include "../io/io.h"
#include <assert.h>
#include "scpu.h"
#include <math.h>
#include <SFML/Graphics.hpp>
#include <limits>
#include <strings.h>
#include <string.h>

#define Type_t int
#include "../stack/stack.h"

struct Cpu {
    size_t program_size;
    char *program;
    size_t ip;
    Stack stack;
    Stack calls;
    int *REG;
    int *RAM;
};

struct Calloc_info {
    void *ptr;

    size_t      line;
    const char *funk;
    const char *file;
};

void get_jmp_param(int *a, int *b, int *arg, Cpu *cpu);

int * get_ptr_arg(Cpu *cpu);

int init_cpu(Cpu *cpu, FILE* program_file);

void cpu_des(Cpu *cpu);

int do_cpu(Cpu *cpu);

void clean_window_buffer(sf::RenderWindow *window);

void window_pause(sf::RenderWindow *window, sf::VertexArray *pointmap);

void draw_window(sf::RenderWindow *window, Cpu *cpu, sf::VertexArray *pointmap);

int pop(Cpu *cpu);


Calloc_info calloced[MAX_CALLOC] = {0};
size_t first_free_calloced = 0;

void * free_mas[MAX_CALLOC] = {0};
size_t first_free_in_free_mas = 0;


void * _my_calloc(size_t _NumOfElements, size_t _SizeOfElements, size_t line, const char *funk, const char *file);
#define my_calloc(_NumOfElements, _SizeOfElements) _my_calloc(_NumOfElements, _SizeOfElements, __LINE__, __FUNCTION__, __FILE__)

void my_free(void *ptr);

void check_calloced();

bool is_free(void *ptr);






int main(int argc, const char *argv[]) {
    const char *file_in_path = nullptr;

    get_infile_name_from_flag(&file_in_path, argc, argv);
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

    check_calloced();
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
        for (size_t a = 0; a < MEM_W * PIXEL_SIZE * MEM_H * PIXEL_SIZE; a++) {
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
        bool end_flag = false;
        
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

        if (end_flag) {
            break;
        }
#ifdef GRAPHICS
        clean_window_buffer(&window);
#endif

    }

#ifdef GRAPHICS
    window_pause(&window, &pointmap);
#endif
    printf("END PROGRAM\n");
    return 0;
}

void cpu_des(Cpu *cpu) {
    assert(cpu != nullptr);

    my_free(cpu->program);

    my_free(cpu->REG);

    my_free(cpu->RAM);

    stack_del(&(cpu->stack));

    stack_del(&(cpu->calls));
}

int init_cpu(Cpu *cpu, FILE *program_file) {
    assert(program_file != nullptr);
    assert(cpu != nullptr);

    cpu->REG = (int *)my_calloc(REG_SIZE, sizeof(int));
    assert(cpu->REG != nullptr);

    cpu ->RAM = (int *)my_calloc(MEM_H * MEM_W, sizeof(int));
    assert(cpu->RAM != nullptr);

    char *file_type = (char *)my_calloc(2, sizeof(char)); // 2 - count SIGN char
    assert(file_type != nullptr);

    fread(file_type, sizeof(char), 2, program_file); // 2 - count SIGN char

    if (file_type[0] != SIGN_FIRST || file_type[1] != SIGN_SECOND) {
        fprintf(stderr, "NOT COMPILE TYPE FILE");
        return 1;
    }

    my_free(file_type);

    char command_version = 0;
    fread(&command_version, sizeof(char), 1, program_file);

    if ((COMMAND_VERSION) != ((unsigned int)(unsigned char)command_version)) {
        fprintf(stderr, "unsupported version of commands");
        return 1;
    }

    fread((char *)(&(cpu->program_size)), 1, sizeof(size_t), program_file);

    fseek(program_file, 0, SEEK_SET);

    cpu->program = (char *) my_calloc(cpu->program_size, sizeof(char));
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

void * _my_calloc(size_t _NumOfElements, size_t _SizeOfElements, size_t line, const char *funk, const char *file) {
    if (first_free_calloced == MAX_CALLOC) {
        return nullptr;
    }

    calloced[first_free_calloced].ptr = calloc(_NumOfElements, _SizeOfElements);

    calloced[first_free_calloced].line = line;
    calloced[first_free_calloced].funk = funk;
    calloced[first_free_calloced].file = file;

    return calloced[first_free_calloced++].ptr;
}

void my_free(void * ptr) {
    if (first_free_in_free_mas == MAX_CALLOC) {
        return;
    }
    free_mas[first_free_in_free_mas++] = ptr;
    free(ptr);
}

void check_calloced() {
    for (size_t i = 0; i < MAX_CALLOC; i++) {
        if (!is_free(calloced[i].ptr) && calloced[i].ptr) {
            printf("not free: %p %Iu %s %s\n", calloced[i].ptr, calloced[i].line, calloced[i].funk, calloced[i].file);
        }
    }
}

bool is_free(void *ptr) {
    for (size_t i = 0; i < MAX_CALLOC; i++) {
        if (ptr == free_mas[i]) {
            return true;
        }
    }
    return false;
}