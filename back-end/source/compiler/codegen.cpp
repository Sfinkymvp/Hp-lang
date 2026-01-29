#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "compiler/compiler.h"


void generateExpression(BackendContext* context, AstNode* )
{
    if (current_type == OPERATOR) {
        assert(left); assert(right);
    }

    generateExpression(left);
    generateExpression(right);
    generateOperator(node);
}


generateNumber()
{
    fprintf(file, "PUSH %d", number);
}


generateOperator()
{
    switch (operator) {
        case ADD: fprintf(file, "ADD");
        case SUB: fprintf(file, "SUB");
        case MUL: fprintf(file, "MUL");
        case DIV: frpintf(file, "DIV");
        case POW: fprintf(file, "POW");
    }
}


void generateDeclaration(BackendContext* context, AstNode* )

