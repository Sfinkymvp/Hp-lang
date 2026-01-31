#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "analyzer/analyzer.h"
#include "colors.h"


static ListElement findSymbol(List* list, ListElement symbol);


void createAnalyzerContext(AnalyzerContext* analyzer, ParserContext* parser)
{
    assert(analyzer); PARSER_ASSERT(parser);

    analyzer->id_table = &parser->id_table;
    analyzer->source_map = parser->source_map;
    analyzer->main_found = false;
    analyzer->status = STATUS_OK;

    StackError status = stackConstructor(&analyzer->scope_stack);
    if (status == STACK_OUT_OF_MEMORY) {
        analyzer->status = STATUS_SYSTEM_OUT_OF_MEMORY;
    }
}


void deleteAnalyzerContext(AnalyzerContext* context)
{
    assert(context);

    while (context->scope_stack.size > 0) {
        popScope(context);
    }

    stackDestructor(&context->scope_stack);
}


void reportAnalyzerWarning(AnalyzerContext* context, AstNode* node, const char* warning_message)
{
    ANALYZER_ASSERT(context);

    if (node != NULL) {
        fprintf(stderr, "Line %zu: " YELLOW("Semantic warning: ") "%s\n",
            node->line, warning_message);
        fprintf(stderr, "%4zu |    %.*s\n", node->line, (int)context->source_map->lines[node->line - 1].length,
        context->source_map->lines[node->line - 1].start);
    } else {
        fprintf(stderr, YELLOW("Semantic warning: ") "%s\n", warning_message);
    }
}


void reportAnalyzerError(AnalyzerContext* context, AstNode* node, const char* error_message)
{
    ANALYZER_ASSERT(context);

    if (node != NULL) {
        fprintf(stderr, "Line %zu: " RED("Semantic error: ") "%s\n",
            node->line, error_message);
        fprintf(stderr, "%4zu |    %.*s\n", node->line, (int)context->source_map->lines[node->line - 1].length,
        context->source_map->lines[node->line - 1].start);
    } else {
        fprintf(stderr, RED("Semantic error: ") "%s\n", error_message);
    }

    context->status = STATUS_SEMANTIC_ERROR;
}


void pushScope(AnalyzerContext* context)
{
    ANALYZER_ASSERT(context);

    List* list = NULL;
    ListError list_status = listConstructor(&list);
    RETURN_LIST_ERROR(context, list_status);

    StackError stack_status = stackPush(&context->scope_stack, (StackElement)list);
    RETURN_STACK_ERROR(context, stack_status);
}


void popScope(AnalyzerContext* context)
{
    ANALYZER_ASSERT(context);

    StackElement temp = NULL;
    StackError stack_status = stackPop(&context->scope_stack, &temp);
    RETURN_STACK_ERROR(context, stack_status);

    List* list = (List*)temp;

    Node* current = list->head;
    while (current != NULL) {
        free(current->data);
        current = current->next;
    }

    listDestructor(list);
    free(list);
}


void createSymbol(AnalyzerContext* context, Symbol** symbol, AstNode* node, bool is_function, int arg_count)
{
    ANALYZER_ASSERT(context); assert(symbol); assert(node);

    *symbol = (Symbol*)calloc(1, sizeof(Symbol));
    if (*symbol == NULL) {
        context->status = STATUS_SYSTEM_OUT_OF_MEMORY;
        return;
    }

    (*symbol)->id_index = node->data.id_index;
    (*symbol)->is_function = is_function;
    (*symbol)->arg_count = arg_count;
    (*symbol)->line = node->line;
}


void declareSymbol(AnalyzerContext* context, ListElement symbol)
{
    ANALYZER_ASSERT(context);

    List* list = (List*)context->scope_stack.data[context->scope_stack.size - 1];
    assert(list);

    ListError list_status = listPushFront(list, symbol);
    if (list_status != LIST_OK) {
        free(symbol);
    }
    RETURN_LIST_ERROR(context, list_status);
}



ListElement findSymbolInCurrentScope(AnalyzerContext* context, ListElement symbol)
{
    ANALYZER_ASSERT(context);

    List* list = (List*)context->scope_stack.data[context->scope_stack.size - 1];
    assert(list);

    return findSymbol(list, symbol);
}


ListElement findSymbolGlobally(AnalyzerContext* context, ListElement symbol)
{
    ANALYZER_ASSERT(context);

    for (int depth = (int)context->scope_stack.size - 1; depth >= 0; depth--) {
        List* list = (List*)context->scope_stack.data[depth];
        assert(list);

        ListElement existing = findSymbol(list, symbol);
        if (existing != NULL) {
            return existing;
        }
    }
    
    return NULL;
}


static ListElement findSymbol(List* list, ListElement symbol)
{
    assert(list);

    Node* current_node = list->head;
    while (current_node != NULL) {
        if (((Symbol*)current_node->data)->id_index == ((Symbol*)symbol)->id_index) {
            return current_node->data;
        }
        current_node = current_node->next;
    }

    return NULL;
}