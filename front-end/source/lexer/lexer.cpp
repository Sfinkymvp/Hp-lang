#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "lexer/lexer.h"
#include "lexer/utils.h"
#include "lexer/token.h"
#include "io/reader.h"


static const char* VALID_SYMBOLS = "();{}+-*/=";


static void expandTokensArray(LexerContext* context);
static void parseSpaces(LexerContext* context);
static void parseEof(LexerContext* context);
static bool isKeyword(LexerContext* context, size_t* keyword_idx);
static void parseKeyword(LexerContext* context, size_t keyword_idx);
static void parseNumber(LexerContext* context);
static void parseIdentifier(LexerContext* context);
static void parseComments(LexerContext* context);
static void parseError(LexerContext* context);
static bool isValidSymbol(char token);


void runLexer(LexerContext* context)
{
    LEXER_ASSERT(context);

    while (context->current[0] != '\0') {
        if (context->tokens_array.count == context->tokens_array.capacity) {
            expandTokensArray(context);
        }

        parseSpaces(context);

        if (isdigit(context->current[0])) {
            parseNumber(context);
            continue;
        } 

        if (strncmp("//", context->current, 2) == 0) {
            parseComments(context);
            continue;
        }

        size_t keyword_idx = 0;
        if (isKeyword(context, &keyword_idx)) {
            parseKeyword(context, keyword_idx);
            continue;
        }

        if (isalpha(context->current[0]) || context->current[0] == '_') {
            parseIdentifier(context);
            continue;
        }

        parseError(context);
        return;
    }

    parseEof(context);
}


static void expandTokensArray(LexerContext* context)
{
    LEXER_ASSERT(context);

    Token* tokens = (Token*)realloc(context->tokens_array.tokens, 
        context->tokens_array.capacity * 2 * sizeof(Token));
    if (tokens == NULL) {
        context->status = STATUS_SYSTEM_OUT_OF_MEMORY;
        return;
    }

    context->tokens_array.tokens = tokens;
    context->tokens_array.capacity *= 2;
}


static void parseSpaces(LexerContext* context)
{
    LEXER_ASSERT(context);

    while (isspace(context->current[0])) {
        if (context->current[0] == '\n') {
            context->current_line++;
        }
        context->current++;
    }
}


static void parseEof(LexerContext* context)
{
    LEXER_ASSERT(context);
    
    /*for (size_t index = 0; index < context->tokens_array.count; index++) {
        Token token = context->tokens_array.tokens[index];
        printf("Index: %3zu, type: %3d, length: %3zu, line: %3zu, text: %.*s\n",
            index, token.type, token.length, token.line, (int)token.length, token.start);
    }*/


    Token* token = &context->tokens_array.tokens[context->tokens_array.count];
    context->tokens_array.count++;

    token->type = TOKEN_EOF;
    token->start = context->current;
    token->length = 1;
    token->line = context->current_line;

    context->current += token->length;
}


static bool isKeyword(LexerContext* context, size_t* keyword_idx) {
    LEXER_ASSERT(context); assert(keyword_idx);

    for (size_t index = 0; index < KEYWORD_TABLE_SIZE; index++) {
        size_t length = KEYWORD_TABLE[index].length;
        if (strncmp(context->current, KEYWORD_TABLE[index].text, length) == 0) {
            char last_char = context->current[length - 1];
            char next_char = context->current[length];

            if (isalnum(last_char) && (isalnum(next_char) || next_char == '_')) {
                continue;
            }

            *keyword_idx = index;
            return true;
        }
    }
    
    return false;
}


static void parseKeyword(LexerContext* context, size_t keyword_idx)
{
    LEXER_ASSERT(context); assert(keyword_idx < KEYWORD_TABLE_SIZE);

    Token* token = &context->tokens_array.tokens[context->tokens_array.count++];

    token->type = KEYWORD_TABLE[keyword_idx].type;
    token->start = context->current;
    token->length = KEYWORD_TABLE[keyword_idx].length;
    token->line = context->current_line;

    context->current += token->length;
}


static void parseNumber(LexerContext* context)
{
    LEXER_ASSERT(context);

    const char* start = context->current;
    const char* end = context->current;
    while (isdigit(end[0])) {
        end++;
    }

    if (isalpha(end[0])) {
        start = end;
        while (isalnum(end[0]) || end[0] == '_') {
            end++;
        }

        size_t error_length = (size_t)(end - start);
        printf("length: %zu\n", error_length);
        reportLexerError(context, start, error_length,
            "Invalid number literal. Unexpected characters following a numeric value");

        context->current = end;
        return;
    }
    
    Token* token = &context->tokens_array.tokens[context->tokens_array.count++];

    token->type = TOKEN_NUMBER;
    token->start = start;
    token->length = (size_t)(end - start);
    token->line = context->current_line;

    context->current = end;
}


static void parseIdentifier(LexerContext* context)
{
    LEXER_ASSERT(context);

    const char* end = context->current;
    while (isalpha(end[0]) || isdigit(end[0]) || end[0] == '_') {
        end++;
    }

    size_t length = (size_t)(end - context->current);
    bool is_keyword = false;
    Token* token = &context->tokens_array.tokens[context->tokens_array.count++];

    for (size_t index = 0; index < KEYWORD_TABLE_SIZE; index++) {
        if (KEYWORD_TABLE[index].length == length &&
            strncmp(context->current, KEYWORD_TABLE[index].text, length) == 0) {

            token->type = KEYWORD_TABLE[index].type;
            token->start = context->current;
            token->length = length;
            token->line = context->current_line;

            is_keyword = true;
            break;
        }
    }

    if (!is_keyword) {
        token->type = TOKEN_IDENTIFIER;
        token->start = context->current;
        token->length = length;
        token->line = context->current_line;
    }

    context->current = end;
}


static void parseComments(LexerContext* context)
{
    LEXER_ASSERT(context);

    if (strncmp("///", context->current, 3) == 0) {
        const char* comment_start = context->current;
        context->current += 3;
        bool found_end = false;

        while (context->current[0] != '\0') {
            if (strncmp("\\\\\\", context->current, 3) == 0) {
                context->current += 3;
                found_end = true;
                break;
            }
            if (context->current[0] == '\n') {
                context->current_line++;
            }

            context->current++;
        }

        if (!found_end) {
            reportLexerError(context, comment_start, 3, "Unterminated multiline comment");
        }
    } else if (strncmp("//", context->current, 2) == 0) {
        context->current += 2;
        while (context->current[0] != '\n' && context->current[0] != '\0') {
            context->current++;
        }
    }
}


static void parseError(LexerContext* context)
{
    LEXER_ASSERT(context);

    const char* error_start = context->current;

    while (context->current[0] != '\0' && !isspace(context->current[0]) && 
        !isValidSymbol(context->current[0])) {
        context->current++;
    }

    size_t error_length = (size_t)(context->current - error_start);

    if (error_length == 0) {
        error_length = 1;
        context->current++;
    }

    reportLexerError(context, error_start, error_length,
        "Unknown token or invalid character sequence encountered");
}


static bool isValidSymbol(char token)
{
    if (isalnum(token) || token == '_') {
        return true;
    }

    if (strchr(VALID_SYMBOLS, token) != NULL) {
        return true;
    } else {
        return false;
    }
}