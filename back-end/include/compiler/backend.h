#ifndef BACKEND_H_
#define BACKEND_H_


#include "ast_types.h"


#define HANDLE_ERROR(context, action)                  \
    do {                                                \
        if ((context)->status != STATUS_OK) {           \
            action;                                     \
            return NULL;                                \
        }                                               \ 
    } while (0)                                         \


typedef enum {
    SYMBOL_UNKNOWN;
    SYMBOL_VARIABLE;
    SYMBOL_PARAMETER;
    SYMBOL_FUNCTION;
} SymbolType;


typedef struct {
    const char* input_file;
    const char* output_file;
} CmdArgs;


typedef struct {
    const char* name;
    SymbolType type;
    size_t address;
} Symbol;


typedef struct {
    Symbol* symbols;
    size_t count;
    size_t capacity;
} SymbolTable;


typedef struct {
    CmdArgs cmd_args;
    FILE* file;
  
    SymbolTable symbol_table;
    size_t label_counter;
    size_t free_ram_address;
    OperationStatus status;
} BackendContext;


#endif // BACKEND_H_