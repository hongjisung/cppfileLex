// includes libclang(https://clang.llvm.org/)

#include <clang-c/Index.h>

#include <iostream>
#include <fstream>
#include <string>
#include <tuple>
#include <vector>
#include <unordered_map>

using namespace std;

const char* target_filename;
const char* output_filename;
string target_filename_s;
ofstream ofs;

CXIndex c_index;
CXTranslationUnit tr_unit;

// tokenlist
// start position, end position, line number, column number, token kind, token name
struct TokenList {
    unsigned startPosition;
    unsigned endPosition;
    CXCursor cursor;
    vector<tuple<unsigned, unsigned, unsigned, CXTokenKind, string>> tokenList;
};
vector<TokenList> exprTokenList;
// key: offset, data: cursor type spelling
unordered_map<int, string> offsetToKindSpelling;
// unordered_map<string, string> offsetToKindSpelling;

unsigned getTokenKindUInt(CXTokenKind ck)
{
    switch (ck)
    {
    case CXToken_Punctuation:
        return 0;
    case CXToken_Keyword:
        return 1;
    case CXToken_Identifier:
        return 2;
    case CXToken_Literal:
        return 3;
    case CXToken_Comment:
        return 4;
    default:
        break;
    }
    return 5;
}

string getTokenKindSpelling(CXTokenKind ck)
{
    switch (ck)
    {
    case CXToken_Punctuation:
        return "Punctuation";
    case CXToken_Keyword:
        return "Keyword";
    case CXToken_Identifier:
        return "Identifier";
    case CXToken_Literal:
        return "Literal";
    case CXToken_Comment:
        return "Comment";
    default:
        break;
    }
    return "N/A";
}

bool isCursorOnFile(CXCursor c, string filename)
{
    bool rvalue;

    // get the cursor's location
    CXSourceLocation loc = clang_getCursorLocation(c);
    CXFile cfile;
    unsigned lin, col, off;
    clang_getSpellingLocation(loc, &cfile, &lin, &col, &off);

    // compare the location and the given filename.
    CXString fn = clang_getFileName(cfile);
    string fns = static_cast<string>(clang_getCString(fn));
    rvalue = (fns.find(filename) != string::npos);

    clang_disposeString(fn);
    
    return rvalue;
}

void writeTokenInfo(unsigned line, unsigned column, unsigned offset, CXTokenKind tk, string tokenContents, string seperator="\t")
{
    ofs
    << line << seperator
    << column << seperator
    << offset << seperator
    << offsetToKindSpelling[offset] << seperator
    // << offsetToKindSpelling[to_string(line) + "+" + to_string(column)] << seperator
    << getTokenKindSpelling(tk) << seperator
    << tokenContents
    << "\n";
}

void writeExprTokenList(string seperator="\t") {
    for (int i = 0; i < exprTokenList.size(); i++) {
        if (i == exprTokenList.size() - 1 || exprTokenList[i].endPosition <= exprTokenList[i+1].startPosition) {
            for (int tkIdx = 0; tkIdx < exprTokenList[i].tokenList.size(); tkIdx++) {
                tuple<unsigned, unsigned, unsigned, CXTokenKind, string> data = exprTokenList[i].tokenList[tkIdx];
                writeTokenInfo(get<0>(data), get<1>(data), get<2>(data), get<3>(data), get<4>(data), seperator);
            }
        }
    }
}

// WARNING: visit_writeTokens depends on global variables in this file.
CXChildVisitResult visit_writeTokens(CXCursor c, CXCursor parent, CXClientData client_data)
{
    // check whether the cursor on the input filename.
    if(!isCursorOnFile(c, target_filename_s))
    {
        return CXChildVisit_Continue;
    }

    // get CXToken value
    CXSourceRange curRange = clang_getCursorExtent(c);
    CXToken *tokens;
    unsigned numtokens;
    clang_tokenize(tr_unit, curRange, &tokens, &numtokens);

    // cursor spelling
    CXString cs = clang_getCursorSpelling(c);
    string cursorSpelling = static_cast<string>(clang_getCString(cs));

    // cursor spelling offset
    unsigned cursorSpellingOffset = 0;
    
    // kind spelling
    CXCursorKind ck = clang_getCursorKind(c);
    if(    ck == CXCursor_UnexposedAttr
        || ck == CXCursor_UnexposedDecl
        || ck == CXCursor_UnexposedExpr
        || ck == CXCursor_UnexposedStmt)
    {
        return CXChildVisit_Recurse;
    }
    CXString cks = clang_getCursorKindSpelling(ck);
    string cursorKindSpelling = static_cast<string>(clang_getCString(cks));

    // token list
    TokenList tokenList;
    tokenList.startPosition = curRange.begin_int_data;
    tokenList.endPosition = curRange.end_int_data;
    tokenList.cursor = c;

    // for each token, print token informations on the stream ofs.
    for(unsigned i = 0; i < numtokens; ++i)
    {
        CXToken t = tokens[i];

        // get the token's location
        CXSourceLocation sloc = clang_getTokenLocation(tr_unit, t);
        CXFile cfile;
        unsigned lin, col, off;
        clang_getSpellingLocation(sloc, &cfile, &lin, &col, &off);

        // get tokenKind and tokenSpelling
        CXTokenKind tk = clang_getTokenKind(t);
        CXString ts = clang_getTokenSpelling(tr_unit, t);
        string tss = static_cast<string>(clang_getCString(ts));

        // write
        // writeTokenInfo(lin, col, tk, tss);
        tokenList.tokenList.push_back(make_tuple(lin, col, off, tk, tss));
        
        if (cursorSpelling == tss || numtokens == 1) {
            cursorSpellingOffset = off;
        }
    }

    if (exprTokenList.size() == 0 || exprTokenList[exprTokenList.size() - 1].endPosition < tokenList.endPosition) {
        exprTokenList.push_back(tokenList);
    }

    if (cursorSpellingOffset) {
        offsetToKindSpelling[cursorSpellingOffset] = cursorKindSpelling;
    }

    // cout<<"execute iter, token count"<<numtokens<<"\n";
    // return CXChildVisit_Continue;
    return CXChildVisit_Recurse;
}

int main(int argc, char **argv)
{
    // argv[1] is the input file name
    // argv[2] is the output file name

    // check whether it is valid call.
    if(argc < 3)
    {
        cerr << "ERROR: at least two arguments needed. Call as <ProgramName> <InputFile> <OutputFile>. Additional arguments will be ignored." << endl;
        exit(1);
    }

    // Set input-output filenames and prepare output file.
    target_filename = argv[1];
    target_filename_s = static_cast<string>(target_filename);
    output_filename = argv[2];
    ofs.open(output_filename);

    // Prepare cursor.
    c_index = clang_createIndex(0,0);
    tr_unit = clang_parseTranslationUnit(
        c_index,
        target_filename,
        nullptr, 0, nullptr, 0, CXTranslationUnit_None
    );
    if(tr_unit == nullptr)
    {
        exit(-1);
    }
    CXCursor cursor = clang_getTranslationUnitCursor(tr_unit);

    // Do core logic - visit and write tokens
    clang_visitChildren(
        cursor,
        visit_writeTokens,
        nullptr
    );

    writeExprTokenList();
    // close output file.
    ofs.close();

    return 0;
}