#ifndef STRING
#define STRING

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

struct KR_string {
    char *ptr = nullptr;
    char *ptr_end = nullptr;
};

int KR_puts(const char *str);

      char * KR_strchr(      char *str, int ch);
const char * KR_strchr(const char *str, int ch);

unsigned int KR_strlen (      char *s);
unsigned int KR_strlen (const char *s);

void KR_strcpy(char *t,       char *s);
void KR_strcpy(char *t, const char *s);

void KR_strncpy(char *t,       char *s, int n);
void KR_strncpy(char *t, const char *s, int n);

void KR_strcat(char *t,       char *s);
void KR_strcat(char *t, const char *s);

void KR_strncat(char *t,       char *s, int n);
void KR_strncat(char *t, const char *s, int n);

char * KR_fgets(char *s, int n, FILE *iop);

char * KR_strdup(      char *s);
char * KR_strdup(const char *s);

int KR_strcmp(char *s1, char *s2);

int KR_strcmp_letonly(char *s1, char *s2);

int KR_strcmp_letonly_rev(KR_string a1, KR_string a2);

void KR_strswap(char **s1, char **s2);

#endif