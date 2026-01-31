#include <stdio.h>
#include <assert.h>

#include "parser/parser.h"
#include "parser/ast.h"
#include "parser/utils.h"
#include "status.h"


#define HANDLE_ERROR(parser_context, action)          \
    do {                                              \
        if ((parser_context)->status != STATUS_OK) {  \
            action;                                   \
            return NULL;                              \
        }                                             \
    } while (0)


static AstNode* parseStatement(ParserContext* context);
static AstNode* parseScope(ParserContext* context);
static AstNode* parseSingleScope(ParserContext* context);
static AstNode* parseCycle(ParserContext* context);
static AstNode* parseIf(ParserContext* context);
static AstNode* parseDeclaration(ParserContext* context);
static AstNode* parseAssignment(ParserContext* context);
static AstNode* parseCall(ParserContext* context);
static AstNode* parseFunction(ParserContext* context);
static AstNode* parseReturn(ParserContext* context);
static AstNode* parseExpression(ParserContext* context);
static AstNode* parseTerm(ParserContext* context);
static AstNode* parseFactor(ParserContext* context);
static AstNode* parseIdentifier(ParserContext* context);
static AstNode* parseNumber(ParserContext* context);

static int stringToInt(const char* string, size_t len);
static void expect(ParserContext* context, TokenType expected_type);
static TokenType getCurrentTokenType(ParserContext* context);
static TokenType getNextTokenType(ParserContext* context);


AstNode* parseProgram(ParserContext* context)
{
    PARSER_ASSERT(context);

    AstNode* head = NULL;
    AstNode* current = NULL; 

    while (getCurrentTokenType(context) != TOKEN_EOF) {
        if (head) {
            AST_DUMP(context, head, "in the parseProgram function loop. Before parsing the Statement");
        }

        AstNode* statement = parseStatement(context);
        HANDLE_ERROR(context, deleteSubtree(head));

        size_t semicolon_line = context->tokens_array->tokens[context->current_token].line;

        expect(context, TOKEN_SEMICOLON);
        HANDLE_ERROR(context, {deleteSubtree(head); deleteSubtree(statement);});

        AstNode* semicolon = makeNode(context, AST_NODE_SEMICOLON, semicolon_line, statement, NULL);
        HANDLE_ERROR(context, {deleteSubtree(head); deleteSubtree(statement);});
        
        if (!head) {
            head = semicolon;
        } else {
            current->right = semicolon;
            semicolon->parent = current;
        }

        current = semicolon;
    }


    return head;
}


static AstNode* parseStatement(ParserContext* context)
{
    PARSER_ASSERT(context); 

    RETURN_PARSER_ERROR(context);

    TokenType current_type = getCurrentTokenType(context);
    if (current_type == TOKEN_IDENTIFIER) {
        TokenType next_type = getNextTokenType(context);
        
        if (next_type == TOKEN_OP_DECLARATION) {
            return parseDeclaration(context);
        } else if (next_type == TOKEN_OP_ASSIGN) {
            return parseAssignment(context);
        } else if (next_type == TOKEN_LEFT_PAREN) {
            return parseCall(context);
        } else {
            reportParserError(context, TOKEN_UNKNOWN, 
                "A declaration, assignment, or left parenthesis operator was expected");
            context->status = STATUS_SYNTAX_ERROR;
        }
    } else if (current_type == TOKEN_LEFT_BRACE) {
        return parseSingleScope(context);
    } else if (current_type == TOKEN_IF) {
        return parseIf(context);
    } else if (current_type == TOKEN_CYCLE) {
        return parseCycle(context);
    } else if (current_type == TOKEN_FUNC) {
        return parseFunction(context);
    } else if (current_type == TOKEN_RETURN) {
        return parseReturn(context);
    } else {
        reportParserError(context, TOKEN_UNKNOWN,
            "An unexpected token was received");
        context->status = STATUS_SYNTAX_ERROR;
    }

    return NULL;
}


static AstNode* parseScope(ParserContext* context) 
{
    PARSER_ASSERT(context);

    RETURN_PARSER_ERROR(context);

    AstNode* head = NULL;
    AstNode* current = NULL;

    expect(context, TOKEN_LEFT_BRACE);
    HANDLE_ERROR(context, {});

    TokenType current_type = getCurrentTokenType(context);
    while (current_type != TOKEN_RIGHT_BRACE && current_type != TOKEN_EOF) {
        AstNode* statement = parseStatement(context);
        HANDLE_ERROR(context, deleteSubtree(head));

        size_t semicolon_line = context->tokens_array->tokens[context->current_token].line;

        expect(context, TOKEN_SEMICOLON);
        HANDLE_ERROR(context, {deleteSubtree(head); deleteSubtree(statement);});

        AstNode* semicolon = makeNode(context, AST_NODE_SEMICOLON, semicolon_line, statement, NULL);
        HANDLE_ERROR(context, {deleteSubtree(statement); deleteSubtree(head);});

        if (!head) {
            head = semicolon;
        } else {
            current->right = semicolon;
            semicolon->parent = current;
        }

        current = semicolon;
        current_type = getCurrentTokenType(context);
    }

    expect(context, TOKEN_RIGHT_BRACE);
    HANDLE_ERROR(context, deleteSubtree(head));

    return head;
}


static AstNode* parseSingleScope(ParserContext* context)
{
    PARSER_ASSERT(context);

    RETURN_PARSER_ERROR(context);

    size_t scope_line = context->tokens_array->tokens[context->current_token].line;

    AstNode* scope = parseScope(context);
    HANDLE_ERROR(context, {});

    AstNode* single_scope = makeNode(context, AST_NODE_SCOPE, scope_line, NULL, scope);
    HANDLE_ERROR(context, deleteSubtree(scope));

    return single_scope;
}


static AstNode* parseCycle(ParserContext* context)
{
    PARSER_ASSERT(context);

    RETURN_PARSER_ERROR(context);

    size_t cycle_line = context->tokens_array->tokens[context->current_token].line;

    expect(context, TOKEN_CYCLE);
    HANDLE_ERROR(context, {});

    expect(context, TOKEN_LEFT_PAREN);
    HANDLE_ERROR(context, {});

    AstNode* expression = parseExpression(context);
    HANDLE_ERROR(context, {});
    
    expect(context, TOKEN_RIGHT_PAREN);
    HANDLE_ERROR(context, deleteSubtree(expression));

    AstNode* scope = parseScope(context);
    HANDLE_ERROR(context, deleteSubtree(expression));

    AstNode* cycle = makeNode(context, AST_NODE_CYCLE, cycle_line, expression, scope);
    HANDLE_ERROR(context, {deleteSubtree(expression); deleteSubtree(scope);});

    return cycle;
}


static AstNode* parseIf(ParserContext* context)
{
    PARSER_ASSERT(context);

    RETURN_PARSER_ERROR(context);

    size_t if_line = context->tokens_array->tokens[context->current_token].line;

    expect(context, TOKEN_IF);
    HANDLE_ERROR(context, {});

    expect(context, TOKEN_LEFT_PAREN);
    HANDLE_ERROR(context, {});

    AstNode* expression = parseExpression(context);
    HANDLE_ERROR(context, {});

    expect(context, TOKEN_RIGHT_PAREN);
    HANDLE_ERROR(context, deleteSubtree(expression));

    AstNode* scope = parseScope(context);
    HANDLE_ERROR(context, deleteSubtree(expression));

    AstNode* if_head = makeNode(context, AST_NODE_COMMA, if_line, expression, scope);
    HANDLE_ERROR(context, {deleteSubtree(expression); deleteSubtree(scope);});

    AstNode* else_scope = NULL;
    if (getCurrentTokenType(context) == TOKEN_ELSE) {
        expect(context, TOKEN_ELSE);
        HANDLE_ERROR(context, {deleteSubtree(expression);
            deleteSubtree(scope); deleteSubtree(if_head);});

        else_scope = parseScope(context);
        HANDLE_ERROR(context, deleteSubtree(if_head));
    }

    AstNode* if_node = makeNode(context, AST_NODE_IF, if_line, if_head, else_scope);
    HANDLE_ERROR(context, {deleteSubtree(if_head); deleteSubtree(else_scope);});

    return if_node;
}


static AstNode* parseDeclaration(ParserContext* context)
{
    PARSER_ASSERT(context);

    RETURN_PARSER_ERROR(context);

    size_t declaration_line = context->tokens_array->tokens[context->current_token].line;

    AstNode* variable = parseIdentifier(context);
    HANDLE_ERROR(context, {});

    expect(context, TOKEN_OP_DECLARATION);
    HANDLE_ERROR(context, deleteSubtree(variable));

    AstNode* expression = parseExpression(context);
    HANDLE_ERROR(context, deleteSubtree(variable));

    AstNode* declaration = makeNode(context, AST_NODE_DECLARATION, declaration_line, variable, expression);
    HANDLE_ERROR(context, {deleteSubtree(variable); deleteSubtree(expression);});

    return declaration;
}


static AstNode* parseAssignment(ParserContext* context)
{
    PARSER_ASSERT(context);

    RETURN_PARSER_ERROR(context);

    size_t assignment_line = context->tokens_array->tokens[context->current_token].line;

    AstNode* variable = parseIdentifier(context);
    HANDLE_ERROR(context, {});

    expect(context, TOKEN_OP_ASSIGN);
    HANDLE_ERROR(context, deleteSubtree(variable));

    AstNode* expression = parseExpression(context);
    HANDLE_ERROR(context, deleteSubtree(variable));

    AstNode* assignment = makeNode(context, AST_NODE_ASSIGNMENT, assignment_line, variable, expression);
    HANDLE_ERROR(context, {deleteSubtree(variable); deleteSubtree(expression);});

    return assignment;
}


static AstNode* parseCall(ParserContext* context)
{
    PARSER_ASSERT(context);

    RETURN_PARSER_ERROR(context);

    size_t call_line = context->tokens_array->tokens[context->current_token].line;

    AstNode* function_name = parseIdentifier(context);
    HANDLE_ERROR(context, {});

    expect(context, TOKEN_LEFT_PAREN);
    HANDLE_ERROR(context, deleteSubtree(function_name));

    AstNode* first_arg = NULL;
    AstNode* last_arg = NULL;
    if (getCurrentTokenType(context) != TOKEN_RIGHT_PAREN) {
        while (true) {
            AstNode* expression = parseExpression(context);
            HANDLE_ERROR(context, {deleteSubtree(function_name); deleteSubtree(first_arg);});

            AstNode* call_head = makeNode(context, AST_NODE_COMMA, call_line, expression, NULL);
            HANDLE_ERROR(context, {deleteSubtree(function_name);
                deleteSubtree(first_arg); deleteSubtree(expression);});

            if (first_arg == NULL) {
                first_arg = call_head;
            } else {
                last_arg->right = call_head;
                call_head->parent = last_arg;
            }
            last_arg = call_head;

            if (getCurrentTokenType(context) == TOKEN_COMMA) {
                context->current_token++;
            } else {
                break;
            }
        }
    }

    expect(context, TOKEN_RIGHT_PAREN);
    HANDLE_ERROR(context, {deleteSubtree(function_name); deleteSubtree(first_arg);});

    AstNode* call = makeNode(context, AST_NODE_CALL, call_line, function_name, first_arg);
    HANDLE_ERROR(context, {deleteSubtree(function_name); deleteSubtree(first_arg);});

    return call;
}

static AstNode* parseFunction(ParserContext* context)
{
    PARSER_ASSERT(context);

    RETURN_PARSER_ERROR(context);

    size_t function_line = context->tokens_array->tokens[context->current_token].line;

    expect(context, TOKEN_FUNC);
    HANDLE_ERROR(context, {});

    AstNode* function_name = parseIdentifier(context);
    HANDLE_ERROR(context, {});

    expect(context, TOKEN_LEFT_PAREN);
    HANDLE_ERROR(context, deleteSubtree(function_name));

    AstNode* first_param = NULL;
    AstNode* last_param = NULL;

    if (getCurrentTokenType(context) != TOKEN_RIGHT_PAREN) {
        while (true) {
            expect(context, TOKEN_PARAM);
            HANDLE_ERROR(context, {deleteSubtree(function_name); deleteSubtree(first_param);});

            AstNode* identifier = parseIdentifier(context);
            HANDLE_ERROR(context, {deleteSubtree(function_name); deleteSubtree(first_param);});

            if (first_param == NULL) {
                first_param = identifier;
            } else {
                last_param->right = identifier;
                identifier->parent = last_param;
            }

            last_param = identifier;

            if (getCurrentTokenType(context) == TOKEN_COMMA) {
                context->current_token++;
            } else {
                break;
            }
        }
    }

    AstNode* function_head = makeNode(context, AST_NODE_COMMA, function_line, function_name, first_param);
    HANDLE_ERROR(context, {deleteSubtree(function_name); deleteSubtree(first_param);});

    expect(context, TOKEN_RIGHT_PAREN);
    HANDLE_ERROR(context, deleteSubtree(function_head));

    AstNode* scope = parseScope(context);
    HANDLE_ERROR(context, deleteSubtree(function_head));

    AstNode* function = makeNode(context, AST_NODE_FUNCTION, function_line, function_head, scope);
    HANDLE_ERROR(context, {deleteSubtree(function_head); deleteSubtree(scope);});

    return function;
}


static AstNode* parseReturn(ParserContext* context)
{
    PARSER_ASSERT(context);

    RETURN_PARSER_ERROR(context);

    size_t return_line = context->tokens_array->tokens[context->current_token].line;

    expect(context, TOKEN_RETURN);
    HANDLE_ERROR(context, {});

    AstNode* expression = parseExpression(context);
    HANDLE_ERROR(context, {});

    AstNode* return_node = makeNode(context, AST_NODE_RETURN, return_line, NULL, expression);
    HANDLE_ERROR(context, deleteSubtree(expression));

    return return_node;
}


static AstNode* parseExpression(ParserContext* context)
{
    PARSER_ASSERT(context);

    RETURN_PARSER_ERROR(context);

    AstNode* left = parseTerm(context);
    HANDLE_ERROR(context, {});

    TokenType current_type = getCurrentTokenType(context);

    while (current_type == TOKEN_OP_ADD || current_type == TOKEN_OP_SUB) {
        size_t op_line = context->tokens_array->tokens[context->current_token].line;
        context->current_token++;

        AstNode* right = parseTerm(context);
        HANDLE_ERROR(context, deleteSubtree(left));

        NodeType op_type = AST_NODE_OP_ADD;
        if (current_type == TOKEN_OP_ADD) {
            op_type = AST_NODE_OP_ADD;
        } else {
            op_type = AST_NODE_OP_SUB;
        }

        AstNode* op_node = makeNode(context, op_type, op_line, left, right);
        HANDLE_ERROR(context, {deleteSubtree(left); deleteSubtree(right);});

        left = op_node;
        current_type = getCurrentTokenType(context);
    }

    return left;
}


static AstNode* parseTerm(ParserContext* context)
{
    PARSER_ASSERT(context);

    RETURN_PARSER_ERROR(context);

    AstNode* left = parseFactor(context);
    HANDLE_ERROR(context, {});

    TokenType current_type = getCurrentTokenType(context);

    while (current_type == TOKEN_OP_MUL || current_type == TOKEN_OP_DIV) {
        size_t op_line = context->tokens_array->tokens[context->current_token].line;
        context->current_token++;

        AstNode* right = parseFactor(context);
        HANDLE_ERROR(context, deleteSubtree(left));

        NodeType op_type = AST_NODE_OP_MUL;
        if (current_type == TOKEN_OP_MUL) {
            op_type = AST_NODE_OP_MUL;
        } else {
            op_type = AST_NODE_OP_DIV;
        }

        AstNode* op_node = makeNode(context, op_type, op_line, left, right);
        HANDLE_ERROR(context, {deleteSubtree(left); deleteSubtree(right);});

        left = op_node;
        current_type = getCurrentTokenType(context);
    }

    return left;
}


static AstNode* parseFactor(ParserContext* context)
{
    PARSER_ASSERT(context);

    RETURN_PARSER_ERROR(context);

    TokenType current_type = getCurrentTokenType(context);

    if (current_type == TOKEN_OP_ADD || current_type == TOKEN_OP_SUB) {
        size_t neg_line = context->tokens_array->tokens[context->current_token].line;
        context->current_token++;

        AstNode* operand = parseFactor(context);
        HANDLE_ERROR(context, {});

        if (current_type == TOKEN_OP_SUB) {
            AstNode* neg_node = makeNode(context, AST_NODE_OP_NEG, neg_line, NULL, operand);
            HANDLE_ERROR(context, deleteSubtree(operand));

            return neg_node;
        }

        return operand;
    } else if (current_type == TOKEN_NUMBER) {
        return parseNumber(context);
    } else if (current_type == TOKEN_LEFT_PAREN) {
        expect(context, TOKEN_LEFT_PAREN);
        HANDLE_ERROR(context, {});

        AstNode* expression = parseExpression(context);
        HANDLE_ERROR(context, {});

        expect(context, TOKEN_RIGHT_PAREN);
        HANDLE_ERROR(context, deleteSubtree(expression));

        return expression;
    } else if (current_type == TOKEN_IDENTIFIER) {
        if (getNextTokenType(context) == TOKEN_LEFT_PAREN) {
            return parseCall(context);
        } else {
            return parseIdentifier(context);
        }
    } else {
        reportParserError(context, TOKEN_UNKNOWN, 
            "Numeric value, parenthesized expression, or identifier expected");
        return NULL;
    }
}


static AstNode* parseIdentifier(ParserContext* context)
{
    PARSER_ASSERT(context);

    RETURN_PARSER_ERROR(context);

   if (getCurrentTokenType(context) != TOKEN_IDENTIFIER) {
        context->status = STATUS_SYNTAX_ERROR;
        reportParserError(context, TOKEN_IDENTIFIER, "Function or variable name expected");
        return NULL;
    }

    size_t identifier_line = context->tokens_array->tokens[context->current_token].line;
    AstNode* identifier = makeNode(context, AST_NODE_IDENTIFIER, identifier_line, NULL, NULL);
    HANDLE_ERROR(context, {});

    size_t id_index = addIdentifier(context);
    HANDLE_ERROR(context, deleteSubtree(identifier));

    identifier->data.id_index = id_index;

    context->current_token++;
    return identifier;
}


static AstNode* parseNumber(ParserContext* context)
{
    PARSER_ASSERT(context);

    RETURN_PARSER_ERROR(context);

    if (getCurrentTokenType(context) != TOKEN_NUMBER) {
        context->status = STATUS_SYNTAX_ERROR;
        reportParserError(context, TOKEN_NUMBER, "Numeric value expected");
        return NULL;
    }

    size_t number_line = context->tokens_array->tokens[context->current_token].line;
    AstNode* number = makeNode(context, AST_NODE_NUMBER, number_line, NULL, NULL);
    HANDLE_ERROR(context, {});

    Token* token = &context->tokens_array->tokens[context->current_token];
    int int_value = stringToInt(token->start, token->length);
    number->data.int_value = int_value;

    context->current_token++;
    return number;
}


static int stringToInt(const char* string, size_t len)
{
    assert(string);
    
    int result = 0;
    for (size_t index = 0; index < len; index++) {
        result = result * 10 + (string[index] - '0');
    }

    return result;
}


static void expect(ParserContext* context, TokenType expected_type)
{
    PARSER_ASSERT(context);

    TokenType current_type = getCurrentTokenType(context);
    if (expected_type == current_type) {
        context->current_token++;
        return;
    }

    context->status = STATUS_SYNTAX_ERROR;
    reportParserError(context, expected_type, "");
}


static TokenType getCurrentTokenType(ParserContext* context)
{
    assert(context);

    return context->tokens_array->tokens[context->current_token].type;
}


static TokenType getNextTokenType(ParserContext* context)
{
    assert(context);

    return context->tokens_array->tokens[context->current_token + 1].type;
}

