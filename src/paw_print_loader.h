#ifndef PAW_PRINT_LOADER
#define PAW_PRINT_LOADER

#include <memory>
#include <vector>

#include <parse_table.h>
#include <token.h>


namespace paw_print {

using namespace parse_table;

using std::shared_ptr;
using std::vector;

class PawPrint;


class PAW_PRINT_API PawPrintLoader {
public:
    
    static shared_ptr<PawPrint> loadText (const char *text);

    static bool tokenize (const char *text, vector<Token> &tokens);
    static bool addIndentTokens (const vector<Token> &tokens, vector<Token> &indented);

private:
    static shared_ptr<ParsingTable> parsing_table_;

    static void _initParsingTable ();
};

}


#endif
