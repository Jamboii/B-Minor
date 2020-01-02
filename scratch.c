#include "scratch.h"
#include <stdio.h>
#include <stdlib.h>

int scratch_table[7]; // simple array that contains status

void scratch_init() {
    for (int i = 0; i < 7; i++) {
        scratch_table[i] = 0;
    }
}

int scratch_alloc() {
    /*Find unused register in table*/
    for (int i = 0; i < 7; i++) {
        if (!scratch_table[i]) { // if entry not in use
            scratch_table[i] = 1;
            return i;
        }
    }
    fprintf(stderr, "code generator error: ran out of scratch registers\n");
    exit(1);
    return -1; // error case

}


void scratch_free(int r) {
    if (r > 6 || r < 0) {
        fprintf(stderr, "code generator error: register number %i does not exist\n", r);
    }
    scratch_table[r] = 0;
}

const char* scratch_name(int r) {
    switch(r) {
        case 0:
            return "%rbx";
            break;
        case 1:
            return "%r10";
            break;
        case 2:
            return "%r11";
            break;
        case 3:
            return "%r12";
            break;
        case 4:
            return "%r13";
            break;
        case 5:
            return "%r14";
            break;
        case 6:
            return "%r15";
            break;
        default:
            return "ERR";
    }
    return "ERR";
}

const char* arg_name(int a) {
    switch(a) {
        case 0:
            return "%rdi";
            break;
        case 1:
            return "%rsi";
            break;
        case 2:
            return "%rdx";
            break;
        case 3:
            return "%rcx";
            break;
        case 4:
            return "%r8";
            break;
        case 5:
            return "%r9";
            break;
    }
}
