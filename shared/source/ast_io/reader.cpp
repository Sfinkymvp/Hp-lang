#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>

#include "ast_io/reader.h"
#include "id_table.h"
#include "ast_shared.h"
#include "status.h"


const size_t BUFFER_SIZE = 256;


#define RETURN_IF_STATUS_NOT_OK(status) \
    do {                                \
        if (status != STATUS_OK)        \
            return status;              \
    } while (0)


static OperationStatus treePrefixLoad(IdentifierTable* id_table, AstNode** root, FILE* file);
static OperationStatus readNode(IdentifierTable* id_table, AstNode** node,
    char* source_code, int* position);
static OperationStatus parseNode(IdentifierTable* id_table, AstNode** node,
    char* source_code, int* position);
static OperationStatus readTitle(IdentifierTable* id_table, AstNode* node,
    char* source_code, int* position);

static NodeType getNodeType(const char* buffer);
static size_t getFileSize(FILE* file);
static inline void skipWhitespaces(char* source_code, int* position);


OperationStatus loadAstTreeFromDisk(IdentifierTable* id_table, AstNode** root, const char* input_file)
{
    assert(id_table); assert(id_table->identifiers); assert(root); assert(input_file);

    FILE* file = fopen(input_file, "r");
    if (file == NULL) {
        return STATUS_IO_FILE_OPEN_ERROR;
    }

    OperationStatus status = treePrefixLoad(id_table, root, file);
    
    if (fclose(file) != 0) {
        if (status == STATUS_OK) {
            status = STATUS_IO_FILE_CLOSE_ERROR;
        }
    }
    
    if (status != STATUS_OK) {
        deleteSubtree(*root);
        *root = NULL;
    }

    return status;
}


static OperationStatus treePrefixLoad(IdentifierTable* id_table, AstNode** root, FILE* file)
{
    assert(id_table); assert(id_table->identifiers); assert(root); assert(file);

    size_t file_size = getFileSize(file);
    if (file_size == 0) {
        return STATUS_IO_FILE_EMPTY;
    }

    char* source_code = (char*)calloc(file_size + 1, 1);
    if (source_code == NULL) {
        return STATUS_SYSTEM_OUT_OF_MEMORY;
    }

    OperationStatus status = STATUS_OK;
    size_t read_size = fread(source_code, sizeof(char), file_size, file);
    if (read_size != file_size) {
        status = STATUS_IO_FILE_READ_ERROR;
    }
    if (status == STATUS_OK) {
        int position = 0;    
        status = readNode(id_table, root, source_code, &position);
    }

    free(source_code);
    return status;
}


static OperationStatus readNode(IdentifierTable* id_table, AstNode** node,
    char* source_code, int* position)
{
    assert(id_table); assert(node); assert(source_code); assert(position);

    skipWhitespaces(source_code, position);

    if (source_code[*position] == '(') {
        return parseNode(id_table, node, source_code, position);
    } else if (strncmp(&source_code[*position], "nil", 3) == 0) {
        (*position) += 3;
        *node = NULL;
        return STATUS_OK;
    }
 
    return STATUS_IO_FILE_READ_ERROR;
}


static OperationStatus parseNode(IdentifierTable* id_table, AstNode** node,
    char* source_code, int* position)
{
    assert(id_table); assert(node); assert(source_code); assert(position);

    OperationStatus status = createNode(node);
    RETURN_IF_STATUS_NOT_OK(status);
    (*position)++;

    skipWhitespaces(source_code, position);

    status = readTitle(id_table, *node, source_code, position);
    RETURN_IF_STATUS_NOT_OK(status);

    skipWhitespaces(source_code, position);

    status = readNode(id_table, &(*node)->left, source_code, position);
    RETURN_IF_STATUS_NOT_OK(status);
    if ((*node)->left != NULL)
        (*node)->left->parent = *node;

    skipWhitespaces(source_code, position);

    status = readNode(id_table, &(*node)->right, source_code, position);
    RETURN_IF_STATUS_NOT_OK(status);
    if ((*node)->right != NULL)
        (*node)->right->parent = *node;

    skipWhitespaces(source_code, position);
    (*position)++;

    return STATUS_OK;
}


static OperationStatus readTitle(IdentifierTable* id_table, AstNode* node,
    char* source_code, int* position)
{
    assert(id_table); assert(node); assert(source_code); assert(position);

    char buffer[BUFFER_SIZE] = {};
    int read_count = 0;
    int result = sscanf(&source_code[*position], "%s%n", buffer, &read_count);
    *position += read_count;
    if (result != 1) {
        return STATUS_IO_FILE_READ_ERROR;
    }

    NodeType type = getNodeType(buffer);
    if (type != AST_NODE_IDENTIFIER) {
        node->type = type;
        return STATUS_OK;
    }

    OperationStatus status = STATUS_OK;
    char* endptr = NULL;
    int value = (int)strtol(buffer, &endptr, 10);

    if (*endptr == '\0') {
        node->type = AST_NODE_NUMBER;
        node->data.int_value = value;
    } else if (endptr == buffer) {
        node->type = AST_NODE_IDENTIFIER;
        status = addIdentifier(id_table, buffer, &node->data.id_index);
    } else {
        status = STATUS_IO_FILE_READ_ERROR;
    }

    return status;
}


static NodeType getNodeType(const char* buffer)
{
    assert(buffer);

    for (size_t index = 0; index < TABLE_SIZE; index++) {
        if (strcmp(buffer, AST_NODE_TYPES_TABLE[index].symbol) == 0) {
            return AST_NODE_TYPES_TABLE[index].type;
        }
    }

    return AST_NODE_IDENTIFIER;
}


static size_t getFileSize(FILE* file)
{
    assert(file != NULL);

    struct stat buf = {};
    if (fstat(fileno(file), &buf) == -1)
        return 0;

    return (size_t)buf.st_size;
} 


static inline void skipWhitespaces(char* source_code, int* position)
{
    assert(source_code); assert(position);

    while (source_code[*position] == ' '  || source_code[*position] == '\t' ||
        source_code[*position] == '\n' || source_code[*position] == '\r') {
        (*position)++;
    }
}