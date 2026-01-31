#ifndef STACK_H_
#define STACK_H_


#include <stdio.h>


const size_t START_CAPACITY = 4;


typedef void* StackElement;


typedef enum {
    STACK_OK = 0,
    STACK_OUT_OF_MEMORY,
    STACK_UNDERFLOW
} StackError;


typedef struct {
    StackElement* data;
    size_t size;
    size_t capacity;
} Stack;


StackError stackConstructor(Stack* stack);


void stackDestructor(Stack* stack);


StackError stackPush(Stack* stack, StackElement value);


StackError stackPop(Stack* stack, StackElement* value);


#endif // STACK_H_
