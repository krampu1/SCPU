#include <stdio.h>
#include <assert.h>
#include <sys/stat.h>
#include "../string/string.h"
#include "io.h"

size_t get_file_size(FILE * ptr_file) {
    assert(ptr_file != nullptr);

    struct stat fileinfo = {};
    fstat(fileno(ptr_file), &fileinfo);

    return fileinfo.st_size;
}

size_t get_data_file(char **buff, const char *file_path) {
    assert(buff      != nullptr);
    assert(file_path != nullptr);


    FILE * ptrFile = fopen(file_path, "rb");
    assert(ptrFile != nullptr);

    size_t file_size = get_file_size(ptrFile);

    *buff = (char *)calloc(file_size + 2, sizeof(**buff));

    assert(buff != nullptr);

    fread(*buff, sizeof(char), file_size, ptrFile);
    (*buff)[file_size + 1] = (*buff)[file_size] = 0;

    fclose(ptrFile);

    return file_size;
}

size_t buff_to_text(KR_string **text, char *buff, size_t buff_size) {
    assert(text != nullptr);
    assert(buff != nullptr);

    size_t text_size = 1;

    for (size_t i = 0; i < buff_size; i++) {
        if (buff[i] == '\n') {
            text_size++;
        }
    }
    
    *text = (KR_string *)calloc(text_size + 1, sizeof(**text));
    
    assert(text != nullptr);

    ((*text)[text_size]).ptr = nullptr;

    ((*text)[0]).ptr = buff;
    
    size_t ptr = 1;

    KR_string *ptr_to_last = *text;

    assert(ptr_to_last != nullptr);

    for (size_t i = 0; i < buff_size + 1; i++) {
        if (ptr_to_last != nullptr && (buff[i] == '\r' || buff[i] == '\n' || buff[i] == 0)) {
            (*ptr_to_last).ptr_end = &buff[i];

            ptr_to_last = nullptr;
        }

        if (buff[i] == '\r') {
            buff[i] = 0;
        }
        if (i > 0 && buff[i - 1] == '\n') {
            buff[i - 1] = 0;

            ptr_to_last = &((*text)[ptr]);

            (*text)[ptr++].ptr = &buff[i];
        }
    }
    if (ptr_to_last != nullptr) {
        (*ptr_to_last).ptr_end = &buff[buff_size];
    }

    for (size_t i = 0; i < text_size; i++) {
        assert((*text)[i].ptr     != nullptr);
        assert((*text)[i].ptr_end != nullptr);
    }

    return text_size;
}

void get_outfile_name_from_flug(const char **file_path, int argc, const char *argv[]) {
    //åñëè åñòü íàçâàíèå ôàéëà ââîäà òî ôëàã -f òî÷íî íå ïîñëåäíèé, çíà÷èò ïåðåáèðàòü argc - 1 áåçñìûñëåííî
    //à òàê æå ýòî ïîìîæåò èçáåæàòü îøèáêè åñëè ôëàã óêàçàëè ïîñëåäíèì
    for (int i = 0; i < argc - 1; i++) {
        if (!strncmp(argv[i], "-o", 3)) {
            *file_path = argv[i + 1];
            break;
        }
    }
}

void get_infile_name_from_flug(const char **file_path, int argc, const char *argv[]) {
    //åñëè åñòü íàçâàíèå ôàéëà ââîäà òî ôëàã -f òî÷íî íå ïîñëåäíèé, çíà÷èò ïåðåáèðàòü argc - 1 áåçñìûñëåííî
    //à òàê æå ýòî ïîìîæåò èçáåæàòü îøèáêè åñëè ôëàã óêàçàëè ïîñëåäíèì
    for (int i = 0; i < argc - 1; i++) {
        if (!strncmp(argv[i], "-f", 3)) {
            *file_path = argv[i + 1];
            break;
        }
    }
}

void fprintf_buff(FILE *ptrfileout, char *buff, size_t buff_size) {
    assert(ptrfileout != nullptr);
    assert(buff       != nullptr);

    for (size_t i = 0; i < buff_size; i++) {
        if (buff[i] == '\0') {
            if (buff[i + 1] != '\0') {
                fprintf(ptrfileout, "\n");
            }
        }
        else {
            fprintf(ptrfileout, "%c", buff[i]);
        }
    }
}

size_t get_text_file(KR_string **text, const char *file_path) {
    assert(text      != nullptr);

    char *buff = nullptr;

    size_t buff_size = get_data_file(&buff, file_path);

    assert(buff != nullptr);

    size_t text_size = buff_to_text(text, buff, buff_size);

    assert(text != nullptr);

    return text_size;
}