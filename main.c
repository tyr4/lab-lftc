#include <stdio.h>
#include <stdlib.h>

#include "files/lexer.h"
#include "files/utils.h"

#define FILE_PATH "../files/test/1.q"

int main(void) {
    char *buf = loadFile(FILE_PATH);

    tokenize(buf);

    showNiceOutput();

    free(buf);
    return 0;
}