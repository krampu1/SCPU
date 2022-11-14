#define GRAPHICS

#include "../io/io.h"
#include "../string/string.h"
#include <stdlib.h>
#include <cstring>
#include <ctype.h>
#include "../scpu/scpu.h"
#include <Windows.h>

const size_t MAX_MET_SIZE            = 20;

const size_t SIZE_SIGNATURE          = 2;

const size_t SIZE_COMMAND_VERSION    = 1;

const size_t SIZE_PROGRAM_SIZE_CONST = 8;

const size_t MAX_COMMAND_SIZE        = 30;

const size_t MAX_flag_NUMBER         = 100;

const char  *DEFAULT_OUT_FILE        = "a.ksc";

const size_t MAX_PROGRESS_BAR_LEN    = 100;

const unsigned char GRAY                      = 177;

const unsigned char BLACK                     = 219;

struct goto_flag{
    KR_string name;
    size_t ptr;
};

#define SET_CHAR(data) *cmd = data; cmd++; (*size)++;

#define FREE_TEXT   free(text[0].ptr);\
                    free(text);

void get_param(KR_string text, char *cmd, goto_flag *goto_flags, goto_flag *jmp, size_t *size);

size_t text_to_program(char **program, KR_string *text, size_t text_size);

void write_commands_to_file(const char *file_out_path, char *program, size_t commands_size);

char make_mask(bool reg, bool mem, bool operand);

bool get_arg(char *txt_ptr, char *param, char *reg, int *operand, goto_flag *goto_flags, goto_flag *jmp, bool *operand_ext);

size_t rid_char(char *begin, char *end, char ch);

void write_flags(goto_flag *flag_ptr, size_t program_ptr, char *str_begin, char *str_end);

char reg_to_num(char *str);

void write_additional_data(char **program_ptr, size_t program_size);

void iteration(char **program_ptr, size_t *program_size, goto_flag *goto_flags, goto_flag **end_flags, size_t text_size, KR_string *text);

bool get_flag(size_t *program_size, goto_flag **end_flags, char *txt_begin, char *txt_end);

bool eq_command(const char *str, const char *cmd, size_t cmd_size);

#define init_progres_bar(str)   printf("%s\n\r", str);                                               \
                                for (size_t i = 0; i < MAX_PROGRESS_BAR_LEN; i++) printf("%c", GRAY);\
                                printf("\r");

void write_progress_bar(size_t progress);

void end_bar();




int main(int argc, const char *argv[]) {
    KR_string *text = nullptr;

    const char *file_in_path = nullptr;
    get_infile_name_from_flag(&file_in_path, argc, argv);
    assert(file_in_path != nullptr);

    size_t text_size = get_text_file(&text, file_in_path);
    assert(text != nullptr);

    char *program = nullptr;

    size_t program_size = text_to_program(&program, text, text_size);
    assert(program != nullptr);

    const char *file_out_path = DEFAULT_OUT_FILE;

    get_outfile_name_from_flag(&file_out_path, argc, argv);
    assert(file_out_path != nullptr);
    
    write_commands_to_file(file_out_path, program, program_size);

    free(program);

    FREE_TEXT;
}




size_t text_to_program(char **program, KR_string *text, size_t text_size) {
    assert(text    != nullptr);
    assert(program != nullptr);

    goto_flag goto_flags[MAX_flag_NUMBER] = {0};

    goto_flag *end_flags = goto_flags;
    
    size_t program_size = SIZE_SIGNATURE + SIZE_COMMAND_VERSION + SIZE_PROGRAM_SIZE_CONST;

    iteration(nullptr, &program_size, goto_flags, &end_flags, text_size, text);

    *program = (char *)calloc(program_size, sizeof(char));
    assert(*program != nullptr);

    char *program_ptr = *program;

    write_additional_data(&program_ptr, program_size);

    iteration(&program_ptr, &program_size, goto_flags, &end_flags, text_size, text);

    return program_size;
}

bool get_arg(char *txt_ptr, char *param, char *reg, int *operand, goto_flag *goto_flags, goto_flag *jmp, bool *operand_ext) {
    assert(txt_ptr     != nullptr);

    assert(param       != nullptr);
    assert(reg         != nullptr);
    assert(operand     != nullptr);

    assert(goto_flags  != nullptr);
    assert(jmp         != nullptr);

    assert(operand_ext != nullptr);

    size_t n_s = 0;
    
    if (sscanf(txt_ptr, "%s%n", param, (int *)&n_s) <= 0) {return 1;}

    txt_ptr += n_s;

    if (isalpha(*param)) {
        *reg = reg_to_num(param);

        if (!(*reg)) {
            *operand = -1;
            for (goto_flag *it = goto_flags; it < jmp; it++) {
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

void get_param(KR_string text, char *cmd, goto_flag *goto_flags, goto_flag *jmp, size_t *size) {
    assert(cmd        != nullptr);

    assert(goto_flags != nullptr);
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
        if (get_arg(txt_ptr, param, &reg, &operand, goto_flags, jmp, &operand_ext)) {
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

void write_flags(goto_flag *flag_ptr, size_t program_ptr, char *str_begin, char *str_end) {
    assert(flag_ptr != nullptr);

    assert(str_begin != nullptr);
    assert(str_end   != nullptr);

    flag_ptr->name.ptr     = str_begin;
    flag_ptr->name.ptr_end = str_end;

    flag_ptr->ptr          = program_ptr;
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

bool get_flag(size_t *program_size, goto_flag **end_flags, char *txt_begin, char *txt_end) {
    assert(program_size != nullptr);
    assert(end_flags    != nullptr);
    assert(txt_begin    != nullptr);
    assert(txt_end      != nullptr);

    for (char *cmd_ptr = txt_begin; cmd_ptr <= txt_end; cmd_ptr++) {
        if (*cmd_ptr == ':') {
            write_flags(*end_flags, *program_size, txt_begin, cmd_ptr);
            (*end_flags)++;

            return true;
        }
    }

    return false;
}

void iteration(char **program_ptr, size_t *program_size, goto_flag *goto_flags, goto_flag **end_flags, size_t text_size, KR_string *text) {
    assert(program_size != nullptr);
    assert(goto_flags   != nullptr);
    assert(end_flags    != nullptr);
    assert(text         != nullptr);

    char cmd[MAX_COMMAND_SIZE] = {0};

    init_progres_bar("start iteration");

    for (size_t command_iterator = 0; command_iterator < text_size; command_iterator++) {
        if (program_ptr == nullptr && get_flag(program_size, end_flags, text[command_iterator].ptr, text[command_iterator].ptr_end)) {
            continue;
        }
        if (program_ptr != nullptr && KR_strchr(text[command_iterator].ptr, ':')) {
            continue;
        }

        size_t cmd_size = 0;

        #define DEF_CMD(CMD, NUM, ARG, CODE) if (eq_command(text[command_iterator].ptr, #CMD, sizeof(#CMD))) {                                   \
                                                cmd_size++;                                                                                      \
                                                if (program_ptr == nullptr) {                                                                    \
                                                    *cmd = NUM;                                                                                  \
                                                }                                                                                                \
                                                else {                                                                                           \
                                                    **program_ptr = NUM;                                                                         \
                                                }                                                                                                \
                                                if(ARG){                                                                                         \
                                                    if (program_ptr == nullptr) {                                                                \
                                                        get_param(text[command_iterator], cmd + 1, goto_flags, *end_flags, &cmd_size);           \
                                                    }                                                                                            \
                                                    else {                                                                                       \
                                                        get_param(text[command_iterator], (*program_ptr) + 1, goto_flags, *end_flags, &cmd_size);\
                                                    }                                                                                            \
                                                }                                                                                                \
                                            }

        #include "../commands/commands.h"

        #undef DEF_CMD

        if (program_ptr == nullptr) {
            *program_size += cmd_size;
        }
        else {
            *program_ptr += cmd_size;
        }

        write_progress_bar(((command_iterator + 1) * MAX_PROGRESS_BAR_LEN) / text_size);
    }

    end_bar();
}

bool eq_command(const char *str, const char *cmd, size_t cmd_size) {
    assert(str != nullptr);
    assert(cmd != nullptr);

    char read_cmd[MAX_COMMAND_SIZE] = {0};

    sscanf(str, "%s", read_cmd);

    return (strncmp(read_cmd, cmd, cmd_size) == 0);
}



void write_progress_bar(size_t progress) {
    static size_t lastprogress = 0;
    if (progress < lastprogress) {
        lastprogress = 0;
    }
    if (lastprogress != progress) {
        do {
            printf("%c", BLACK);
            lastprogress++;
        } while (lastprogress < progress);
    }
}

void end_bar() {
     printf("\rcomplit\n");
}