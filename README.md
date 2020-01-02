BMinor Compiler Project
-------------------------------

This repository contains the source code for a B-Minor compiler, [as described by Professor Douglas Thain](https://www3.nd.edu/~dthain/courses/cse40243/fall2019/). It was a semester long project for Notre Dame's [CSE 40243 - Compilers and Language Design](https://www3.nd.edu/~dthain/courses/cse40243/fall2019/). The compiler is mostly functional and has some issues, but I take particular pride in finishing a largely working compiler for the language.

The compiler handles parsing, scanning, typechecking and code generation. It has to be assembled, but it can compile alongside C functions

## Instructions  
To make compiler: `make bminor`  
To scan: `bminor -scan source.bminor`  
To parse: `bminor -parse source.bminor`  
To typecheck: `bminor -typecheck source.bminor`  
To generate assembly code: `bminor -codegen source.bminor sourcefile.s`  
To convert assembly into executable: `gcc -g sourcefile.s library.c -o program`
