#include <stdio.h>
#include <stdlib.h>
#include "./string.h"
#include <assert.h>
#include <ctype.h>

int KR_puts(const char *s) {
    assert(s != nullptr);

    int i = 0;
    for (;       s[i]; i++) {
        if (putc(s[i], stdout) == EOF) return EOF;
    }

    if (putc('\n', stdout) == EOF) return EOF;
    return i;
}

const char * KR_strchr(const char *s, int ch) {
    assert(s != nullptr);

    do {
        if  (*s == ch) return s;
    } while (*s++);
    
    return nullptr;
}

char * KR_strchr(char *s, int ch) {
    assert(s != nullptr);

    do {
        if  (*s == ch) return s;
    } while (*s++);

    return nullptr;
}

unsigned int KR_strlen (char *s) {
    assert(s != nullptr);

    char   *p = s;
    while (*p) p++;

    return (unsigned int)(p - s);
}

unsigned int KR_strlen (const char *s) {
    assert(s != nullptr);

    const char *p = s;
    while (    *p) p++;

    return (unsigned int)(p - s);
}

void KR_strcpy(char *s, char *ct) {
    assert(s  != nullptr);
    assert(ct != nullptr);

    while ((*s = *ct)) {
        s++;
        ct++;
    }
}

void KR_strcpy(char *s, const char *ct) {
    assert(s  != nullptr);
    assert(ct != nullptr);

    while ((*s = *ct)) {
        s++;
        ct++;
    }
}

void KR_strncpy(char *s, char *ct, int n) {
    assert(s  != nullptr);
    assert(ct != nullptr);

    for (int i = 0; i < n && (*s = *ct); i++) {
        s++;
        ct++;
    }
    *s = 0;
}

void KR_strncpy(char *s, const char *ct, int n) {
    assert(s  != nullptr);
    assert(ct != nullptr);

    for (int i = 0; i < n && (*s = *ct); i++) {
        s++;
        ct++;
    }
    *s = 0;
}

void KR_strcat(char *s, char *ct) {
    assert(s  != nullptr);
    assert(ct != nullptr);

    KR_strcpy(KR_strchr(s, '\0'), ct);
}

void KR_strcat(char *s, const char *ct) {
    assert(s  != nullptr);
    assert(ct != nullptr);

    KR_strcpy(KR_strchr(s, '\0'), ct);
}

void KR_strncat(char    *s, char  *ct, int n) {
    assert(s  != nullptr);
    assert(ct != nullptr);

    KR_strncpy(KR_strchr(s, '\0'), ct, n);
}

void KR_strncat(char *s, const char *ct, int n) {
    assert(s  != nullptr);
    assert(ct != nullptr);

    KR_strncpy(KR_strchr(s, '\0'), ct, n);
}

char * KR_fgets(char *s, int n, FILE *stream) {
    assert(s      != nullptr);
    assert(stream != nullptr);

    int c    = 0;
    char *cs = nullptr;

    cs = s;
    while (--n > 0 && (c = getc(stream)) != EOF)
        if ((*cs++ = (char)c) == '\n' ) break;

    if (cs != s && *(cs-1) == '\n') {
        *(--cs) = '\0';
    }
    else {
        *cs = '\0';
    }

    return (c == EOF && cs == s) ? nullptr : s;
}

char * KR_strdup(char *s) {
    assert(s != nullptr);

    char *p = (char *) calloc(KR_strlen(s) + 1, sizeof(*p));

    KR_strcpy(p, s);

    return p;
}

char * KR_strdup(const char *s) {
    assert(s != nullptr);

    char *p = nullptr;
    p = (char *) malloc(KR_strlen(s) + 1);

    KR_strcpy(p, s);

    return p;
}

int KR_strcmp(char *s1, char *s2) {
    assert(s1 != nullptr);
    assert(s2 != nullptr);

    int i = -1; // (-1)++ = 0 -> first element
    do {
        i++;
        if (s1[i] < s2[i]) {
            return -1;
        }
        else if (s1[i] > s2[i]) {
            return 1;
        }
    } while (s1[i] != 0 && s2[i] != 0);
    return 0;
}

int KR_strcmp(char *s1, KR_string s2) {
    assert(s1 != nullptr);

    assert(s2.ptr     != nullptr);
    assert(s2.ptr_end != nullptr);

    int i = 0; // (-1)++ = 0 -> first element
    while (s1[i] != 0 && s2.ptr + i < s2.ptr_end) {
        if (s1[i] < s2.ptr[i]) {
            return -1;
        }
        else if (s1[i] > s2.ptr[i]) {
            return 1;
        }
        i++;
    }
    if (s1[i] == 0 && s2.ptr + i == s2.ptr_end) {
        return 0;
    }
    return 1;
}

int KR_strcmp_letonly(char *s1, char *s2) {
    assert(s1 != nullptr);
    assert(s2 != nullptr);

    int dif = 0;
    
    do {
        while (!isalpha(*s1) && *s1 != 0) s1++;
        while (!isalpha(*s2) && *s2 != 0) s2++;

        dif = ((int)((unsigned int)*s1) - (int)((unsigned int)*s2));
    } while (*s1++ != 0 && *s2++ != 0 && dif == 0);

    return dif;
}

int KR_strcmp_letonly_rev(KR_string a1, KR_string a2) {
    char *s1 = a1.ptr;
    char *s2 = a2.ptr;

    assert(s1 != nullptr);
    assert(s2 != nullptr);

    char *p1 = a1.ptr_end;
    char *p2 = a2.ptr_end;

    assert(p1 != nullptr);
    assert(p2 != nullptr);

    int dif = 0;

    while (p1 >= s1 && !isalpha(*p1)) p1--;
    while (p2 >= s2 && !isalpha(*p2)) p2--;

    while (dif == 0 && p1 > s1 && p2 > s2) {
        dif = (int)((unsigned int)*p1) - (int)((unsigned int)*p2);

        p1--; p2--;
        while (p1 >= s1 && !isalpha(*p1)) p1--;
        while (p2 >= s2 && !isalpha(*p2)) p2--;
    }
    
    if (dif == 0) {
        if (p1 < s1 && p2 < s2) {
            return 0;
        }
        if (p1 < s1) {
            return -1;
        }
        if (p2 < s2) {
            return 1;
        }
    }
    return dif;
}

void KR_strswap(char **s1, char **s2) {
    assert(s1 != nullptr);
    assert(s1 != nullptr);

    char *b = *s1;
    *s1 = *s2;
    *s2 = b;
}