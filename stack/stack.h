//assert add + nodefine + noconst
#ifndef STACK_H
#define STACK_H

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

const char *log_file_path = "log.out";

enum Stack_default_values{
    DEFAULT_STACK_SIZE = 10,
    STACK_DEL_PTR      = 1337,
    POISON_CHAR        = 228,
    CANARY_CONST       = 0xDED1CA1D,
};

struct Stack{
    unsigned int canary_left;

    Type_t       *data;
    size_t       size;
    size_t       capacity;
    size_t       alloc;

    const char         *name;
    size_t              line;
    const char         *file;
    const char         *func;

    void         (* data_fprintf)(FILE *, void *);

    unsigned int hash;
    unsigned int data_hash;

    unsigned int canary_right;
};

static void default_data_fprintf(FILE *file, void *data);

static void *stack_recalloc(void *data, size_t data_size, size_t elem_size, size_t *alloc);

#ifdef HASH_PROT
static unsigned int calculate_hash(char *data, size_t data_size);

static bool hash_is_good(Stack *stack);
#endif

static bool stack_error(Stack *stack);

void _stack_create(Stack *stack, const char *name, size_t line, const char *file, const char *func);

static void _stack_dump(FILE *ptr_log_file, const Stack *stack, const char *name, const size_t line, const char *file, const char *func);

void stack_del(Stack* stack);

[[maybe_unused]] static void stack_change_out_funk(Stack *stack, void (* data_fprintf)(FILE *, void *));

[[maybe_unused]] Type_t stack_pop(Stack* stack);

[[maybe_unused]] size_t stack_push(Stack* stack, Type_t a);

#ifndef NDEBUG
    #define stack_assert(stack) if (stack_error(stack)) stack_dump(stack)
#else
    #define stack_assert(stack)
#endif

#define stack_dump(stack) {FILE *ptr_log_file = fopen(log_file_path, "a");                            \
                           _stack_dump(ptr_log_file, stack, #stack, __LINE__, __FILE__, __FUNCTION__);\
                           fclose(ptr_log_file);                                                      \
                          }

#define memory_alloc_error() {FILE *ptr_log_file = fopen(log_file_path , "a");                                                        \
                              fprintf(ptr_log_file, "memory allocation error in %s() at %s(%d)\n", __FUNCTION__, __FILE__, __LINE__);\
                              fclose(ptr_log_file);                                                                                   \
                             }

void _stack_create(Stack *stack, const char *name, size_t line, const char *file, const char *func) {
    assert(stack != nullptr);
    assert(name  != nullptr);
    assert(file  != nullptr);
    assert(func  != nullptr);
    
    stack->canary_left  = (unsigned int)CANARY_CONST;
    stack->canary_right = (unsigned int)CANARY_CONST;

    stack->alloc        = 0;
#ifdef CANARY_PROT
    stack->data         = (Type_t *)stack_recalloc((void *)sizeof(CANARY_CONST), DEFAULT_STACK_SIZE, sizeof(Type_t), &(stack->alloc));
#else
    stack->data         = (Type_t *)stack_recalloc(nullptr, DEFAULT_STACK_SIZE, sizeof(Type_t), &(stack->alloc));
#endif
    stack->size         = 0;
    stack->capacity     = DEFAULT_STACK_SIZE;

    stack->name         = name;
    stack->line         = line;
    stack->file         = file;
    stack->func         = func;

    stack->data_fprintf = default_data_fprintf;
    
    stack->hash         = 0;
    stack->data_hash    = 0;
#ifdef HASH_PROT
    stack->hash         = calculate_hash((char *)stack, sizeof(Stack));
    stack->data_hash    = calculate_hash((char *)(stack->data), DEFAULT_STACK_SIZE * sizeof(Type_t));
#endif
    
    stack_assert(stack);
}

#define stack_create(stack) \
        _stack_create(stack, #stack, (size_t)__LINE__, __FILE__, __FUNCTION__)
                            

static bool stack_error(Stack *stack) {
    if (stack == nullptr) return true;

    if (stack->size > stack->capacity) return true;

    if (stack->data == nullptr) return true;

    if (stack->data == (Type_t *)STACK_DEL_PTR) return true;   

    if (stack->capacity == 0) return true;
#ifdef CANARY_PROT
    if (stack->canary_left != CANARY_CONST) return true;

    if (stack->canary_right != CANARY_CONST) return true;

    if (*((unsigned int *)((char *)(stack->data) - sizeof(CANARY_CONST))) != CANARY_CONST) return true;

    if (*((unsigned int *)((char *)(stack->data) + stack->capacity * sizeof(Type_t))) != CANARY_CONST) return true;
#endif

#ifdef HASH_PROT
    if (!hash_is_good(stack)) return true;
#endif
    
    return false;
}

static void default_data_fprintf(FILE *file, void *data) {
    fprintf(file, "%p", data); // printf pointer
}

static void _stack_dump(FILE *ptr_log_file, const Stack *stack, const char *name, const size_t line, const char *file, const char *func) {
    assert(ptr_log_file != nullptr);

    fprintf(ptr_log_file, "error in %s() at %s(%Iu):\n", func, file, line);

    fprintf(ptr_log_file, "  Stack(%p) \"%s\"\n", stack, name);

    fprintf(ptr_log_file, "  \"%s\" at %s() at %s\n", stack->name, stack->func, stack->file);
    
    fprintf(ptr_log_file, "  {\n    size = %Iu\n", stack->size);
    fprintf(ptr_log_file, "    capacity = %Iu\n",  stack->capacity);
    fprintf(ptr_log_file, "    hash = %o\n",  stack->hash);
    fprintf(ptr_log_file, "    data hash = %o\n",  stack->data_hash);
    fprintf(ptr_log_file, "    data[%p]\n",        stack->data);

    if (stack->data != nullptr) {
        for (size_t i = 0; i < stack->capacity; i++) {
            if (i < stack->size) {
                fprintf(ptr_log_file, "    *[%Iu]=", i);
                stack->data_fprintf(ptr_log_file, (void *) (stack->data + i));
                fprintf(ptr_log_file, "\n");
            }
            else {
                if (*((char *)(stack->data + i)) == (char)POISON_CHAR) {
                    fprintf(ptr_log_file, "     [%Iu]=", i);
                    stack->data_fprintf(ptr_log_file, (void *) (stack->data + i));
                    fprintf(ptr_log_file, "(poison)\n");
                }
                else {
                    fprintf(ptr_log_file, "     [%Iu]=", i);
                    stack->data_fprintf(ptr_log_file, (void *) (stack->data + i));
                    fprintf(ptr_log_file, "\n");
                }
            }
        }
    }
    fprintf(ptr_log_file, "  }\n");
    
    fclose(ptr_log_file);
}

#ifdef HASH_PROT
static unsigned int calculate_hash(char *data, size_t data_size) {
    assert(data != nullptr);

    unsigned int hash = 0;

    for (size_t i = 0; i < data_size; i++) {
        hash += (unsigned char)data[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}
#endif

static void *stack_recalloc(void *data, size_t data_size, size_t elem_size, size_t *alloc) {
    assert(alloc != nullptr);

#ifdef CANARY_PROT
    data = (void *)realloc((void *)((char *)data - sizeof(CANARY_CONST)), data_size * elem_size + sizeof(CANARY_CONST) * 2);

    *((unsigned int *)data) = (unsigned int)CANARY_CONST;

    data = (void *)(((char *)data) + sizeof(CANARY_CONST));

    *((unsigned int *)((char *)data + data_size * elem_size)) = (unsigned int)CANARY_CONST;
#else
    data = (void *)realloc((void *)data, data_size * elem_size);
#endif

    for (size_t i = *alloc; i < data_size * elem_size; i++) {
        *((char *)data + i) = (char)POISON_CHAR;
    }

    *alloc = data_size * elem_size;

    return data;
}

#ifdef HASH_PROT
static void recalculate_stack_hash(Stack *stack) {
    assert(stack != nullptr);

    stack->hash      = 0;
    stack->data_hash = 0;
    stack->hash      = calculate_hash((char *)stack, sizeof(Stack));
    stack->data_hash = calculate_hash((char *)(stack->data), stack->capacity * sizeof(Type_t));
}

static bool hash_is_good(Stack *stack) {
    assert(stack != nullptr);

    unsigned int hash      = stack->hash;
    unsigned int data_hash = stack->data_hash;
    stack->hash      = 0;
    stack->data_hash = 0;

    recalculate_stack_hash(stack);

    if (hash == stack->hash && data_hash == stack->data_hash) {
        return true;
    }

    stack->hash      = hash;
    stack->data_hash = data_hash;
    
    return false;
}
#endif

[[maybe_unused]] static void stack_change_out_funk(Stack *stack, void (* data_fprintf)(FILE *, void *)) {
    assert(data_fprintf != nullptr);

    stack_assert(stack);

    stack->data_fprintf = data_fprintf;

#ifdef HASH_PROT
    recalculate_stack_hash(stack);
#endif
    
    stack_assert(stack);
}

static size_t stack_resize_ifneed(Stack *stack) {
    stack_assert(stack);

    size_t new_stack_capacity = stack->capacity;

    if (stack->size == new_stack_capacity || stack->size * 4 <= new_stack_capacity) {
        new_stack_capacity = stack->size * 2;
    }
    
    if (new_stack_capacity < DEFAULT_STACK_SIZE) {
        new_stack_capacity = DEFAULT_STACK_SIZE;
    }

    stack->data = (Type_t *)stack_recalloc(stack->data, new_stack_capacity,sizeof(Type_t), &(stack->alloc));

    if (stack->data == nullptr) {
        memory_alloc_error();
        return 0;
    }

    stack->capacity = new_stack_capacity;

#ifdef HASH_PROT
    recalculate_stack_hash(stack);
#endif

    stack_assert(stack);

    return stack->capacity;
}

[[maybe_unused]] size_t stack_push(Stack* stack, Type_t a) {
    stack_assert(stack);

    if (stack_resize_ifneed(stack) == 0) {
        return 0;
    }
    
    *(stack->data+(stack->size++)) = a;

#ifdef HASH_PROT
    recalculate_stack_hash(stack);
#endif

    stack_assert(stack);

    return stack->size;
}

[[maybe_unused]] Type_t stack_pop(Stack* stack) {
    stack_assert(stack);

    if (stack->size == 0) {
        stack_dump(stack);

        return 0;
    }

    Type_t elem = stack->data[stack->size-- - 1];
    
    *((char *)(&(stack->data[stack->size]))) = POISON_CHAR;

#ifdef HASH_PROT
    recalculate_stack_hash(stack);
#endif

    stack_resize_ifneed(stack);

    stack_assert(stack);

    return elem;
}

void stack_del(Stack* stack) {
    stack_assert(stack);

    stack_recalloc(stack->data, 0, sizeof(Type_t), &(stack->alloc));

    stack->data = (Type_t *)STACK_DEL_PTR;

    stack->size = 1;

    stack->capacity = 0;

#ifdef HASH_PROT
    recalculate_stack_hash(stack);
#endif
}

#endif