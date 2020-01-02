#include "decl.h"
#include "scope.h"
#include "label.c"
#include "scratch.h"
#include <string.h>
#include <stdio.h>

extern int typerr;
extern int reserr;
extern int yylineno;

int isvoid = 0;

struct decl *decl_create(char *name,
                         struct type *type,
                         struct expr *value,
                         struct stmt *code)
{
    struct decl *d = calloc(1, sizeof(*d));
    d->name = name;
    d->type = type;
    d->value = value;
    d->code = code;
    d->param_number = 0;

    return d;
}

void decl_delete(struct decl *d)
{
    if (!d)
        return;

    type_delete(d->type);
    expr_delete(d->value);

    if (d)
        free(d);
}

void decl_print(struct decl *d, int indent)
{
    if (!d)
        return;
    //print_tabs(indent);
    printf("%s:", d->name);
    type_print(d->type);
    if (d->type->kind == TYPE_ARRAY)
    {
        struct expr* ptr = d->value;
        printf(" = {");
        while (ptr) {
            exprs_print(ptr, 0);
            printf(", ");
            ptr = ptr->right;
        }
        printf("}");
    }
    else if (d->value)
    {
        printf(" = ");
        if (d->value->kind != EXPR_BOOL_LITERAL)
        {
            expr_print(d->value);
        }
        else
        {
            if (d->value->literal_value == 0)
            {
                printf("true");
            }
            else
            {
                printf("false");
            }
        }
    }

    if (d->type->kind == TYPE_FUNCTION)
    {
        if (d->code)
        {
            printf(" = {\n");
            stmt_print(d->code, indent + 1);
            printf("}\n");
        }
        else
        {
            printf(";");
        }
    }
    else
    {
        printf(";");
    }
    printf("\n");
    decl_print(d->next, indent);
}

void decl_resolve(struct decl *d, int print)
{
    if (!d)
        return;

    symbol_t kind = scope_level() > 1 ? SYMBOL_LOCAL : SYMBOL_GLOBAL;

    d->symbol = symbol_create(kind, type_copy(d->type), strdup(d->name));
    d->symbol->which = 0;

    expr_resolve(d->value);
    scope_bind(d->name, d->symbol);

    if (print)
    {
        if (kind == SYMBOL_GLOBAL)
        {
            printf("%s resolves to global %s\n", d->name, d->name);
        }
        else if (kind == SYMBOL_LOCAL)
        {
            printf("%s resolves to local %i\n", d->name, d->symbol->which);
        }
        else if (kind == SYMBOL_PARAM)
        {
            printf("%s resolves to parameter %i\n", d->name, d->symbol->which);
        }
    }

    if (d->type->kind == TYPE_FUNCTION)
    { // enter parameter list scope
        scope_function_enter();
        param_list_resolve(d->type->params, print);
        stmt_resolve(d->code, print);
        scope_exit();
    }

    decl_resolve(d->next, print);
}

void decl_typecheck(struct decl *d)
{
    if (!d)
        return;
    struct type *t = expr_typecheck(d->value);
    //fprintf(stdout, "typechecking decl of %s\n", d->name); fflush(stdout);
    if (d->type->kind == TYPE_FUNCTION)
    { // typecheck the function
        if (d->type->params)
        {
            if (!param_list_compare_decl(d->type->params, d->symbol->type->params))
            {
                fprintf(stderr, "type error: declaration of %s and its prototype have different types\n", d->name);
                typerr++;
            } // compare function definition with resolved name
        }
        stmt_return_typecheck(d);
        //stmt_return_typecheck_recursive(d->code, d);
    }
    else if (d->value)
    {
        if ((d->type->kind != TYPE_ARRAY) && (d->type->kind != TYPE_STRING))
        { // not an array nor string
            if (!type_compare(t, d->symbol->type))
            {
                fprintf(stderr, "type error: declaration of %s and initialization do not match types\n", d->name);
                typerr++;
            }
        }
        else if (d->value && d->type->kind == TYPE_STRING)
        { // this is a string declaration
            if (!type_compare(t, d->symbol->type))
            {
                fprintf(stderr, "type error: string %s is not set to a string literal or string type \n", d->name);
                typerr++;
            }
        }
        else if (d->type->kind == TYPE_ARRAY)
        {                 // array typechecking
            int iter = 0; // to count the numbers and check for size

            struct expr *eptr = d->value;

            while (eptr)
            {
                // compare each type to the individual expression
                iter++;
                if (!type_compare(expr_typecheck(eptr), d->type->subtype))
                { // type_compare(element, array)
                    fprintf(stderr, "type error: array type and item declaration do not match");
                    typerr++;
                }

                if (eptr->right)
                {
                    eptr = eptr->right;
                }
                else
                {
                    eptr = 0;
                }
            }
            if (d->type->size)
            {
                if ((iter != d->type->size) && iter)
                { // declared and assigned sizes are different
                    fprintf(stderr, "type error: array %s declaration has %i elements instead of %i\n", d->name, iter, d->type->size);
                    typerr++;
                }
            }
        }

        if (d->type->kind == TYPE_AUTO)
        { // Auto declaration
            if (d->value)
            {
                d->type = type_copy(expr_typecheck(d->value));
            }
            else
            {
                fprintf(stderr, "cannot assign type to auto if value is undeclared\n");
                typerr++;
            }
        }
        if (d->type->kind == TYPE_VOID && d->type->kind != TYPE_FUNCTION)
        {
            fprintf(stderr, "type error: cannot declare variable with type void\n");
            typerr++;
        }
    }

    stmt_typecheck(d->code);

    decl_typecheck(d->next);
}

void decl_codegen(struct decl *d, FILE *outfile)
{
    if (!d)
        return;
    // doing data section for globally declared scope
    // doing global scope variable declaration
    switch (d->symbol->kind)
    {
    case SYMBOL_GLOBAL: // checking for global data declaration
        switch (d->type->kind)
        {
        case TYPE_INTEGER:
        case TYPE_CHARACTER:
        case TYPE_BOOLEAN:
            fprintf(outfile, ".data\n");
            fprintf(outfile, ".global %s\n", d->name);
            if (d->value->literal_value) {
                fprintf(outfile, "%s: .quad %li\n", d->name, (long int)(d->value->literal_value));
            } else {
                fprintf(outfile, "%s: .quad 0\n", d->name);
            }
            break;
        case TYPE_STRING:
            fprintf(outfile, ".data\n");
            fprintf(outfile, ".global %s\n", d->name);
            fprintf(outfile, "%s: .string %s\n", d->name, d->value->string_literal);
            break;
        case TYPE_ARRAY:
            fprintf(outfile, ".data\n");
            struct expr *arrptr = d->value;
            if (d->type->subtype->kind == TYPE_STRING) {
                fprintf(stderr, "code generation error: arrays of strings not supported\n");
            } else if (d->type->subtype->kind == TYPE_STRING) {
                fprintf(stderr, "code generation error: multi-dimensional arrays not supported\n");
            } else  {
                struct expr* arrptr = d->value->right;
                char BUFFER[BUFSIZ];
                char name[BUFSIZ];
                sprintf(name, "%s: .quad %li", d->name, (long int)d->value->literal_value);
                while (arrptr)  {
                    sprintf(BUFFER, ", %li", (long int)arrptr->literal_value);
                    strcat(name, BUFFER);
                    arrptr = arrptr->right;
                }
                strcat(name, "\n");
                fputs(name, outfile);
            }
            break;
        case TYPE_FUNCTION:
            if (d->code)
            { // if no code, it's a preamble, which makes it useless for codegen, only used in type checking
                fprintf(outfile, ".text\n");
                fprintf(outfile, ".global %s\n", d->name);
                fprintf(outfile, "%s:\n", d->name); // emit label with function's name

                // preamble of function
                fprintf(outfile, "\tPUSHQ %%rbp\n");       // pushing base pointer
                fprintf(outfile, "\tMOVQ %%rsp, %%rbp\n"); // changing value of stack pointer to new frame

                struct param_list *ptr = d->type->params;
                
                int argctr = 0;
                int varctr = 0;
                while (ptr)
                { // pushing old values of argument regs into stack
                    fprintf(outfile, "\tPUSHQ %s\n", arg_name(argctr));
                    if (ptr->next) {
                        ptr = ptr->next;
                        argctr++;} else {
                        break;
                    }
                }

                d->param_number = argctr;

                // count number of local variables needed to allocate them - NOTE: make recursive for logic stuff
                struct stmt *stptr = d->code;
                
                while (stptr) {
                    if (stptr->kind == STMT_DECL) {
                        varctr++;
                    }
                    stptr = stptr->next;
                }
                fprintf(outfile, "\tSUBQ $%i, %%rsp\n", varctr * 8); // allocate additional local variables

                // indiscriminately save callee-saved registers
                fprintf(outfile, "\tPUSHQ %%rbx\n");
                fprintf(outfile, "\tPUSHQ %%r12\n");
                fprintf(outfile, "\tPUSHQ %%r13\n");
                fprintf(outfile, "\tPUSHQ %%r14\n");
                fprintf(outfile, "\tPUSHQ %%r15\n");

                // generating actual content of function
                stmt_codegen(d->code, outfile);

                // postamble of function
                fprintf(outfile, "\n.%s_epilogue:\n", d->name);

                // restore argument registers that were thrown in
                for (int j = argctr; j > 0; j--)
                {
                    fprintf(outfile, "\tPOPQ %s\n", arg_name(j));
                }
                // restore callee-saved registers
                fprintf(outfile, "\tPOPQ %%r15\n");
                fprintf(outfile, "\tPOPQ %%r14\n");
                fprintf(outfile, "\tPOPQ %%r13\n");
                fprintf(outfile, "\tPOPQ %%r12\n");
                fprintf(outfile, "\tPOPQ %%rbx\n");

                fprintf(outfile, "\tMOVQ %%rbp, %%rsp\n"); // reset stack to base pointer
                fprintf(outfile, "\tPOPQ %%rbp\n");        // restore old base pointer

                fprintf(outfile, "\tRET\n"); // return to caller - stuff to do return statemnts as well
            }
            break;
        }
        fprintf(outfile, "\n");
        break;
    case SYMBOL_LOCAL:
        switch (d->type->kind)
        {
        case TYPE_INTEGER:
        case TYPE_CHARACTER:
        case TYPE_BOOLEAN:
            expr_codegen(d->value, outfile); // generating code for expression, reg value will be placed in d->value->reg
            if (d->value)
            {
                fprintf(outfile, "\tMOVQ %s, %s\n", scratch_name(d->value->reg), symbol_codegen(d->symbol));
                scratch_free(d->value->reg); // d->value->reg is saved
            } else {
                fprintf(outfile, "\tMOVQ $0, %s\n", symbol_codegen(d->symbol)); // moving empty value into saved register
            }

            break;
        case TYPE_STRING:
            expr_codegen(d->value, outfile);
            if (d->value){
                fprintf(outfile, "\tMOVQ %s, %s\n", scratch_name(d->value->reg), symbol_codegen(d->symbol)); // moving addreesses around
            } else {
                fprintf(outfile, "\tMOVQ $0, %s\n", symbol_codegen(d->symbol)); // moving empty value into saved register
            }
            break;

        case TYPE_ARRAY:
            fprintf(stderr, "codegen error: cannot declare arrays in local scope\n");
            break;
        }
    }

    decl_codegen(d->next, outfile);
    return;
}
