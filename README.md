# Lexical Analyzer in C

A small lexical analyzer (lexer) written in C. It reads a `.c` source file, breaks it into tokens, and prints what kind of token each one is: keyword, identifier, operator, literal and so on.

I built this as part of the Advanced C module in my embedded systems training at Emertxe. A lexer is the first stage of a compiler, so this project was a good way to practise file I/O, string handling and pointers on something that actually does a real job.

## What it can recognise

| Token type | Examples |
|---|---|
| Keywords | `int`, `float`, `return`, `if`, `while`, `struct` (20 keywords) |
| Identifiers | `main`, `count`, `_temp1` |
| Operators | `+`, `-`, `=`, `==`, `<=`, `&&`, `++`, `->` |
| Special characters | `;` `,` `{` `}` `(` `)` `[` `]` |
| Literals | integers (`5`), floats (`3.14`), characters (`'a'`), strings (`"Hello\n"`) |
| Preprocessor lines | `#include <stdio.h>`, `#define MAX 10` |
| Arrays | an identifier followed by `[`, e.g. `arr[5]` |

Comments (`//` and `/* ... */`) and whitespace are skipped.

## Project structure

```
LexicalAnalyzer/
├── lexer.h     # Token struct, TokenType enum, function declarations
├── lexer.c     # All the lexer logic
├── main.c      # Driver: opens the file, loops over tokens, prints them
└── test.c      # Sample C file to try it on
```

## Build and run

```bash
gcc -Wall -o lexical_analyser main.c lexer.c
./lexical_analyser test.c
```

Running it without a file prints the usage message:

```
Usage: ./lexical_analyser <c file>
```

## Sample output

Input (`hello.c`):

```c
#include <stdio.h>
int main()
{
    printf("Hello World\n");
    return 0;
}
```

Output:

```
Open    : hello.c : Success
Parsing : hello.c : Started

Preprocessor : #include <stdio.h>
Keyword      : int
Identifier   : main
Special      : (
Special      : )
Special      : {
Identifier   : printf
Special      : (
Literal      : "Hello World\n"
Special      : )
Special      : ;
Keyword      : return
Literal      : 0
Special      : ;
Special      : }

Parsing : hello.c : Done
```

## How it works

1. `main` opens the file with `initializeLexer()` and keeps calling `getNextToken()` until it gets an empty lexeme, which means end of file.
2. `getNextToken()` skips whitespace and comments, then looks at the first character to decide what it's reading:
   - letter or `_` : keep reading letters/digits (keyword or identifier)
   - digit : keep reading digits and `.` (integer or float)
   - `"` or `'` : read until the closing quote, handling escapes like `\"`
   - operator character : read one or two characters (`=`, `==`, `<=`, `&&`)
   - `#` : read the whole preprocessor line
   - anything else : a single character like `;` or `{`
3. `categorizeToken()` labels the finished lexeme using helpers like `isKeyword()`, `isOperator()` and `isConstant()`. The order of checks matters, for example keywords are checked before identifiers.
4. Because a lexer only sees one token at a time, `main` remembers the previous token. If it sees `[` right after an identifier, it reports that identifier as an array.

The main trick is lookahead: to know where a token ends, the lexer has to read one character too far, then hand it back with `ungetc()`.

## Changes I made to the given skeleton

- Added a `PREPROCESSOR` token type to `lexer.h`.
- `main` stops on an empty lexeme (end of file) instead of on `UNKNOWN`, because invalid characters are also `UNKNOWN` and would stop the loop early.
- Added an `argc` check and an error message if the file can't be opened.

## Known limitations

- No hex (`0x1F`) or octal numbers, no float suffixes (`3.5f`), no numbers starting with `.`
- Not every C operator is covered (for example `.`, `?`, `:`, `~`, `^`)
- Something like `12ab` is read as two tokens, `12` and `ab`, instead of being reported as an error
- An unterminated `/* ...` comment runs to the end of the file with no warning
- No line numbers in the output

## Ideas for improving it

- Track line numbers and give proper error messages
- Add hex/octal numbers and float suffixes
- Build a symbol table for identifiers
- Feed the tokens into a simple parser

## What I learned

- Using `fgetc` and `ungetc` for character-by-character file reading
- Why `fgetc` returns `int` and not `char` (so EOF can be told apart from real data)
- Splitting a project cleanly across a header and source files
- Testing with many small input files, one feature at a time, rather than one big file
