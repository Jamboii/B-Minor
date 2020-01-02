// Scanner, Parser and Pretty Printer, written by Georges Alsankary for CSE 40243

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "decl.h"
#include "stmt.h"
#include "expr.h"
#include "type.h"
#include "param_list.h"
#include "scope.h"

extern FILE *yyin;
extern int yylex();
extern char *yytext;
extern int yyparse();
extern int yylineno;
extern struct decl* parser_result;

int typerr = 0;
int reserr = 0;

/* Debugging flags*/
#ifdef YYDEBUG
    int yydebug = 0;
#endif

char* scanner_tokentotext(int t) {
    if (t == TOKEN_ERROR)           return "ERROR";
    else if (t == TOKEN_PLUS)       return "PLUS";
    else if (t == TOKEN_MINUS)      return "MINUS";
    else if (t == TOKEN_MULT)       return "MULT";
    else if (t == TOKEN_DIV)        return "DIV";
    else if (t == TOKEN_MODULUS)    return "MODULUS";
    else if (t == TOKEN_EXP)        return "EXP";    
    else if (t == TOKEN_INCREMENT)  return "INCREMENT";
    else if (t == TOKEN_DECREMENT)  return "DECREMENT";
    else if (t == TOKEN_NOT)        return "LOGIC_NOT";
    else if (t == TOKEN_AND)        return "LOGIC_AND";
    else if (t == TOKEN_OR)         return "LOGIC_OR";
    else if (t == TOKEN_ASSIGNMENT) return "ASSIGNMENT";
    else if (t == TOKEN_GE)         return "GE";
    else if (t == TOKEN_GT)         return "GT";
    else if (t == TOKEN_LE)         return "LE";
    else if (t == TOKEN_LT)         return "LT";
    else if (t == TOKEN_EQ)         return "EQ";
    else if (t == TOKEN_NEQ)        return "NEQ";
    else if (t == TOKEN_AUTO)       return "AUTO";
    else if (t == TOKEN_ELSE)       return "ELSE";
    else if (t == TOKEN_TRUE)       return "TRUE";
    else if (t == TOKEN_FALSE)      return "FALSE";
    else if (t == TOKEN_FOR)        return "FOR";
    else if (t == TOKEN_WHILE)      return "WHILE";
    else if (t == TOKEN_IF)         return "IF";
    else if (t == TOKEN_FUNCTION)   return "FUNCTION";
    else if (t == TOKEN_PRINT)      return "PRINT";
    else if (t == TOKEN_RETURN)     return "RETURN";
    else if (t == TOKEN_VOID)       return "VOID";
    else if (t == TOKEN_ARRAY)      return "ARRAY";
    else if (t == TOKEN_CHAR)       return "CHARACTER";
    else if (t == TOKEN_STRING)     return "STRING";
    else if (t == TOKEN_BOOLEAN)    return "BOOLEAN";
    else if (t == TOKEN_LEFTPAREN)  return "LEFT_PARENTHESIS";
    else if (t == TOKEN_RIGHTPAREN) return "RIGHT_PARENTHESIS";
    else if (t == TOKEN_LEFTSQ)     return "LEFT_SQUARE";
    else if (t == TOKEN_RIGHTSQ)    return "RIGHT_SQUARE";
    else if (t == TOKEN_LEFTCURL)   return "LEFT_CURLY";
    else if (t == TOKEN_RIGHTCURL)  return "RIGHT_CURLY";
    else if (t == TOKEN_IDENT)      return "IDENTIFIER";
    else if (t == TOKEN_STRING_LITERAL) return "STRING_LITERAL";
    else if (t == TOKEN_CHAR_LITERAL)   return "CHARACTER_LITERAL";
    else if (t == TOKEN_INT_LITERAL)    return "INTEGER_LITERAL";
    else if (t == TOKEN_INT)        return "INTEGER";
    else if (t == TOKEN_ERROR)      return "ERROR";
    else if (t == TOKEN_SEMICOLON)  return "SEMICOLON";
    else if (t == TOKEN_COLON)      return "COLON";
    else if (t == TOKEN_COMMA)      return "COMMA";  
    else return 0;
}

int main(int argc, char *argv[]) {
    yyin = fopen(argv[2], "r");
    if (!yyin) {
        printf(" error: could not open %s\n", argv[2]);
        return 1;
    }

    if (!strcmp(argv[1], "-scan")) {
        while(1) {
            int t = yylex();
            if (!t || t == TOKEN_EOF) {
                exit(0);
            }
            else if (t == TOKEN_ERROR) {
                fprintf(stderr, "scan error\n");
                exit(1);
            }
            else {
                if (t == TOKEN_STRING_LITERAL || t == TOKEN_CHAR_LITERAL) {}
                if (t == TOKEN_INT_LITERAL) {
                    printf("INTEGER_LITERAL %s\n", yytext);
                }
                else if (t == TOKEN_IDENT) {
                    printf("IDENTIFIER %s\n", yytext);
                }
                else if (t == TOKEN_CHAR_LITERAL) {
                    char BUFF[160];
                    // stripping single quotes
                    for (int i = 0; i < strlen(yytext); i++) {
                        BUFF[i] = yytext[i + 1];
                    }
                    BUFF[strlen(yytext) - 2] = '\0';

                    // stripping slashes
                    if (BUFF[0] == '\\') { // starts with backslash
                        if  ((BUFF[1] == 'n') || (BUFF[1] == '0') || (BUFF[1] == '\\')) { // is a special character
                            printf("CHAR_LITERAL %s\n", BUFF);
                        }
                        else {// is  like any other character
                            printf("CHAR_LITERAL %c\n", BUFF[1]);
                        }
                    }
                    printf("CHAR_LITERAL %s\n", BUFF);

                }
                else if (t == TOKEN_STRING_LITERAL) {
                    char BUFF[160]; // declaring and cleaning buffer
                    for (int i = 0; i < 160; i++) {
                        BUFF[i] = 0;
                    }
                    // stripping string from quotation marks
                    for (int i = 0; i < strlen(yytext); i++) {
                        BUFF[i] = yytext[i + 1];
                    }
                    BUFF[strlen(yytext) - 2] = '\0';

                    // handling escaped characters
                    printf("STRING_LITERAL ");
                    char* PTR = BUFF;
                    while (*PTR != '\0') {
                        switch(*PTR) {
                            case '\n':
                                fprintf(stderr, "scan error: cannot use literal newline. Use \\n instead\n");
                                return TOKEN_ERROR; 
                                break;
                            case '\\': // check if special case or normal cancel
                                PTR++;
                                switch(*PTR) {
                                    case 'n':
                                        printf("\\n");
                                        break;
                                    case '0':
                                        printf("\\0");
                                        break;
                                    case '\\':
                                        printf("\\\\");
                                        break;
                                    default:
                                        printf("%c", *PTR);
                                }
                                break;
                            default:
                            printf("%c", *PTR);
                        }
                        PTR++;
                    }
                    printf("\n");
                }
                else {
                    printf("%s\n", scanner_tokentotext(t));
                }
            }


        }
    }
    else if (!strcmp(argv[1], "-parse")) {
        // Declaring values
        int pvalue;

        // placing yyparse in pvalue and returning AST
        pvalue = yyparse();

        if (pvalue == 1) {
            fprintf(stderr, "parse failed on line %d\n", yylineno);
            exit(1);
        } else {
            printf("Parse successful!\n");
            exit(0);
        }
    }
    
    else if (!strcmp(argv[1], "-print")) {
        // Declaring values
        int pvalue;

        // placing yyparse in pvalue and returning AST
        pvalue = yyparse();

        if (pvalue == 1) {
            fprintf(stderr, "parse failed on line %d\n", yylineno);
            exit(1);
        } else {
            decl_print(parser_result, 0);
            exit(0);
        }
    }

    else if (!strcmp(argv[1], "-resolve")) {
        // Declaring values
        int pvalue;

        // placing yyparse in pvalue and returning AST
        pvalue = yyparse();

        if (pvalue == 1) {
            fprintf(stderr, "parse failed on line %d\n", yylineno);
            exit(1);
        } else {
            scope_enter();

            decl_resolve(parser_result, 1);
            if(reserr) {
                fprintf(stderr, "resolve error: %i resolve error(s)\n", reserr);
                exit(1);
            }
            exit(0);
        }
    }

    else if (!strcmp(argv[1], "-typecheck")) {
        // Declaring values
        int pvalue;

        // placing yyparse in pvalue and returning AST
        pvalue = yyparse();

        if (pvalue == 1) {
            fprintf(stderr, "parse failed on line %d\n", yylineno);
            exit(1);
        } else {
            scope_enter();
            decl_resolve(parser_result, 0);
            if(reserr) {
                fprintf(stderr, "resolve error: one or more resolve error(s)\n");
                exit(1);
            }

            decl_typecheck(parser_result);
            if (typerr) {
                fprintf(stderr, "type error: %i type error(s)\n", typerr);
                exit(1);
            } else {
                fprintf(stdout, "typecheck successful!\n");
                exit(0);
            }
        }
    }    

    else if (!strcmp(argv[1], "-codegen")) {
        if (!argv[3]) {
            fprintf(stderr, "error: please enter output file name\n");
            exit(1);
        }
        // Declaring values
        int pvalue;
        FILE* outfile;

        outfile = fopen(argv[3], "w+");

        // placing yyparse in pvalue and returning AST
        pvalue = yyparse();

        if (pvalue == 1) {
            fprintf(stderr, "parse failed on line %d\n", yylineno);
            exit(1);
        } else {
            scope_enter();
            decl_resolve(parser_result, 0);
            if(reserr) {
                fprintf(stderr, "resolve error: one or more resolve error(s)\n");
                exit(1);
            }
            decl_typecheck(parser_result);
            if (typerr) {
                fprintf(stderr, "type error: %i type error(s)\n", typerr);
                exit(1);
            }
            
            
            decl_codegen(parser_result, outfile);
            int fret = fclose(outfile);
	    if (fret) {
	        fprintf(stderr, "file error: file not outputted\n"); 
	    }
        }
    }

    else {
        fprintf(stderr, "wrong flag\n");
        exit(1);
    }

    exit(0);
}
