#include <stdio.h>
#include "../io/io.h"
#include <assert.h>
#include "scpu.h"
#include <math.h>
#include <SFML/Graphics.hpp>

#define Type_t int
#include "../stack/stack.h"

void get_jmp_param(size_t *ip, int *a, int *b, int *arg, char *program, int *REG, int *RAM, Stack *stack);

int * get_ptr_arg(char *program, int *REG, int *RAM, size_t *ip);

int main(int argc, const char *argv[]) {
    sf::RenderWindow window(sf::VideoMode(320*4, 200*4), "Test Window");
    window.setFramerateLimit(10);
    sf::VertexArray pointmap(sf::Points, 320*4 * 200*4);

    const char *file_in_path = nullptr;

    get_infile_name_from_flug(&file_in_path, argc, argv);
    assert(file_in_path != nullptr);

    FILE *program_file = fopen(file_in_path, "rb");
    assert(program_file != nullptr);

    char *file_type = (char *)calloc(2, sizeof(char));
    assert(file_type != nullptr);

    fread(file_type, sizeof(char), 2, program_file);

    if (file_type[0] != 'K' || file_type[1] != 'C') {
        printf("NOT COMPILE TYPE FILE");
        return 0;
    }

    char command_version = 0;
    fread(&command_version, sizeof(char), 1, program_file);

    if ((COMMAND_VERSION) != ((unsigned int)(unsigned char)command_version)) {
        printf("unsupported version of commands");
        return 0;
    }

    size_t program_size = 0;
    fread((char *)(&program_size), 4, 1, program_file);

    fseek(program_file, 0, SEEK_SET);

    char *program = (char *) calloc(program_size, sizeof(char));
    assert(program != nullptr);

    fread(program, program_size, sizeof(char), program_file);

    size_t ip = BEGIN_PROGRAM_PTR;

    Stack stack = {};
    stack_create(&stack);

    Stack calls = {};
    stack_create(&calls);

    int REG[5] = {0};

    int RAM[320*200] = {0};

    while(true) {
        switch (program[ip++]) {
            #define DEF_CMD(CMD, NUM, ARG, CODE) case NUM: {CODE}

            #include "../commands/commands.h"

            #undef DEF_CMD

            default: {
                printf("command_not_found ip:%d", ip-1);
                return 0;
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
}

void get_jmp_param(size_t *ip, int *a, int *b, int *arg, char *program, int *REG, int *RAM, Stack *stack) {
    char cmd_param = program[(*ip)++];
    char reg = 0;
    int inte = 0;

    if (cmd_param & REG_MASK) {
        reg = program[(*ip)++];
        if (reg < 1 || reg > 4) {
            printf("error in reg number, number reg is %d in ip:%d", reg, ip - 2);
            assert(reg < 1 || reg > 4);
        }

        *arg = REG[reg];
    }

    if (cmd_param & INT_MASK) {
        for (size_t i = 0; i < sizeof(int); i++) {
            ((char *)(&inte))[i] = program[(*ip)++];
        }

        if (cmd_param & REG_MASK) {
            *arg += inte;
        }
        else {
            *arg = inte;
        }
    }

    if (cmd_param & MEM_MASK) {
        *arg = RAM[*arg];
    }

    *b = stack_pop(stack);
    *a = stack_pop(stack); 
}

int * get_ptr_arg(char *program, int *REG, int *RAM, size_t *ip) {
    int inte = 0;
    int *ret = 0;

    char cmd_param = program[(*ip)++];

    if (cmd_param & REG_MASK) {
        ret = &REG[program[(*ip)++]];
        inte = *ret;
    }

    if (cmd_param & INT_MASK) {
        inte = *((int *)(&program[*ip]));
        *ip += 4;

        if (cmd_param & REG_MASK) {
            inte += *ret;
        }

        REG[0] = inte;
        ret = &(REG[0]);
    }

    if (cmd_param & MEM_MASK) {
        ret = &(RAM[inte]);
    }

    return ret;
}