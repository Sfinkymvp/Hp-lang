#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "analyzer/analyzer.h"
#include "colors.h"


static void analyzeNode(AnalyzerContext* context, AstNode* node);
static void analyzeFunction(AnalyzerContext* context, AstNode* node);
static bool checkReturnPath(AnalyzerContext* context, AstNode* node);
static void analyzeCall(AnalyzerContext* context, AstNode* node);
static void analyzeDeclaration(AnalyzerContext* context, AstNode* node);
static void analyzeAssignment(AnalyzerContext* context, AstNode* node);
static void analyzeIdentifier(AnalyzerContext* context, AstNode* node);
static void analyzeReturn(AnalyzerContext* context, AstNode* node);
static void analyzeCycle(AnalyzerContext* context, AstNode* node);
static void analyzeIf(AnalyzerContext* context, AstNode* node);
static void analyzeElse(AnalyzerContext* context, AstNode* node);
static void analyzeScope(AnalyzerContext* context, AstNode* node);


void analyzeProgram(AnalyzerContext* context, AstNode* ast_root)
{
    ANALYZER_ASSERT(context); assert(ast_root);

    pushScope(context);
    RETURN_ANALYZER_ERROR(context);

    analyzeNode(context, ast_root);

    if (context->status == STATUS_OK && !context->main_found) {
        reportAnalyzerError(context, NULL, "The \"main\" function is missing.");
        // Ошибка отсутствия main
    }

    popScope(context);
}


static void analyzeNode(AnalyzerContext* context, AstNode* node)
{
    ANALYZER_ASSERT(context);

    if (node == NULL) {
        return; 
    }

    if (context->scope_stack.size == 1) {
        if (node->type == AST_NODE_CALL || node->type == AST_NODE_ASSIGNMENT ||
        node->type == AST_NODE_RETURN || node->type == AST_NODE_CYCLE ||
        node->type == AST_NODE_IF || node->type == AST_NODE_SCOPE) {
            reportAnalyzerError(context, node, "Only declarations and functions are allowed in global scope");
            return;
            // В глобальной области видимости могут быть только функции и глобальные переменные
        }
    } else if (node->type == AST_NODE_FUNCTION) {
        reportAnalyzerError(context, node, "Declaring a function inside another function");
        return;
        // Ошибка объявления функции в функции
    }

    switch (node->type) {
        case AST_NODE_FUNCTION:     analyzeFunction(context, node);      break;
        case AST_NODE_CALL:         analyzeCall(context, node);          break;
        case AST_NODE_DECLARATION:  analyzeDeclaration(context, node);   break;
        case AST_NODE_ASSIGNMENT:   analyzeAssignment(context, node);    break;
        case AST_NODE_IDENTIFIER:   analyzeIdentifier(context, node);    break;
        case AST_NODE_RETURN:       analyzeReturn(context, node);        break;
        case AST_NODE_CYCLE:        analyzeCycle(context, node);         break;
        case AST_NODE_IF:           analyzeIf(context, node);            break;
        case AST_NODE_ELSE:         analyzeElse(context, node);          break;
        case AST_NODE_SCOPE:        analyzeScope(context, node->right);  break;
        case AST_NODE_OP_NEG:       analyzeNode(context, node->right);   break;
        case AST_NODE_NUMBER:                                            break;
        case AST_NODE_COMMA: {
            fprintf(stderr, "Unexpected comma in a function %s", __FUNCTION__); break;
        }
        default: {
            analyzeNode(context, node->left);
            RETURN_ANALYZER_ERROR(context);

            analyzeNode(context, node->right);
        }
    }
}


static void analyzeFunction(AnalyzerContext* context, AstNode* node)
{
    ANALYZER_ASSERT(context); assert(node->left);

    AstNode* function_head = node->left;
    AstNode* function_name = function_head->left;

    int arg_count = 0;
    AstNode* parameter = function_head->right;
    while (parameter != NULL) {
        arg_count++;
        parameter = parameter->right;
    }

    Symbol* symbol = NULL;
    createSymbol(context, &symbol, function_name, true, arg_count);
    RETURN_ANALYZER_ERROR(context);

    Symbol* existing = (Symbol*)findSymbolGlobally(context, (ListElement)symbol);
    if (existing != NULL) {
        free(symbol);
        if (existing->is_function) {
            reportAnalyzerError(context, node, "Declaring a function that already exists");
            return;
            // Ошибка повторного объявления функции с тем же названием 
        } else {
            reportAnalyzerError(context, node, "Declaring a function whose name overlaps with a global variable");
            return;
            // Ошибка объявления функции с таким же названием, как у глобальной переменной
        }
    }
    if (strcmp("main", context->id_table->identifiers[symbol->id_index]) == 0) {
        context->main_found = true;
    }
    declareSymbol(context, (ListElement)symbol);
    RETURN_ANALYZER_ERROR(context);

    pushScope(context);
    RETURN_ANALYZER_ERROR(context);

    parameter = function_head->right;
    while (parameter != NULL) {
        createSymbol(context, &symbol, parameter, false, 0);
        RETURN_ANALYZER_ERROR(context);

        existing = (Symbol*)findSymbolInCurrentScope(context, (ListElement)symbol);
        if (existing != NULL) {
            free(symbol);
            reportAnalyzerError(context, node, "Identical names of function parameters");
            return;
            // Ошибка одинаковых имен параметров функции
        }
        existing = (Symbol*)findSymbolGlobally(context, (ListElement)symbol);
        if (existing != NULL && existing->is_function) {
            free(symbol);
            reportAnalyzerError(context, node, "The parameter name overlaps with the function");
            return;
            // Ошибка затенения функции именем параметра
        }

        declareSymbol(context, (ListElement)symbol);
        RETURN_ANALYZER_ERROR(context);

        parameter = parameter->right;
    }

    analyzeNode(context, node->right);
    popScope(context);
    RETURN_ANALYZER_ERROR(context);

    if (!checkReturnPath(context, node->right)) {
        printf("Node type: %d, line: %zu\n", node->type, node->line);
        reportAnalyzerError(context, node, "the function has no guaranteed return");
        return;
        // Ошибка отсутствия гарантированного return в функции
    }

}


static bool checkReturnPath(AnalyzerContext* context, AstNode* node)
{
    ANALYZER_ASSERT(context);

    if (node == NULL) {
        return false;
    }

    switch (node->type) {
        case AST_NODE_RETURN: {
            return true;
        }
        case AST_NODE_SEMICOLON: {
            return checkReturnPath(context, node->left) || checkReturnPath(context, node->right);
        }
        case AST_NODE_SCOPE: {
            return checkReturnPath(context, node->right);
        }
        case AST_NODE_IF: {
            if (node->right) {
                return checkReturnPath(context, node->left->right) && checkReturnPath(context, node->right);
            } else {
                return false;
            }
        }
        case AST_NODE_ELSE: {
            return checkReturnPath(context, node->right);
        }
        default: {
            return false;
        }
    }
}


static void analyzeCall(AnalyzerContext* context, AstNode* node) 
{
    ANALYZER_ASSERT(context); assert(node); assert(node->left);

    Symbol* symbol = NULL;
    createSymbol(context, &symbol, node->left, false, 0);
    RETURN_ANALYZER_ERROR(context);
    Symbol* existing = (Symbol*)findSymbolGlobally(context, (ListElement)symbol);
    free(symbol);

    if (existing == NULL) {
        reportAnalyzerError(context, node, "Called by an undeclared function");
        return;
        // Вызов необъявленной функции
    }
    if (!existing->is_function) {
        reportAnalyzerError(context, node, "Attempt to call a variable");
        return;
        // Вызов переменной, а не функции
    }

    AstNode* current_param = node->right;
    int arg_count = 0;
    while (current_param != NULL) {
        analyzeNode(context, current_param->left);
        RETURN_ANALYZER_ERROR(context);

        arg_count++;
        current_param = current_param->right;
    }

    if (existing->arg_count != arg_count) {
        reportAnalyzerError(context, node, "Incorrect number of arguments when calling a function");
        // Неверное количество аргументов функции 
    }
}


static void analyzeDeclaration(AnalyzerContext* context, AstNode* node)
{
    ANALYZER_ASSERT(context); assert(node); assert(node->left); assert(node->right);

    analyzeNode(context, node->right);
    RETURN_ANALYZER_ERROR(context);

    Symbol* new_symbol = NULL;
    createSymbol(context, &new_symbol, node->left, false, 0);
    RETURN_ANALYZER_ERROR(context);

    Symbol* existing = (Symbol*)findSymbolGlobally(context, new_symbol);
    if (existing != NULL) {
        if (existing->is_function) {
            free(new_symbol);
            reportAnalyzerError(context, node, "The parameter name overlaps with the function");
            return;
            // Ошибка затенения функции именем переменной
        } else {
            reportAnalyzerWarning(context, node, "Variable shading");
        }
    }
    existing = (Symbol*)findSymbolInCurrentScope(context, new_symbol);
    if (existing != NULL) {
        free(new_symbol);
        reportAnalyzerError(context, node, "Attempt to declare an already existing variable");
        return;
        // Повторное объявление переменной, уже существующей в текущей области
    }

    declareSymbol(context, new_symbol);
}


static void analyzeAssignment(AnalyzerContext* context, AstNode* node)
{
    ANALYZER_ASSERT(context); assert(node); assert(node->left); assert(node->right);

    analyzeNode(context, node->right);
    RETURN_ANALYZER_ERROR(context);

    Symbol* target_symbol = NULL;
    createSymbol(context, &target_symbol, node->left, false, 0);
    RETURN_ANALYZER_ERROR(context);

    Symbol* existing = (Symbol*)findSymbolGlobally(context, target_symbol);
    free(target_symbol);
    if (existing == NULL) {
        reportAnalyzerError(context, node, "Attempt to assign a value to a non-existent variable");
        // Ошибка присваивания несуществующей переменной
    }
    if (existing != NULL && existing->is_function) {
        reportAnalyzerError(context, node, "Attempt to assign a value to a function");
        // Ошибка присваивания функции значения
    }
}


static void analyzeIdentifier(AnalyzerContext* context, AstNode* node)
{
    ANALYZER_ASSERT(context); assert(node);

    Symbol* target_symbol = NULL;
    createSymbol(context, &target_symbol, node, false, 0);
    RETURN_ANALYZER_ERROR(context);

    Symbol* existing = (Symbol*)findSymbolGlobally(context, target_symbol);
    free(target_symbol);
    if (existing == NULL) {
        reportAnalyzerError(context, node, "Using a non-existent variable");
        // Ошибка использования несуществующей переменной
    }
    if (existing != NULL && existing->is_function) {
        reportAnalyzerError(context, node, "Attempt to use function name");
        // Ошибка использования имени функции
    }
}


static void analyzeReturn(AnalyzerContext* context, AstNode* node)
{
    ANALYZER_ASSERT(context); assert(node);
    
    analyzeNode(context, node->right);
}


static void analyzeCycle(AnalyzerContext* context, AstNode* node)
{
    ANALYZER_ASSERT(context); assert(node);

    analyzeNode(context, node->left);
    RETURN_ANALYZER_ERROR(context);

    analyzeScope(context, node->right);
}


static void analyzeIf(AnalyzerContext* context, AstNode* node)
{
    ANALYZER_ASSERT(context); assert(node);

    AstNode* if_head = node->left;
    analyzeNode(context, if_head->left);
    RETURN_ANALYZER_ERROR(context);

    analyzeScope(context, if_head->right);
    RETURN_ANALYZER_ERROR(context);

    analyzeNode(context, node->right);
}


static void analyzeElse(AnalyzerContext* context, AstNode* node)
{
    ANALYZER_ASSERT(context); assert(node);

    analyzeScope(context, node->right);
}


static void analyzeScope(AnalyzerContext* context, AstNode* node)
{
    ANALYZER_ASSERT(context); assert(node);

    pushScope(context);
    RETURN_ANALYZER_ERROR(context);

    analyzeNode(context, node);
    popScope(context);
}