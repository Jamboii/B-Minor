#include "label.h"
#include <stdio.h>
#include <stdlib.h>

int label_counter = 0;

int label_create() {
    // Increment global counter and return current value
    return label_counter++;
}

const char* label_name(int label) {
    // Return a label in string form, so that label 15 is represented as ".L15"
    char* str = malloc(sizeof(char)*5);
    sprintf(str, ".L%i", label);
    return str;

}