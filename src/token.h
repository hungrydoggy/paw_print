#ifndef PAW_PRINT_TOKEN
#define PAW_PRINT_TOKEN

#include <string>


namespace paw_print {

using std::string;

class Token {
public:
    enum Type {
        INT,
        DOUBLE,
        STRING,
        COLON,
        COMMA,
        DASH,
        SQUARE_OPEN,
        SQUARE_CLOSE,
        CURLY_OPEN,
        CURLY_CLOSE,
    };

    Type type;
    int first_idx;
    int last_idx;
    int indent;


    Token (Type type, int first_idx, int last_idx, int indent);

    string toString (const char *text);
};

}


#endif
