#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>


#include "defs.h"
#include "colors.h"
#include "optimizer.h"


#define ZERO(node) isNum(node, 0)
#define ONE(node)  isNum(node, 1)


typedef enum {
    FOLD_NOT_CONST = 0,
    FOLD_CONST,
    FOLD_OPTIMIZED
} FoldStatus;


static FoldStatus foldConstants(OptimizerContext* context, AstNode* node);
static int evaluateNode(OptimizerContext* context, AstNode* node);

static bool simplifyOperations(OptimizerContext* context, AstNode* node);
static bool simplifyDispatcher(OptimizerContext* context, AstNode* node);

static bool simplifyAdd(OptimizerContext* context, AstNode* node);
static bool simplifySub(OptimizerContext* context, AstNode* node);
static bool simplifyMul(OptimizerContext* context, AstNode* node);
static bool simplifyDiv(OptimizerContext* context, AstNode* node);
static bool simplifyPow(OptimizerContext* context, AstNode* node);
static bool simplifyNeg(OptimizerContext* context, AstNode* node);

static bool setNodeToChild(OptimizerContext* context, AstNode* node, bool is_left);
static void replaceWithChild(AstNode* parent, AstNode* child);
static bool setNodeToNum(OptimizerContext* context, AstNode* node, int num);
static bool isNum(AstNode* node, int value);


void optimizeTree(OptimizerContext* context)
{
    OPTIMIZER_ASSERT(context);

    bool changed = true;
    while (changed) {
        FoldStatus status = foldConstants(context, context->root);
        if (context->status != STATUS_OK) {
            return;
        }
        bool simplified = simplifyOperations(context, context->root);
        if (context->status != STATUS_OK) {
            return;
        }

        changed = simplified || status == FOLD_OPTIMIZED;
    }  
}


static FoldStatus foldConstants(OptimizerContext* context, AstNode* node)
{
    OPTIMIZER_ASSERT(context);
    
    if (!node)
        return FOLD_CONST;

    FoldStatus left_res =  foldConstants(context, node->left);
    FoldStatus right_res = foldConstants(context, node->right);

    switch (node->type) {
        case AST_NODE_OP_ADD:
        case AST_NODE_OP_SUB:
        case AST_NODE_OP_MUL:
        case AST_NODE_OP_DIV:
        case AST_NODE_OP_POW:
        case AST_NODE_OP_NEG: {
            if ((left_res == FOLD_OPTIMIZED || left_res == FOLD_CONST) &&
                (right_res == FOLD_OPTIMIZED || right_res == FOLD_CONST)) {
                int result = evaluateNode(context, node);
                if (context->status != STATUS_OK) {
                    return FOLD_NOT_CONST;
                }

                setNodeToNum(context, node, result);
                return FOLD_OPTIMIZED;
            } else {
                return FOLD_NOT_CONST;
            }
        }
        case AST_NODE_NUMBER: return FOLD_CONST;
        default:  {
            if (left_res == FOLD_OPTIMIZED || right_res == FOLD_OPTIMIZED) {
                return FOLD_OPTIMIZED;
            }
            return FOLD_NOT_CONST;
        }
    }
}


static int evaluateNode(OptimizerContext* context, AstNode* node)
{
    assert(context); assert(node); assert(node->right);

    if (node->type == AST_NODE_OP_NEG) {
        return -1 * node->right->data.int_value;
    }
    
    assert(node->left);
    int left = node->left->data.int_value;
    int right = node->right->data.int_value;

    switch (node->type) {
        case AST_NODE_OP_ADD: return left + right;
        case AST_NODE_OP_SUB: return left - right;
        case AST_NODE_OP_MUL: return left * right;
        case AST_NODE_OP_DIV: {
            if (right == 0) {
                fprintf(stderr, RED("Division error: ") "Division by zero\n");
                context->status = STATUS_OPTIMIZATION_ERROR;
                return 0;
            }
            return left / right;
        }
        case AST_NODE_OP_POW: return (int)pow(left, right);
        default: {
            fprintf(stderr, "invalid node type %d in %s\n", node->type, __FUNCTION__);
            context->status = STATUS_OPTIMIZATION_ERROR;
            return 0;
        }
    }
}


static bool simplifyOperations(OptimizerContext* context, AstNode* node)
{
    OPTIMIZER_ASSERT(context);
    
    if (!node) {
        return false;
    }

    bool changed = false;
    if (simplifyOperations(context, node->left) ||
        simplifyOperations(context, node->right)) {
        changed = true;
    }

    bool current_changed = simplifyDispatcher(context, node);

    return changed || current_changed;
}


static bool simplifyDispatcher(OptimizerContext* context, AstNode* node)
{
    assert(context);

    switch (node->type) {
        case AST_NODE_OP_ADD: return simplifyAdd(context, node);
        case AST_NODE_OP_SUB: return simplifySub(context, node);
        case AST_NODE_OP_MUL: return simplifyMul(context, node);
        case AST_NODE_OP_DIV: return simplifyDiv(context, node);
        case AST_NODE_OP_POW: return simplifyPow(context, node);
        case AST_NODE_OP_NEG: return simplifyNeg(context, node);
        default: return false;
    }
}


static bool simplifyAdd(OptimizerContext* context, AstNode* node)
{
    assert(context); assert(node);

    if (ZERO(node->left)) {
        return setNodeToChild(context, node, false);
    }
    if (ZERO(node->right)) {
        return setNodeToChild(context, node, true);
    }

    return false;
}


static bool simplifySub(OptimizerContext* context, AstNode* node)
{
    assert(context); assert(node);

    if (ZERO(node->right)) {
        return setNodeToChild(context, node, true);
    }

    return false;
}


static bool simplifyMul(OptimizerContext* context, AstNode* node)
{
    assert(context); assert(node);

    if (ZERO(node->left) || ZERO(node->right)) {
        return setNodeToNum(context, node, 0);
    }
    if (ONE(node->left)) {
        return setNodeToChild(context, node, false);
    }
    if (ONE(node->right)) {
        return setNodeToChild(context, node, true);
    }

    return false;
}


static bool simplifyDiv(OptimizerContext* context, AstNode* node)
{
    assert(context); assert(node);

    if (ZERO(node->right)) {
        fprintf(stderr, RED("Division error: ") "Division by zero\n");
        context->status = STATUS_OPTIMIZATION_ERROR;
        return false;
    }
    if (ZERO(node->left)) {
        return setNodeToNum(context, node, 0);
    }
    if (ONE(node->right)) {
        return setNodeToChild(context, node, true);
    }

    return false;
}


static bool simplifyPow(OptimizerContext* context, AstNode* node)
{
    assert(context); assert(node);

    if (ZERO(node->left)) {
        return setNodeToNum(context, node, 0);
    }
    if (ONE(node->left) || ZERO(node->right)) {
        return setNodeToNum(context, node, 1);
    }
    if (ONE(node->right)) {
        return setNodeToChild(context, node, true);
    }

    return false;
}

static bool simplifyNeg(OptimizerContext* context, AstNode* node)
{
    assert(context); assert(node);

    if (node->right->type == AST_NODE_OP_NEG) {
        setNodeToChild(context, node, false);
        setNodeToChild(context, node, false);
        return true;
    }

    return false;
}


static bool setNodeToChild(OptimizerContext* context, AstNode* node, bool is_left)
{
    assert(context); assert(node);

    if (is_left) {
        deleteSubtree(node->right);
        node->right = NULL;
        replaceWithChild(node, node->left);
    } else {
        deleteSubtree(node->left);
        node->left = NULL;
        replaceWithChild(node, node->right);
    }

    return true;
}


static void replaceWithChild(AstNode* parent, AstNode* child)
{
    assert(parent); assert(child);

    parent->type = child->type;
    parent->data.int_value = child->data.int_value;
    parent->left = child->left;
    parent->right = child->right;
    if (parent->left) parent->left->parent = parent;
    if (parent->right) parent->right->parent = parent;

    free(child);
}


static bool setNodeToNum(OptimizerContext* context, AstNode* node, int num)
{
    OPTIMIZER_ASSERT(context); assert(node);

    deleteSubtree(node->left);
    node->left = NULL;
    deleteSubtree(node->right);
    node->right = NULL;
    node->type = AST_NODE_NUMBER;
    node->data.int_value = num; 

    return true;
}


static bool isNum(AstNode* node, int value)
{
    assert(node);

    return node->type == AST_NODE_NUMBER && node->data.int_value == value;
}