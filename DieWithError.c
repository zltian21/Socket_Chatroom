/*
    DieWithError.c
    Handling and Report Error
*/
#include <stdio.h> 
#include <stdlib.h>

void DieWithError(char *errorMessage) {
    perror(errorMessage);
    exit(1);
}