#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "io/reader.h"
#include "ast_dump/html_generator.h"
#include "parser/utils.h"
#include "parser/ast.h"
#include "parser/parser.h"
#include "colors.h"
#include "status.h"


void createParserContext(ParserContext* parser, LexerContext* lexer)
{
    assert(parser); LEXER_ASSERT(lexer);

    parser->args = lexer->args;
    parser->source_map = &lexer->source_map;
    parser->tokens_array = &lexer->tokens_array;
    parser->current_token = 0;
    parser->status = STATUS_OK;

    parser->status = createIdentifierTable(&parser->id_table);
    if (parser->status != STATUS_OK) {
        return;
    }
    
    parser->dump_file = openAstDumpFile();
}


void deleteParserContext(ParserContext* context)
{
    assert(context);

    deleteIdentifierTable(&context->id_table);
    context->current_token = 0;

    if (context->dump_file != NULL) {
        fclose(context->dump_file);
    }
}


void reportParserError(ParserContext* context, TokenType expected_type, const char* error_message)
{
    PARSER_ASSERT(context);

    Token* current_token = &context->tokens_array->tokens[context->current_token];
    size_t current_line = current_token->line;
    const char* error_start = current_token->start;

    const char* line_start = context->source_map->lines[current_line - 1].start;
    const size_t line_length = context->source_map->lines[current_line - 1].length;

    size_t error_offset = (size_t)(error_start - line_start);

    if (expected_type != TOKEN_UNKNOWN) {
        fprintf(stderr, "Line %zu: Column %zu: " RED("Syntax error: ")
            "expected ", current_line, error_offset);
        
        const char* keyword_string = getKeywordString(expected_type);
        if (keyword_string) {
            fprintf(stderr, CYAN("'%s'"), keyword_string);
        } else {
            if (expected_type == TOKEN_NUMBER) {
                fprintf(stderr, CYAN("Number"));
            } else {
                fprintf(stderr, CYAN("Identifier"));
            }
        }

        fprintf(stderr, ", but founded " RED("'%.*s'") "\n", (int)current_token->length, current_token->start);
    } else {
        fprintf(stderr, "Line %zu: Column %zu: " RED("Syntax error") "\n", current_line, error_offset);
    }
    if (error_message[0] != '\0') {
        fprintf(stderr, "    %s\n", error_message);
    }

    fprintf(stderr, "%4zu |    %.*s", current_line, (int)error_offset, line_start);
    fprintf(stderr, RED("%.*s"), (int)current_token->length, error_start);
    
    size_t remaining_length = line_length - (error_offset + current_token->length);
    fprintf(stderr, "%.*s\n", (int)remaining_length, error_start + current_token->length);

    fprintf(stderr, "          ");
    for (size_t index = 0; index < error_offset; index++) {
        if (line_start[index] == '\t') {
            fprintf(stderr, "\t");
        } else {
            fprintf(stderr, " ");
        }
    }

    fprintf(stderr, RED("^\n"));

    context->status = STATUS_SYNTAX_ERROR;
}


AstNode* makeNode(ParserContext* context, NodeType type, size_t line, AstNode* left, AstNode* right)
{
    PARSER_ASSERT(context);

    AstNode* node = NULL;
    context->status = createNode(&node);
    if (context->status != STATUS_OK) {
        return NULL;
    }

    node->type = type;
    node->left = left;
    node->right = right;
    node->data.id_index = 0;
    node->line = line;

    if (left) {
        left->parent = node;
    }
    if (right) {
        right->parent = node;
    }

    return node;
}


size_t addIdentifier(ParserContext* context)
{
    PARSER_ASSERT(context);

    Token* token = &context->tokens_array->tokens[context->current_token];
    assert(token); assert(token->start); assert(token->type == TOKEN_IDENTIFIER);

    for (size_t index = 0; index < context->id_table.count; index++) {
        if (strncmp(token->start, context->id_table.identifiers[index], token->length) == 0 &&
            context->id_table.identifiers[index][token->length] == '\0') {
            return index;
        }
    }

    if (context->id_table.count == context->id_table.capacity) {
        expandIdentifierTable(&context->id_table);
        if (context->status != STATUS_OK) {
            return (size_t)-1;
        }
    }

    size_t new_index = context->id_table.count;
    char* identifier = context->id_table.identifiers[new_index];

    identifier = (char*)calloc(token->length + 1, sizeof(char));
    if (identifier == NULL) {
        context->status = STATUS_SYSTEM_OUT_OF_MEMORY;
        return (size_t)-1;
    }

    memcpy(identifier, token->start, token->length);
    identifier[token->length] = '\0';

    context->id_table.identifiers[new_index] = identifier;
    context->id_table.count++;

    return new_index;
}

