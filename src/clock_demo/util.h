#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

double get_avg(int64_t *array, int size){
    int64_t sum = 0;
    for(int i = 0; i < size; ++i){
        sum += array[i];
    }
    return (sum/(double)size);
}

// Borrowed from Stackoverflow
char* stringToBinary(char* s) {
    if(s == NULL) return 0; /* no input string */
    size_t len = strlen(s);
    char *binary = (char*) malloc(len*8 + 1); // each char is one byte (8 bits) and + 1 at the end for null terminator
    binary[0] = '\0';
    for(size_t i = 0; i < len; ++i) {
        char ch = s[i];
        for(int j = 7; j >= 0; --j){
            if(ch & (1 << j)) {
                strcat(binary,"1");
            } else {
                strcat(binary,"0");
            }
        }
    }
    return binary;
}

// And the inverse
char* binaryToString(char* s){
    if(s == NULL) return 0; /* no input string */
    size_t len = strlen(s);
    char *result = (char*) malloc(len/8 + 1); // 8 bits are one char and + 1 at the end for null terminator
    int i;
    for(i = 0; i < len/8; i+=1){
        char ch = 0;
        for(int j = 0; j < 8; ++j){
            ch |= ((s[(i<<3) + j]-48) << (7-j));
            //printf("%d << %d\n", s[i+j]-48, (7-j));
        }
        result[i] = ch;
        //printf("%c", ch);
    }
    result[i+1] = '\0';
    return result;
}