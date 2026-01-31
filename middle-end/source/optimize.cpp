#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>


#include "defs.h"


#define ZERO(node) isNum(node, 0)
#define ONE(node)  isNum(node, 1)

typedef enum {
    FOLD_NOT_CONST = 0,
    FOLD_CONST,
    FOLD_OPTIMIZED
} FoldStatus;


static FoldStatus foldConstants(OptimizerContext* context, AstNode* node, size_t tree_idx);

static bool simplifyOperations(OptimizerContext* context, AstNode* node, size_t tree_idx);
static bool simplifyDispatcher(OptimizerContext* context, AstNode* node, size_t tree_idx);

static bool simplifyAdd(OptimizerContext* context, AstNode* node, size_t tree_idx);
static bool simplifySub(OptimizerContext* context, AstNode* node, size_t tree_idx);
static bool simplifyMul(OptimizerContext* context, AstNode* node, size_t tree_idx);
static bool simplifyDiv(OptimizerContext* context, AstNode* node, size_t tree_idx);
static bool simplifyPow(OptimizerContext* context, AstNode* node, size_t tree_idx);

static bool setNodeToChild(OptimizerContext* context, AstNode* node, size_t tree_idx, bool is_left);
static void replaceWithChild(AstNode* parent, AstNode* child);
static bool setNodeToNum(OptimizerContext* context, AstNode* node, size_t tree_idx, double num);
static bool isNum(AstNode* node, double num);


typedef bool (*simplifierFunc)(OptimizerContext* context, AstNode* node, size_t tree_idx);
const simplifierFunc SIMPLIFIERS[OP_MAX_COUNT] = {
    [OP_ADD] = simplifyAdd,
    [OP_SUB] = simplifySub,
    [OP_MUL] = simplifyMul,
    [OP_DIV] = simplifyDiv,
    [OP_POW] = simplifyPow
};


void optimizeTree(OptimizerContext* context)
{
    OPTIMIZER_ASSERT(context);

    bool changed = true;
    while (changed) {
        FoldStatus status = foldConstants(context, context->root);
        bool simplified = simplifyOperations(context, context->root);
        changed = simplified || status == FOLD_OPTIMIZED;
    }  

    //TREE_DUMP(context, tree_idx, STATUS_OK, "source tree");
}


static FoldStatus foldConstants(OptimizerContext* context, AstNode* node)
{
    OPTIMIZER_ASSERT(context);
    
    if (!node)
        return FOLD_CONST;

    switch (node->type) {
        case NODE_OP: {
            FoldStatus left_res =  foldConstants(context, node->left);
            FoldStatus right_res = foldConstants(context, node->right);

            if ((left_res == FOLD_OPTIMIZED || left_res == FOLD_CONST) &&
                (right_res == FOLD_OPTIMIZED || right_res == FOLD_CONST)) {
                setNodeToNum(context, node, evaluateNode(context, node));
                return FOLD_OPTIMIZED;
            } else {
                return FOLD_NOT_CONST;
            }
        }
        case NODE_VAR: return FOLD_NOT_CONST;
        case NODE_NUM: return FOLD_CONST;
        default:       return FOLD_NOT_CONST;
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
    if (node->type != NODE_OP) {
        return changed;
    }
    bool current_changed = simplifyDispatcher(context, node);

    return changed || current_changed;
}


static bool simplifyDispatcher(OptimizerContext* context, AstNode* node)
{
    assert(context);

    switch (node->type) {
        case AST_NODE_OP_ADD: return simplifyAdd(context, node);
        case AST_NODE_OP_SUB:
        case AST_NODE_OP_MUL:
        case AST_NODE_OP_DIV:
        case AST_NODE_OP_POW:
        case AST_NODE_OP_NEG:
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

    if (ZERO(node->left)) {
        return setNodeToNum(context, node, 0);
    }
    if (ONE(node->right)) {
        return setNodeToChild(context, node, true);
    }

    return false;
}


static bool simplifyPow(OptimizerContext* context, AstNode* node
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