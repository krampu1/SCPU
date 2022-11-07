#define GRAPHICS

#include "../io/io.h"
#include "../string/string.h"
#include <stdlib.h>
#include <cstring>
#include <ctype.h>
#include "../scpu/scpu.h"

const size_t MAX_MET_SIZE            = 20;

const size_t SIZE_SIGNATURE          = 2;

const size_t SIZE_COMMAND_VERSION    = 1;

const size_t SIZE_PROGRAM_SIZE_CONST = 8;

const size_t MAX_COMMAND_SIZE        = 30;

const size_t MAX_FLUG_NUMBER         = 100;

const char  *DEFAULT_OUT_FILE        = "a.ksc";

struct goto_flug{
    KR_string name;
    size_t ptr;
};

#define SET_CHAR(data) *cmd = data; cmd++; (*size)++;

void get_param(KR_string text, char *cmd, goto_flug *goto_flugs, goto_flug *jmp, size_t *size);

size_t text_to_program(char **program, KR_string *text, size_t text_size);

void write_commands_to_file(const char *file_out_path, char *program, size_t commands_size);

char make_mask(bool reg, bool mem, bool operand);

bool get_arg(char *txt_ptr, char *param, char *reg, int *operand, goto_flug *goto_flugs, goto_flug *jmp, bool *operand_ext);

size_t rid_char(char *begin, char *end, char ch);

void write_flugs(goto_flug *flug_ptr, size_t program_ptr, char *str_begin, char *str_end);

char reg_to_num(char *str);

void write_additional_data(char **program_ptr, size_t program_size);

void first_iteration(size_t *program_size, goto_flug *goto_flugs, goto_flug **end_flugs, size_t text_size, KR_string *text);

void second_iteration(char **program_ptr, goto_flug *goto_flugs, goto_flug *end_flugs, size_t text_size, KR_string *text);

bool get_flug(size_t *program_size, goto_flug **end_flugs, char *txt_begin, char *txt_end);

bool eq_command(const char *str, const char *cmd, size_t cmd_size);




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

    const char *file_out_path = DEFAULT_OUT_FILE;

    get_outfile_name_from_flug(&file_out_path, argc, argv);
    assert(file_out_path != nullptr);
    
    write_commands_to_file(file_out_path, program, program_size);
}




size_t text_to_program(char **program, KR_string *text, size_t text_size) {
    assert(text    != nullptr);
    assert(program != nullptr);

    goto_flug goto_flugs[MAX_FLUG_NUMBER] = {0};

    goto_flug *end_flugs = goto_flugs;
    
    size_t program_size = SIZE_SIGNATURE + SIZE_COMMAND_VERSION + SIZE_PROGRAM_SIZE_CONST;

    first_iteration(&program_size, goto_flugs, &end_flugs, text_size, text);

    *program = (char *)calloc(program_size, sizeof(char));
    assert(*program != nullptr);

    char *program_ptr = *program;

    write_additional_data(&program_ptr, program_size);

    second_iteration(&program_ptr, goto_flugs, end_flugs, text_size, text);

    return program_size;
}

bool get_arg(char *txt_ptr, char *param, char *reg, int *operand, goto_flug *goto_flugs, goto_flug *jmp, bool *operand_ext) {
    assert(txt_ptr     != nullptr);

    assert(param       != nullptr);
    assert(reg         != nullptr);
    assert(operand     != nullptr);

    assert(goto_flugs  != nullptr);
    assert(jmp         != nullptr);

    assert(operand_ext != nullptr);

    size_t n_s = 0;
    
    if (sscanf(txt_ptr, "%s%n", param, (int *)&n_s) <= 0) {return 1;}

    txt_ptr += n_s;

    if (isalpha(*param)) {
        *reg = reg_to_num(param);

        if (!(*reg)) {
            *operand = -1;
            for (goto_flug *it = goto_flugs; it < jmp; it++) {
                if (KR_strcmp(param, it->name) == 0) {
                    *operand = (int)it->ptr;
                }
            }
            *operand_ext = true;
        }
    }
    else {
        if (!sscanf(param, "%d", operand)){
            fprintf(stderr, "error while reading param(int): %s", param);
        }
        else {
            *operand_ext = true;
        }
    }

    return 0;
}

void get_param(KR_string text, char *cmd, goto_flug *goto_flugs, goto_flug *jmp, size_t *size) {
    assert(cmd        != nullptr);

    assert(goto_flugs != nullptr);
    assert(jmp        != nullptr);

    assert(size       != nullptr);
    
    char txt[MAX_COMMAND_SIZE] = {0};
    
    char *txt_ptr = txt;
    char *txt_end = txt + (text.ptr_end - text.ptr);

    strncpy(txt, text.ptr, text.ptr_end - text.ptr + 1);

    size_t n = 0;

    char param[MAX_COMMAND_SIZE] = {0};
    sscanf(txt, "%s%n", param, (int *)&n);

    txt_ptr += n;

    bool mem = false;

    if (rid_char(txt_ptr, txt_end, '[')) { mem = true; }
    if (rid_char(txt_ptr, txt_end, ']')) { mem = true; }
    rid_char(txt_ptr, txt_end, '+');

    char reg  = 0;
    bool operand_ext = false;

    int operand = 0;

    for (int i = 0; i < 2; i++){
        if (get_arg(txt_ptr, param, &reg, &operand, goto_flugs, jmp, &operand_ext)) {
            break;
        }
    }

    char mask = make_mask(reg, mem, operand_ext);
    
    SET_CHAR(mask);

    if (reg) {
        SET_CHAR(reg);
    }
    if (operand_ext) {
        *((int *)cmd) = operand;
        cmd += sizeof(int);
        (*size) += sizeof(int);
    }
}

void write_commands_to_file(const char *file_out_path, char *program, size_t commands_size) {
    assert(file_out_path != nullptr);
    assert(program       != nullptr);

    FILE *ptr_file_out = fopen(file_out_path, "wb");
    assert(ptr_file_out != nullptr);

    fwrite(program, sizeof(char), commands_size, ptr_file_out);

    fclose(ptr_file_out);
}

char make_mask(bool reg, bool mem, bool operand_ext) {
    char mask = 0;

    if (reg) {
        mask |= REG_MASK;
    }
    if (mem) {
        mask |= MEM_MASK;
    }
    if (operand_ext) {
        mask |= INT_MASK;
    }

    return mask;
}

size_t rid_char(char *begin, char *end, char ch) {
    assert(begin != nullptr);
    assert(end   != nullptr);

    size_t count = 0;

    for (char *i = begin; i < end; i++) {
        if (*i == ch) {
            *i = ' ';
            count++;
        }
    }

    return count;
}

void write_flugs(goto_flug *flug_ptr, size_t program_ptr, char *str_begin, char *str_end) {
    assert(flug_ptr != nullptr);

    assert(str_begin != nullptr);
    assert(str_end   != nullptr);

    flug_ptr->name.ptr     = str_begin;
    flug_ptr->name.ptr_end = str_end;

    flug_ptr->ptr          = program_ptr;
}

char reg_to_num(char *str) {
    assert(str != nullptr);

    if      (strncmp("rax", str, 4) == 0) {return 1;}
    else if (strncmp("rbx", str, 4) == 0) {return 2;}
    else if (strncmp("rcx", str, 4) == 0) {return 3;}
    else if (strncmp("rdx", str, 4) == 0) {return 4;}
    return 0;
}

void write_additional_data(char **program_ptr, size_t program_size) {
    assert( program_ptr != nullptr);
    assert(*program_ptr != nullptr);

    *((*program_ptr)++) = SIGN_FIRST;
    *((*program_ptr)++) = SIGN_SECOND;
    *((*program_ptr)++) = COMMAND_VERSION;
    
    (**(size_t **)program_ptr) = program_size;
    *program_ptr += sizeof(size_t);
}

bool get_flug(size_t *program_size, goto_flug **end_flugs, char *txt_begin, char *txt_end) {
    assert(program_size != nullptr);
    assert(end_flugs    != nullptr);
    assert(txt_begin    != nullptr);
    assert(txt_end      != nullptr);

    for (char *cmd_ptr = txt_begin; cmd_ptr <= txt_end; cmd_ptr++) {
        if (*cmd_ptr == ':') {
            write_flugs(*end_flugs, *program_size, txt_begin, cmd_ptr);
            (*end_flugs)++;

            return true;
        }
    }

    return false;
}

void first_iteration(size_t *program_size, goto_flug *goto_flugs, goto_flug **end_flugs, size_t text_size, KR_string *text) {
    assert(program_size != nullptr);
    assert(goto_flugs   != nullptr);
    assert(end_flugs    != nullptr);
    assert(text         != nullptr);

    char cmd[MAX_COMMAND_SIZE] = {0};

    for (size_t i = 0; i < text_size; i++) {
        if (get_flug(program_size, end_flugs, text[i].ptr, text[i].ptr_end)) {
            continue;
        }

        size_t cmd_size = 0;

        #define DEF_CMD(CMD, NUM, ARG, CODE) if (eq_command(text[i].ptr, #CMD, sizeof(#CMD))) {            \
                                                *cmd = NUM;cmd_size++;                                         \
                                                if(ARG){                                                       \
                                                get_param(text[i], cmd + 1, goto_flugs, *end_flugs, &cmd_size);\
                                                }                                                              \
                                            }

        #include "../commands/commands.h"

        #undef DEF_CMD

        *program_size += cmd_size;
    }
}

void second_iteration(char **program_ptr, goto_flug *goto_flugs, goto_flug *end_flugs, size_t text_size, KR_string *text) {
    assert(program_ptr != nullptr);
    assert(goto_flugs  != nullptr);
    assert(end_flugs   != nullptr);
    assert(text        != nullptr);

    for (size_t i = 0; i < text_size; i++) {
        if (KR_strchr(text[i].ptr, ':')) {
            continue;
        }

        size_t cmd_size = 0;

        #define DEF_CMD(CMD, NUM, ARG, CODE)    if (eq_command(text[i].ptr, #CMD, sizeof(#CMD))) {                       \
                                                    **program_ptr = NUM;                                                      \
                                                    cmd_size++;                                                              \
                                                    if (ARG) {                                                               \
                                                        get_param(text[i], (*program_ptr) + 1, goto_flugs, end_flugs, &cmd_size);\
                                                        }                                                                    \
                                                    }

        #include "../commands/commands.h"

        #undef DEF_CMD

        *program_ptr += cmd_size;
    }
}

bool eq_command(const char *str, const char *cmd, size_t cmd_size) {
    assert(str != nullptr);
    assert(cmd != nullptr);

    char read_cmd[MAX_COMMAND_SIZE] = {0};

    sscanf(str, "%s", read_cmd);

    return (strncmp(read_cmd, cmd, cmd_size) == 0);
}