#include <stdio.h>
#include <stdlib.h>

#include "files/lexer.h"
#include "files/utils.h"
#include "files/parser.h"

#define FILE_PATH "../files/test/1.q"

int main() {
    char *buf = loadFile(FILE_PATH);

    tokenize(buf);

    showNiceOutput();

    parse();

    free(buf);
    return 0;
}