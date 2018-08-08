#ifndef PAW_PRINT_TOKEN
#define PAW_PRINT_TOKEN

#include <string>


namespace parse_table {

using std::string;

class Token {
public:
    enum Type {
        INDENT,
        DEDENT,
        INT,
        DOUBLE,
        STRING,
        COLON,
        COMMA,
        DASH,
		SHARP,
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
