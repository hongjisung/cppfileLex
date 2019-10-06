// includes libclang(https://clang.llvm.org/) and cppjson(https://github.com/nlohmann/json)

#include <clang-c/Index.h>
#include <nlohmann/json.hpp>

#include <iostream>
#include <fstream>
#include <string>

using namespace std;
using nlohmann::json;

const char* target_filename;
const char* output_filename;
string target_filename_s;
ofstream ofs;
json j;

CXIndex c_index;
CXTranslationUnit tr_unit;

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

void writeTokenInfo(unsigned line, unsigned column, CXTokenKind tk, string tokenContents)
{
    ofs
    << line << ", "
    << column << ", "
    << getTokenKindSpelling(tk) << ", "
    << tokenContents
    << endl;
}

void addTokenInfoToJson(json &j, unsigned line, unsigned column, CXTokenKind tk, string tokenContents)
{
    json jj;
    string ks = getTokenKindSpelling(tk);

    jj.push_back(line);
    jj.push_back(column);
    jj.push_back(ks);
    jj.push_back(tokenContents);

    j.push_back(jj);
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
        addTokenInfoToJson(j, lin, col, tk, tss);
    }

    return CXChildVisit_Continue;
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

    // write collected json to file
    ofs << j.dump(4);

    // close output file.
    ofs.close();

    return 0;
}