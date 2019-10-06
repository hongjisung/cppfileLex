# cppfileLex
Lex a single cpp file and print (row, column, TokenKind, TokenContents) into json file or csv file. It automatically removes macro-expanded codes(e.g. the code added by "#include").

## Prerequisites
* GNU make ( >= 4.1 )
* clang ( >= 6.0.0 )

## Build
* `make all`

## Run
* `./lx_cpp2json.o <input filename> <output filename>`

## Reference
* [libclang doc](https://clang.llvm.org/doxygen/group__CINDEX.html)
* libclang tutorial 1 https://gist.github.com/yifu/3761845
* libclang tutorial 2 https://shaharmike.com/cpp/libclang/
