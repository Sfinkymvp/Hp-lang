#include <stdio.h>
#include <assert.h>

#include "io/reader.h"


static size_t getFileSize(FILE* file);
static inline void skipWhitespaces(char* buffer, int* position);


AstNode* loadAstTreeFromDisk(BackendContext* context)
{
    assert(context);
   
    FILE* input_file = fopen(context->args.input_file, "r");
    if (input_file == NULL) {
        context->status = STATUS_IO_FILE_OPEN_ERROR;
    }
    HANDLE_ERROR(context, {});

    size_t file_size = getFileSize(input_file); 
    if (file_size == 0) {
        context->status = STATUS_IO_FILE_EMPTY;
    }
    HANDLE_ERROR(context, fclose(input_file));

    char* buffer = (char*)calloc(file_size + 1, 1);
    if (buffer == NULL) {
        context->status = STATUS_OUT_OF_MEMORY;
    }
    HANDLE_ERROR(context, fclose(input_file));

    size_t read_size = fread(tree->buffer, sizeof(char), file_size, input_file);
    if (read_size != file_size) {
        context->status = STATUS_IO_FILE_READ_ERROR;
    }
    HANDLE_ERROR(context, {fclose(input_file); free(buffer);});

    int position = 0;    
    AstNode* ast_root = readNode(context, &position);
    HANDLE_ERROR(context, {fclose(input_file); free(buffer);});

    if (fclose(input_file) != 0) {
        context->status = STATUS_IO_FILE_CLOSE_ERROR;
    }
    HANDLE_ERROR(context, {free(buffer); deleteSubtree(ast_root)});

    return TREE_OK;
}


static AstNode* readNode(BackendContext* context, char* buffer, AstNode* node, int* position)
{
    assert(context); assert(buffer); assert(node); assert(position);

    skipWhitespaces(tree->buffer, position);

    if (tree->buffer[*position] == '(') {
        return parseNode(tree, node, position);
    } else if (strncmp(&tree->buffer[*position], "nil", 3) == 0) {
        (*position) += 3;
        *node = NULL;
        return TREE_OK;
    }
 
    return TREE_INPUT_FILE_READ_ERROR;
}


static AstNode* parseNode(BackendContext* context, char* buffer, AstNode* node, int* position)
{
    assert(context); assert(buffer); assert(node); assert(position);
  
    TreeStatus status = createNode(node);
    RETURN_IF_NOT_OK(status);
    (*position)++;

    skipWhitespaces(tree->buffer, position);

    status = readTitle(&(*node)->data, tree->buffer, position);
    (*node)->is_dynamic = false;
    RETURN_IF_NOT_OK(status);

    skipWhitespaces(tree->buffer, position);
    status = readNode(tree, &(*node)->left, position);
    RETURN_IF_NOT_OK(status);
    if ((*node)->left != NULL)
        (*node)->left->parent = *node;

    skipWhitespaces(tree->buffer, position);
    status = readNode(tree, &(*node)->right, position);
    RETURN_IF_NOT_OK(status);
    if ((*node)->right != NULL)
        (*node)->right->parent = *node;

    skipWhitespaces(tree->buffer, position);
    (*position)++;

    return TREE_OK;
}


static TreeStatus readTitle(char** destination, char* buffer, int* position)
{
    assert(destination); assert(buffer); assert(position);

    int read_len = 0;
    int result = sscanf(buffer + *position, "\"%*[^\"]\"%n", &read_len);
    if (result != 0)
        return TREE_INPUT_FILE_READ_ERROR;

    buffer[*position + read_len - 1] = '\0';
    *destination = buffer + *position + 1;
    *position += read_len;

    return TREE_OK;
}


static size_t getFileSize(FILE* file)
{
    assert(file != NULL);

    struct stat buf = {};
    if (fstat(fileno(file), &buf) == -1)
        return 0;

    return (size_t)buf.st_size;
}



static inline void skipWhitespaces(char* buffer, int* position)
{
    assert(buffer), assert(position);

    while (buffer[*position] == ' '  || buffer[*position] == '\t' ||
           buffer[*position] == '\n' || buffer[*position] == '\r')
        (*position)++;
}

