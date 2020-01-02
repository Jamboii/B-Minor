#include "symbol.h"
#include <stdio.h>
#include <string.h>


struct symbol * symbol_create( symbol_t kind, struct type *type, char *name ) {
    struct symbol* s = malloc(sizeof(*s));

    s->kind = kind;
    s->type = type_copy(type);
    s->name = strdup(name);

    return s;
}

struct symbol* symbol_copy(struct symbol* in) {
    struct symbol* s = symbol_create(in->kind, in->type, in->name);
    s->which = in->which;

    return s;
}

const char* symbol_codegen(struct symbol* s) {
    /* return a string which is a fragment of an instruction, representing
    the address computation needed for a given symbol
    */
   // first examine scope of a symbol
   // Global variables: name in assembly is same as in source language - if there is a global variable var:integer, then symbol should return var
   // local variables and function parameters: return an address computation that yields the position of that local parameter on the stack
   // Position 0 is at addr -8(%rbp), 1 is at -16(%rbp)

    char* str = malloc(sizeof(char)*10);

    switch(s->kind) {
        case SYMBOL_GLOBAL:
            return strdup(s->name); // simply return name of global variable
            break;
        case SYMBOL_PARAM: // use argument variables // works for second two
        case SYMBOL_LOCAL:
            sprintf(str, "-%i(%%rbp)", s->which*8);
            return str;
            break;
   }
}