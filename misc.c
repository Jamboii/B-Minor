#include "misc.h"

void print_tabs(int indent) {
    for (int i = 0; i < indent; i++) {
        fprintf(stdout, "   ");
    }
}