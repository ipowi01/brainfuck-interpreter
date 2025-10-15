#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {TOO_FEW_ARGS = 1, NO_FILE} errors;
typedef struct {
    unsigned char *data;
    size_t size;
    size_t capacity;
} Tape;

void tape_init(Tape *t) {
    t->capacity = 1024;      // initial size
    t->size = 0;
    t->data = malloc(t->capacity);
    memset(t->data, 0, t->capacity);
}

void tape_free(Tape *t) {
    free(t->data);
}

void tape_ensure(Tape *t, size_t index) {
    if (index >= t->capacity) {
        size_t new_capacity = t->capacity * 2;
        while (new_capacity <= index) new_capacity *= 2;
        t->data = realloc(t->data, new_capacity);
        memset(t->data + t->capacity, 0, new_capacity - t->capacity);
        t->capacity = new_capacity;
    }
    if (index >= t->size) t->size = index + 1;
}
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <brainfuck_file>\n", argv[0]);
        return TOO_FEW_ARGS;
    }

    // Load program
    FILE *file = fopen(argv[1], "r");
    if (!file) { perror("Error opening file"); return NO_FILE; }

    fseek(file, 0, SEEK_END);
    long code_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *code = malloc(code_size + 1);
    fread(code, 1, code_size, file);
    code[code_size] = '\0';
    fclose(file);

    // Precompute jump table
    int *jump = malloc(code_size * sizeof(int));
    int *stack = malloc(code_size * sizeof(int));
    int sp = 0;
    for (long i = 0; i < code_size; i++) {
        if (code[i] == '[') stack[sp++] = i;
        else if (code[i] == ']') {
            int start = stack[--sp];
            jump[start] = i;
            jump[i] = start;
        }
    }

    // Initialize tape
    Tape tape;
    tape_init(&tape);
    size_t ptr = 0;

    for (long ip = 0; ip < code_size; ip++) {
        switch (code[ip]) {
            case '>': ptr++; tape_ensure(&tape, ptr); break;
            case '<': if (ptr > 0) ptr--; break;
            case '+': tape.data[ptr]++; break;
            case '-': tape.data[ptr]--; break;
            case '.': putchar(tape.data[ptr]); break;
            case ',': tape.data[ptr] = getchar(); break;
            case '[': if (tape.data[ptr] == 0) ip = jump[ip]; break;
            case ']': if (tape.data[ptr] != 0) ip = jump[ip]; break;
        }
    }

    tape_free(&tape);
    free(code);
    free(jump);
    free(stack);
    return 0;
}