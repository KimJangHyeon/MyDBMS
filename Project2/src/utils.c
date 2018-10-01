#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


void panic(char* error) {
        printf("panic: %s\n", error);
        exit(0);
}


