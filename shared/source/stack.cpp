#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../include/stack.h"


static StackError stackExpand(Stack* stack);


StackError stackConstructor(Stack* stack)
{
    assert(stack);

    stack->data = (StackElement*)calloc(START_CAPACITY, sizeof(StackElement));
    if (stack->data == NULL) {
        return STACK_OUT_OF_MEMORY;
    }

    stack->capacity = START_CAPACITY;
    stack->size = 0;

    return STACK_OK;
}


void stackDestructor(Stack* stack) 
{
    assert(stack);

    free(stack->data);
}


StackError stackPush(Stack* stack, StackElement data)
{
    assert(stack);

    if (stack->size == stack->capacity) {
        StackError error_code = stackExpand(stack);
        if (error_code != STACK_OK) {
            return error_code;
        }
    }

    stack->data[stack->size] = data;
    stack->size++;
    
    return STACK_OK;
}


StackError stackPop(Stack* stack, StackElement* value)
{
    assert(stack);

    if (stack->size == 0) {
        return STACK_UNDERFLOW;
    }

    stack->size--;
    *value = stack->data[stack->size];

    return STACK_OK;
}


static StackError stackExpand(Stack* stack)
{
    assert(stack);

    void* temp = realloc(stack->data, stack->capacity * 2 * sizeof(StackElement));
    if (temp == NULL) {
        return STACK_OUT_OF_MEMORY;
    }

    stack->data = (StackElement*)temp;
    stack->capacity *= 2;

    return STACK_OK;
}
