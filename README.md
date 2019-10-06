# cppfileLex
Lex a single cpp file and print (row, column, TokenKind, TokenContents) into json file or csv file. It automatically removes macro-expanded codes(e.g. the code added by "#include").

## Prerequisites
* GNU make ( >= 4.1 )
* clang ( >= 6.0.0 )

## Build
* `make all`

## Run
* `./lx_cpp2json.o <input filename> <output filename>`

