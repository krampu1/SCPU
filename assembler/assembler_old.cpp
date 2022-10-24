#include "../io/io.h"
#include "../string/string.h"
#include <stdlib.h>
#include <cstring>
#include <ctype.h>
#include "../scpu/scpu.h"

const size_t MAX_MET_SIZE      = 20;

size_t SIZE_SIGNATURE          = 2;

size_t SIZE_COMMAND_VERSION    = 1;

size_t SIZE_PROGRAM_SIZE_CONST = 8;

struct goto_flug{
    char *name;
    size_t ptr;
};

void get_args(KR_string text, char *cmd, goto_flug *goto_flugs, goto_flug *jmp, size_t *size);

size_t text_to_program(char **program, KR_string *text, size_t text_size);

void write_commands_to_file(const char *file_out_path, char *program, size_t commands_size);

int main(int argc, const char *argv[]) {
    KR_string *text = nullptr;

    const char *file_in_path = nullptr;
    get_infile_name_from_flug(&file_in_path, argc, argv);
    assert(file_in_path != nullptr);

    size_t text_size = get_text_file(&text, file_in_path);
    assert(text != nullptr);

    char *program = nullptr;

    size_t program_size = text_to_program(&program, text, text_size);
    assert(program != nullptr);

    const char *file_out_path = "a.ksc";

    get_outfile_name_from_flug(&file_out_path, argc, argv);
    assert(file_out_path != nullptr);
    
    write_commands_to_file(file_out_path, program, program_size);
    printf("2");
}

size_t text_to_program(char **program, KR_string *text, size_t text_size) {
    assert(text != nullptr);

    goto_flug goto_flugs[100] = {0};
    goto_flug *end_flugs = goto_flugs;
    
    size_t program_size = SIZE_SIGNATURE + SIZE_COMMAND_VERSION + SIZE_PROGRAM_SIZE_CONST;

    for (int i = 0; i < text_size; i++) {
        bool jmp_m = false;
        
        for (char *cmd_ptr = text[i].ptr; cmd_ptr <= text[i].ptr_end; cmd_ptr++) {
            if (*cmd_ptr == ':') {
                end_flugs->name = (char *)calloc(MAX_MET_SIZE, sizeof(char));
                assert(end_flugs->name != nullptr);

                char *name_ptr = end_flugs->name;

                //KR_strncpy(name_ptr, text[i].ptr, cmd_ptr - text[i].ptr);

                for (char *m_ptr = text[i].ptr; m_ptr < cmd_ptr; m_ptr++) {
                    *(name_ptr++) = *(m_ptr);
                }

                end_flugs->ptr = program_size;

                end_flugs++;

                jmp_m = true;
            }
        }

        if (jmp_m) {
            continue;
        }

        char *cmd = (char *)calloc(10, 1);

        size_t cmd_size = 0;

        #define DEF_CMD(CMD, NUM, ARG, CODE) if (strncmp(text[i].ptr, #CMD, sizeof(#CMD) - 1)==0) {          \
                                                *cmd = NUM;cmd_size++;                                       \
                                                if(ARG){                                                     \
                                                get_args(text[i], cmd + 1, goto_flugs, end_flugs, &cmd_size);\
                                                }}

        #include "../commands/commands.h"

        #undef DEF_CMD

        program_size += cmd_size;
        free(cmd);
    }

    *program = (char *)calloc(program_size, sizeof(char));
    assert(*program != nullptr);

    char *program_ptr = *program;

    *(program_ptr++) = 'K';
    *(program_ptr++) = 'C';
    *(program_ptr++) = COMMAND_VERSION;
    
    for (size_t i = 0; i < sizeof(size_t); i++) {
        *(program_ptr++) = ((char *)(&program_size))[i];
    }

    for (int i = 0; i < text_size; i++) {
        bool jmp_m = false;

        for (char *cmd_ptr = text[i].ptr; cmd_ptr <= text[i].ptr_end; cmd_ptr++) {
            if (*cmd_ptr == ':') {
                jmp_m = true;
            }
        }

        if (jmp_m) {
            continue;
        }

        size_t cmd_size = 0;

        #define DEF_CMD(CMD, NUM, ARG, CODE) if (strncmp(text[i].ptr, #CMD, sizeof(#CMD) - 1)==0) {*program_ptr = NUM;cmd_size++;if(ARG){get_args(text[i], program_ptr + 1, goto_flugs, end_flugs, &cmd_size);}}

        #include "../commands/commands.h"

        #undef DEF_CMD

        program_ptr += cmd_size;
    }

    return program_size;
}

void get_args(KR_string text, char *cmd, goto_flug *goto_flugs, goto_flug *jmp, size_t *size) {
    char *txt = (char *)calloc(text.ptr_end - text.ptr + 1, 1);
    char *txt_end = txt + (text.ptr_end - text.ptr);

    for (int i = 0; i < text.ptr_end - text.ptr + 1; i++) {
        txt[i] = text.ptr[i];
    }

    size_t n = 0;
    char *command = (char *)calloc(text.ptr_end - text.ptr + 1, 1);
    sscanf(txt, "%s%n", command, &n);

    txt += n;

    bool mem = false;

    for (char *p = txt; p <= txt_end; p++) {
        if (*p == '[' || *p == ']') {
            mem = true;
            *p = ' ';
        }
        else if (*p == '+') {
            *p = ' ';
        }
    }

    char reg  = 0;
    bool inte = false;

    int integer = 0;

    size_t n_s = 0;
    char *param = (char *)calloc(text.ptr_end - text.ptr + 1, 1);

    for (int i = 0; i < 2; i++){ 
        int rez = 0;
        if (sscanf(txt, "%s%n", param, &n_s) <= 0) {break;}
        txt += n_s;
        if (isalpha(*param)) {
            if (strncmp("rax", param, 4) == 0) {reg = 1;}
            else if (strncmp("rbx", param, 4) == 0) {reg = 2;}
            else if (strncmp("rcx", param, 4) == 0) {reg = 3;}
            else if (strncmp("rdx", param, 4) == 0) {reg = 4;}
            else {
                integer = -1;
                for (goto_flug *it = goto_flugs; it < jmp; it++) {
                    if (strncmp(param, it->name, MAX_MET_SIZE) == 0) {
                        integer = it->ptr;
                    }
                }
                inte = true;
            }
        }
        else{
            if (!sscanf(param, "%d", &integer)){
                printf("error while reading param(int): %s", param);
            }
            else {
                inte = true;
            }
        }
    }
    char mask = 0;
    if (reg) {
        mask |= REG_MASK;
    }
    if (mem) {
        mask |= MEM_MASK;
    }
    if (inte) {
        mask |= INT_MASK;
    }
    *cmd = mask;
    cmd++;
    (*size)++;
    if (reg) {
        *cmd = reg;
        cmd++;
        (*size)++;
    }
    if (inte) {
        for (int i = 0; i < 4; i++) {
            *cmd = ((char *)(&integer))[i];
            cmd++;
            (*size)++;
        }
    }
}

void write_commands_to_file(const char *file_out_path, char *program, size_t commands_size) {
    assert(program != nullptr);

    FILE *ptr_file_out = fopen(file_out_path, "wb");
    assert(ptr_file_out != nullptr);

    fwrite(program, sizeof(char), commands_size, ptr_file_out);

    fclose(ptr_file_out);
}