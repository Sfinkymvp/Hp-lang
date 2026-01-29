#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>

#include "io/reader.h"
#include "lexer/token.h"


static size_t getFileSize(FILE* file);
static size_t getLinesCount(const char* buffer);


OperationStatus readSourceFile(SourceMap* source_map, const char* filename)
{
    assert(source_map); assert(filename);

    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        return STATUS_IO_FILE_OPEN_ERROR;
    }

    size_t file_size = getFileSize(file);
    if (file_size == 0) {
        fclose(file);
        return STATUS_IO_FILE_EMPTY;
    }

    char* buffer = (char*)calloc(file_size + 1, sizeof(char));
    if (buffer == NULL) {
        fclose(file);
        return STATUS_SYSTEM_OUT_OF_MEMORY;
    }

    size_t read_count = fread(buffer, sizeof(char), file_size, file);
    if (read_count != file_size) {
        fclose(file);
        free(buffer);
        return STATUS_IO_FILE_READ_ERROR;
    }

    if (fclose(file) != 0) {
        free(buffer);
        return STATUS_IO_FILE_CLOSE_ERROR;
    }

    OperationStatus status = createSourceMap(source_map, buffer);
    if (status != STATUS_OK) {
        deleteSourceMap(source_map);
    }

    return status;
}


static size_t getFileSize(FILE* file)
{
    assert(file != NULL);

    struct stat buf = {};
    if (fstat(fileno(file), &buf) == -1)
        return 0;

    return (size_t)buf.st_size;
}


OperationStatus createSourceMap(SourceMap* source_map, char* buffer)
{
    assert(source_map); assert(buffer);

    source_map->buffer = buffer;
    source_map->lines_count = getLinesCount(buffer);
    source_map->lines = (LineInfo*)calloc(source_map->lines_count, sizeof(LineInfo));
    if (source_map->lines == NULL) {
        return STATUS_SYSTEM_OUT_OF_MEMORY;
    }

    source_map->lines[0].start = buffer;
    size_t index = 0;

    for (index = 1; *buffer != '\0';) {
        if (*buffer == '\n') {
            source_map->lines[index].start = buffer + 1;
            source_map->lines[index - 1].length =
                (size_t)(source_map->lines[index].start - source_map->lines[index - 1].start - 1);
            index++;
        }
        buffer++;
    }
    assert(index == source_map->lines_count);

    source_map->lines[index - 1].length =
        (size_t)(buffer - source_map->lines[index - 1].start);

    return STATUS_OK;
}


static size_t getLinesCount(const char* buffer) 
{
    assert(buffer);

    if (*buffer == '\0') {
        return 0;
    }

    size_t lines_count = 0;

    while (*buffer != '\0') {
        if (*buffer == '\n') {
            lines_count++;
        }
        buffer++;
    }

    return lines_count + 1;
}


void deleteSourceMap(SourceMap* source_map)
{
    assert(source_map);

    if (source_map->buffer) {
        free(source_map->buffer);
        source_map->buffer = NULL;
    }
    if (source_map->lines) {
        free(source_map->lines);
        source_map->lines = NULL;
    }
}