%{
    #include "parser.h"
    #include <limits.h>
    
    int fileno();
%}

%option nounput
%option noinput
%option yylineno

DIGIT [0-9]
LETTER [a-zA-Z]
%%
(\t|\n|\r|\ )+                             /*skip whitespace*/
(\/\/.+)|"/*"([^*]|(\*+[^*/]))*\*+\/  /*C or C++ Style*/
\+\+                                    { return TOKEN_INCREMENT;   }
\+                                      { return TOKEN_PLUS;        }
-                                       { return TOKEN_MINUS;       }
--                                      { return TOKEN_DECREMENT;   }
\*                                      { return TOKEN_MULT;        }
\/                                      { return TOKEN_DIV;         }
%                                       { return TOKEN_MODULUS;     }
\^                                      { return TOKEN_EXP;         }
!                                       { return TOKEN_NOT;         }
&&                                      { return TOKEN_AND;         }
\|\|                                    { return TOKEN_OR;          }
\=                                      { return TOKEN_ASSIGNMENT;  }
\<                                      { return TOKEN_LT;          }
\<\=                                    { return TOKEN_LE;          }
\>                                      { return TOKEN_GT;          }
\>\=                                    { return TOKEN_GE;          } 
\=\=                                    { return TOKEN_EQ;          }
\!\=                                    { return TOKEN_NEQ;         }    
auto                                    { return TOKEN_AUTO;        }
else                                    { return TOKEN_ELSE;        }
true                                    { return TOKEN_TRUE;        }
false                                   { return TOKEN_FALSE;       }
for                                     { return TOKEN_FOR;         }
while                                   { return TOKEN_WHILE;       }
if                                      { return TOKEN_IF;          }
function                                { return TOKEN_FUNCTION;    }
print                                   { return TOKEN_PRINT;       }
return                                  { return TOKEN_RETURN;      }
void                                    { return TOKEN_VOID;        }
array                                   { return TOKEN_ARRAY;       }
boolean                                 { return TOKEN_BOOLEAN;     }
string                                  { return TOKEN_STRING;      }
char                                    { return TOKEN_CHAR;        }
integer                                 { return TOKEN_INT;          }
\(                                      { return TOKEN_LEFTPAREN;   }
\)                                      { return TOKEN_RIGHTPAREN;  }
\[                                      { return TOKEN_LEFTSQ;      }
\]                                      { return TOKEN_RIGHTSQ;     }
\{                                      { return TOKEN_LEFTCURL;    }
\}                                      { return TOKEN_RIGHTCURL;   }
;                                       { return TOKEN_SEMICOLON;   }
:                                       { return TOKEN_COLON;       }
,                                       { return TOKEN_COMMA;       }
\"([^"\\]|\\.|\\\n)*\"                  {   if (strlen(yytext) > 160) {
                                                fprintf(stderr, "scan error: string is longer than 160 characters\n"); return TOKEN_ERROR;
                                                }
                                            return TOKEN_STRING_LITERAL;
                                        }
'(\\?.?)'                               {   
                                            if (!strcmp(yytext, "''")) {fprintf(stderr, "scan error: cannot have empty character\n"); return TOKEN_ERROR;}
                                            else if (!strcmp(yytext, "'\\'")) {fprintf(stderr, "scan error: invalid character\n"); return TOKEN_ERROR;}
                                            
                                            char BUFF[160];
                                            // stripping single quotes
                                            for (int i = 0; i < strlen(yytext); i++) {
                                                BUFF[i] = yytext[i + 1];
                                            }
                                            BUFF[strlen(yytext) - 2] = '\0';

                                            // stripping slashes
                                            if (BUFF[0] == '\\') { // starts with backslash
                                                if  ((BUFF[1] == 'n') || (BUFF[1] == '0') || (BUFF[1] == '\\')) { // is a special character
                                                    return TOKEN_CHAR_LITERAL;
                                                }
                                                else {// is  like any other character
                                                    return TOKEN_CHAR_LITERAL;
                                                }
                                            }
                                            
                                            return TOKEN_CHAR_LITERAL;        
                                        }
{DIGIT}+                                {
                                            if (strtol(yytext, NULL, 10) == LONG_MIN)        {fprintf(stderr, "scan error: integer underflow\n"); return TOKEN_ERROR;}
                                            else if (strtol(yytext, NULL, 10) == LONG_MAX)   {fprintf(stderr, "scan error: integer overflow\n"); return TOKEN_ERROR;}
                                            else {return TOKEN_INT_LITERAL;}
                                        }                                       
({LETTER}|_)(({LETTER}|{DIGIT}|_)*)     {   if (strlen(yytext) > 160) {fprintf(stderr, "scan error: identifier longer than 160 characters\n"); return TOKEN_ERROR;}
                                            return TOKEN_IDENT;       
                                        } /*Starts w/ letter or underscore, ends with letter  underscore or digit*/
.                                       { fprintf(stderr, "scan error: %s is an invalid token\n", yytext); return TOKEN_ERROR;       }
%%
int yywrap() { return 1;         }