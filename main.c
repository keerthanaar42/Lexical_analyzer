#include <stdio.h>
#include <string.h>
#include "lexer.h"

int main(int argc, char *argv[])
{
    const char *names[] = { "Keyword", "Operator", "Special", "Literal",
                            "Identifier", "Preprocessor", "Unknown" };
    TokenType prevType = UNKNOWN;
    char prevLexeme[MAX_TOKEN_SIZE] = "";
    Token token;

    if (argc != 2)
    {
        printf("Usage: %s <c file>\n", argv[0]);
        return 1;
    }

    initializeLexer(argv[1]);
    printf("Open    : %s : Success\n", argv[1]);
    printf("Parsing : %s : Started\n\n", argv[1]);

    /* empty lexeme = end of file (UNKNOWN alone can't be used, bad characters are UNKNOWN too) */
    while ((token = getNextToken()).lexeme[0] != '\0')
    {
        printf("%-12s : %s\n", names[token.type], token.lexeme);

        /* identifier followed by '[' means it is an array */
        if (token.type == SPECIAL_CHARACTER && token.lexeme[0] == '[' && prevType == IDENTIFIER)
            printf("%-12s : %s\n", "  -> Array", prevLexeme);

        prevType = token.type;
        strcpy(prevLexeme, token.lexeme);
    }

    printf("\nParsing : %s : Done\n", argv[1]);
    return 0;
}
