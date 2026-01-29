#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "ast_dump/html_generator.h"
#include "lexer/token.h"
#include "ast_dump/ast_visualizer.h"
#include "parser/ast.h"


const size_t BUFFER_SIZE = 256;
static const char* GRAPH_DUMP_DIRECTORY = "images";
static const char* HTML_DUMP_FILENAME = "ast_dump.html";


#define DUMP_FILE context->dump_file


typedef struct {
    OperationStatus status;
    const char* message;
    const char* file;
    const char* function;
    int line;
} DumpInfo;


static void createHtmlDump(ParserContext* context, DumpInfo* info, const char* image, size_t image_counter);
static void writeTreeInfo(ParserContext* context, DumpInfo* info, size_t image_counter);
static void convertDotToSvg(const char* dot_file, const char* svg_file);


void openAstDumpFile(ParserContext* context)
{
    assert(context);

    char command[BUFFER_SIZE] = "";
    snprintf(command, BUFFER_SIZE, "rm -rf %s && mkdir -p %s",
        GRAPH_DUMP_DIRECTORY, GRAPH_DUMP_DIRECTORY);
    system(command);

    char filename[BUFFER_SIZE] = "";
    snprintf(filename, BUFFER_SIZE, "%s", HTML_DUMP_FILENAME);

    context->dump_file = fopen(filename, "w");
    assert(context->dump_file);
}


void visualizeAstTree(ParserContext* context, AstNode* ast_root, const char* file, 
    const char* function, int line, const char* format, ...)
{
    PARSER_ASSERT(context); assert(ast_root); assert(file);
    assert(function); assert(format);

    static size_t image_counter = 1;

    char message[BUFFER_SIZE] = {};
    if (format[0] != '\0') {
        va_list args;
        va_start(args, format);
        vsnprintf(message, BUFFER_SIZE, format, args);
        va_end(args);
    }
 
    DumpInfo info = {context->status, message, file, function, line};
    char graph_dot_file[BUFFER_SIZE] = {};
    char graph_svg_file[BUFFER_SIZE] = {};

    snprintf(graph_dot_file, BUFFER_SIZE, "%s/tree_graph_%03zu.dot",
        GRAPH_DUMP_DIRECTORY, image_counter);
    snprintf(graph_svg_file, BUFFER_SIZE, "%s/tree_graph_%03zu.svg",
        GRAPH_DUMP_DIRECTORY, image_counter);

    generateGraph(context, ast_root, graph_dot_file);
    convertDotToSvg(graph_dot_file, graph_svg_file);

    char command[BUFFER_SIZE * 2] = {};
    snprintf(command, BUFFER_SIZE * 2, "rm %s", graph_dot_file);
    system(command);

    createHtmlDump(context, &info, graph_svg_file, image_counter);

    image_counter++;
}


static void createHtmlDump(ParserContext* context, DumpInfo* info, const char* image, size_t image_counter)
{
    PARSER_ASSERT(context); assert(info); assert(image);

    fprintf(DUMP_FILE,
        "<html>\n"
        "\t<style>\n"
        "\t\tbody {font-family: monospace;}\n"
        "\t</style>\n"
        "\t<body>\n");

    writeTreeInfo(context, info, image_counter);

    fprintf(DUMP_FILE,
        "\t\t<div style=\"overflow-x: auto; white-space: nowrap;\">\n"
        "\t\t\t<img src=\"%s\" style=\"zoom:0.65; -moz-transform:scale(0.1); -moz-transform-origin:top left;\">\n"
        "\t\t</div>\n"
        "\t\t<hr style=\"margin: 40px 0; border: 2px solid #ccc;\">\n"
        "\t</body>\n"
        "</html>\n", image);
}


static void writeTreeInfo(ParserContext* context, DumpInfo* info, size_t image_counter)
{
    PARSER_ASSERT(context); assert(info);

    fprintf(DUMP_FILE,
        "\t\t<h1>AST DUMP #%03zu</h1>\n"
        "\t\t<h2>Dump {%s:%d} called from %s()</h2>\n"
        "\t\t<h3>STATUS: %d </h3>\n"
        "\t\t<h3>MESSAGE: %s</h3>\n",
        image_counter, info->file, info->line, info->function, context->status, info->message);
}


static void convertDotToSvg(const char* dot_file, const char* svg_file)
{
    assert(dot_file); assert(svg_file);

    char command[BUFFER_SIZE] = {};
    snprintf(command, BUFFER_SIZE, "dot -Tsvg %s -o %s",
             dot_file, svg_file);

    system(command);
}