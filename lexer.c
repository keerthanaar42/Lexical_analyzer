#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

static const char* keywords[MAX_KEYWORDS] = {
    "int", "float", "return", "if", "else", "while", "for", "do", "break", "continue",
    "char", "double", "void", "switch", "case", "default", "const", "static", "sizeof", "struct"
};

static const char* operators = "+-*/%=!<>|&";
static const char* specialCharacters = ",;{}()[]";

static FILE* fp = NULL;      /* the source file we are scanning */

/* ---------- 1. open the file ---------- */
void initializeLexer(const char* filename)
{
    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("Error: cannot open file %s\n", filename);
        exit(1);
    }
}

/* ---------- 2. small helper checks ---------- */
int isKeyword(const char* str)
{
    int i;
    for (i = 0; i < MAX_KEYWORDS; i++)
    {
        if (strcmp(str, keywords[i]) == 0)
            return 1;
    }
    return 0;
}

/* every character of str must be an operator character (handles ==, <=, &&, ++ ...) */
int isOperator(const char* str)
{
    int i;
    if (str[0] == '\0')
        return 0;
    for (i = 0; str[i] != '\0'; i++)
    {
        if (strchr(operators, str[i]) == NULL)
            return 0;
    }
    return 1;
}

int isSpecialCharacter(char ch)
{
    if (ch == '\0')
        return 0;                       /* strchr would match the '\0' at the end! */
    return strchr(specialCharacters, ch) != NULL;
}

/* number (int / float), "string" or 'c' */
int isConstant(const char* str)
{
    int i, dots = 0, digits = 0;
    int len = strlen(str);

    if (len == 0)
        return 0;

    /* string or character literal: must start AND end with the same quote */
    if (str[0] == '"' || str[0] == '\'')
        return (len >= 2 && str[len - 1] == str[0]);

    /* number: only digits and at most one '.' */
    for (i = 0; i < len; i++)
    {
        if (isdigit((unsigned char)str[i]))
            digits++;
        else if (str[i] == '.')
            dots++;
        else
            return 0;
    }
    return (digits > 0 && dots <= 1);
}

/* starts with letter or '_', rest letters/digits/'_', and not a keyword */
int isIdentifier(const char* str)
{
    int i;
    if (!(isalpha((unsigned char)str[0]) || str[0] == '_'))
        return 0;
    for (i = 1; str[i] != '\0'; i++)
    {
        if (!(isalnum((unsigned char)str[i]) || str[i] == '_'))
            return 0;
    }
    return !isKeyword(str);
}

/* ---------- 3. decide the type of a finished lexeme ---------- */
void categorizeToken(Token* token)
{
    char* s = token->lexeme;

    if (s[0] == '#')
        token->type = PREPROCESSOR;
    else if (isKeyword(s))
        token->type = KEYWORD;
    else if (isOperator(s))
        token->type = OPERATOR;
    else if (s[1] == '\0' && isSpecialCharacter(s[0]))
        token->type = SPECIAL_CHARACTER;
    else if (isConstant(s))
        token->type = CONSTANT;
    else if (isIdentifier(s))
        token->type = IDENTIFIER;
    else
        token->type = UNKNOWN;
}

/* ---------- 4. the heart: cut the next lexeme out of the file ---------- */
Token getNextToken()
{
    Token token;
    int ch, next, prev, i = 0;

    token.lexeme[0] = '\0';
    token.type = UNKNOWN;

    /* skip spaces, newlines and comments */
    while (1)
    {
        ch = fgetc(fp);
        while (ch != EOF && isspace(ch))
            ch = fgetc(fp);

        if (ch != '/')
            break;                              /* not a possible comment */

        next = fgetc(fp);
        if (next == '/')                        /* // comment: skip to end of line */
        {
            while ((ch = fgetc(fp)) != EOF && ch != '\n')
                ;
        }
        else if (next == '*')                   /* block comment: skip to the closing star-slash */
        {
            prev = 0;
            while ((ch = fgetc(fp)) != EOF && !(prev == '*' && ch == '/'))
                prev = ch;
        }
        else                                    /* just a division operator */
        {
            ungetc(next, fp);
            break;
        }
    }

    if (ch == EOF)
        return token;                           /* empty lexeme means "end of file" */

    if (ch == '#')                              /* preprocessor line: take the whole line */
    {
        while (ch != EOF && ch != '\n' && ch != '\r' && i < MAX_TOKEN_SIZE - 1)
        {
            token.lexeme[i++] = ch;
            ch = fgetc(fp);
        }
    }
    else if (isalpha(ch) || ch == '_')          /* keyword or identifier */
    {
        while ((isalnum(ch) || ch == '_') && i < MAX_TOKEN_SIZE - 1)
        {
            token.lexeme[i++] = ch;
            ch = fgetc(fp);
        }
        ungetc(ch, fp);                         /* give back the extra character we peeked at */
    }
    else if (isdigit(ch))                       /* integer or float */
    {
        while ((isdigit(ch) || ch == '.') && i < MAX_TOKEN_SIZE - 1)
        {
            token.lexeme[i++] = ch;
            ch = fgetc(fp);
        }
        ungetc(ch, fp);
    }
    else if (ch == '"' || ch == '\'')           /* string or character literal */
    {
        int quote = ch;
        token.lexeme[i++] = ch;
        while (i < MAX_TOKEN_SIZE - 1)
        {
            next = fgetc(fp);
            if (next == EOF || next == '\n')    /* unterminated literal, stop here */
            {
                ungetc(next, fp);
                break;
            }
            token.lexeme[i++] = next;
            if (next == '\\')                   /* escape like \n or \" : copy the next char too */
            {
                next = fgetc(fp);
                if (next != EOF && i < MAX_TOKEN_SIZE - 1)
                    token.lexeme[i++] = next;
            }
            else if (next == quote)             /* closing quote */
                break;
        }
    }
    else if (strchr(operators, ch))             /* operator, maybe two characters: == <= && ++ -> */
    {
        token.lexeme[i++] = ch;
        next = fgetc(fp);
        if (next == '=' ||
            (next == ch && strchr("+-&|<>", ch)) ||
            (ch == '-' && next == '>'))
            token.lexeme[i++] = next;
        else
            ungetc(next, fp);
    }
    else                                        /* ; , { } ( ) [ ] or something unknown */
    {
        token.lexeme[i++] = ch;
    }

    token.lexeme[i] = '\0';
    categorizeToken(&token);
    return token;
}
