#include "token.h"

#include <sstream>


namespace paw_print {

using std::stringstream;

Token::Token (Type type, int first_idx, int last_idx, int indent)
:type(type),
 first_idx(first_idx),
 last_idx(last_idx),
 indent(indent) {
}

string Token::toString (const char *text) {
    stringstream ss;
    ss << "Token(";
    switch (type) {
        case INT         : ss << "INT, "         ; break;
        case DOUBLE      : ss << "DOUBLE, "      ; break;
        case STRING      : ss << "STRING, "      ; break;
        case COLON       : ss << "COLON, "       ; break;
        case COMMA       : ss << "COMMA, "       ; break;
        case DASH        : ss << "DASH, "        ; break;
		case SHARP       : ss << "SHARP, "       ; break;
        case SQUARE_OPEN : ss << "SQUARE_OPEN, " ; break;
        case SQUARE_CLOSE: ss << "SQUARE_CLOSE, "; break;
        case CURLY_OPEN  : ss << "CURLY_OPEN, "  ; break;
        case CURLY_CLOSE : ss << "CURLY_CLOSE, " ; break;
    }
    auto size = last_idx - first_idx + 2;
    auto str = new char [size];
    memcpy(str, &text[first_idx], size-1);
    str[size-1] = 0;
    ss << first_idx << ", " << last_idx << ", indent:" << indent << ")        " << str;
    delete[] str;
    return ss.str();
}


}
