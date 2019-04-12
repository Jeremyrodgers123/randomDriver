//
//  rc4.h
//  rc4
//
//  Created by Jeremy Rodgers on 4/9/19.
//  Copyright © 2019 Jeremy Rodgers. All rights reserved.
//

#ifndef rc4_h
#define rc4_h

#include <stdio.h>
#include <stdlib.h>
typedef struct RC4{
    uint8_t arr [256];
    int size;
}RC4;

void createIdentityArr(const int size, uint8_t *arr){
    int i;
    for(i = 0; i < size; i++){
        arr[i] = i;
    }
}

int stringLen(char s []){
    int i, length;
    length = 0;
    i = 0;
    while(s[i] != '\0')
    {
        length++;
        i++;
        if(i > 100){
            printf("Something went wrong finding str length");
            exit(1);
        }
        
    }
    return length;
}

void createKey(char passcode [], uint8_t key []){
    int i, index;
    int passcodeLen;
    passcodeLen = stringLen(passcode);
    for(i = 0; i < passcodeLen; i++){
        index = i % 8;
        key[index] = key[index] ^ passcode[i] ;
    }
    //print(key, 8);
}
void print(struct RC4* rc4){
    int i;
    for(i = 0; i < rc4->size; i++){
        printf("%d ", rc4->arr[i]);
    }
}

void printS(char s [], int size){
    int i;
    for(i = 0; i < size; i++){
        printf("%c", s[i]);
    }
}

void swap(struct RC4* rc4, int a, int b){
    uint8_t temp = rc4->arr[a];
    rc4->arr[a] = rc4->arr[b];
    rc4->arr[b] = temp;
}

RC4 * initRC4(struct RC4 *rc4, char passcode []){
    rc4->size = 256;
    createIdentityArr(256, rc4->arr);
    print(rc4);
    
    uint8_t key [] = {0,0,0,0,0,0,0,0};
    
    createKey(passcode, key);
    int i, j;
    j = 0;
    for(i = 0; i < rc4->size; i++){
        j = (j + rc4->arr[i] + key[i % 8] ) % rc4->size;
        swap(rc4, i, j);
    }
    return rc4;
}

uint8_t * myEncrypt(struct RC4 *rc4, char message [], int size){
    int i, j, k, t;
    i = j = k = t = 0;
    //TODO: FREE
    unsigned char *cypherText = malloc(sizeof(char) * size);
    
    for(k = 0; k < size; k++){
        i = (i + 1) % 256;
        j = (j + 1) % 256;
        swap(rc4, i, j);
        int t = (rc4 -> arr[i] + rc4 -> arr[j]) % 256;
        cypherText[k] = rc4-> arr[t] ^ message [k];
    }
    return cypherText;
}

#endif /* rc4_h */
