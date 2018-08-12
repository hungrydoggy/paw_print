#include "./paw_print_loader.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "./paw_print.h"


using std::cout;
using std::endl;
using std::ifstream;
using std::stringstream;


namespace paw_print {


shared_ptr<ParsingTable> PawPrintLoader::parsing_table_;


static bool _addNumberToken (
        const char *text,
        int first_idx,
        int last_idx,
        int indent,
        vector<Token> &tokens) {

    // find point idx and check number format
    auto point_idx = last_idx + 1;
    for (int ci=first_idx; ci<=last_idx; ++ci) {
        auto c = text[ci];
        if (c == '.') {
            if (point_idx <= last_idx) {
                // TODO err not number
                cout << "err: \'"
                        << string(text + first_idx, last_idx - first_idx + 1)
                        << "\' is not valid number." << endl;
                return false;
            }
            point_idx = ci;
        }else if (c < '0' || c > '9') {
            // TODO err not number
            cout << "err: \'"
                    << string(text + first_idx, last_idx - first_idx + 1)
                    << "\' is not valid number." << endl;
            return false;
        }
    }


    auto is_int = point_idx > last_idx;
    auto type = (is_int == true)? TokenType::INT : TokenType::DOUBLE;

    tokens.push_back(Token(type, first_idx, last_idx, indent));

    return true;
}

static bool _addStringToken (
        const char *text,
        int first_idx,
        int last_idx,
        int indent,
        vector<Token> &tokens) {

    tokens.push_back(Token(TokenType::STRING, first_idx, last_idx, indent));

    return true;
}

static bool _addWordToken (
        const char *text,
        int first_idx,
        int last_idx,
        int indent,
        vector<Token> &tokens) {

    if (text[first_idx] >= '0' && text[first_idx] <= '9')
        return _addNumberToken(text, first_idx, last_idx, indent, tokens);
    else
        return _addStringToken(text, first_idx, last_idx, indent, tokens);
}

// return next idx
static int _findAndAddToken_quoted(
        char quote_char,
        const char *text,
        int start_idx,
        int indent,
        vector<Token> &tokens) {

    auto idx = start_idx;
    for (; true; ++idx) {
        const char c = text[idx];
        switch (c) {
            case 0:
                //TODO err text is end without closing quote
                cout << "err: text is end without closing quote" << endl;
                return -1;
            case '\n':
                //TODO err text is newlined without closing quote
                cout << "err: text is newlined without closing quote" << endl;
                return -1;
            case '\\':
                ++idx;
                break;
            default:
                if (c == quote_char) {
                        auto is_ok = _addStringToken(text, start_idx, idx - 1, indent, tokens);
                        if (is_ok == false)
                            return -1;
                        return idx + 1;
                }
                break;
        }

    }
}

// return next idx
static int _findAndSkip_comment (
		const char *text,
		int start_idx) {
	
	auto idx = start_idx;
	for (; true; ++idx) {
		const char c = text[idx];
		switch (c) {
		case 0:
		case '\n':
			return idx;
		default:
			break;
		}
	}
}

// return next idx
static int _findAndAddToken (
        const char *text,
        int start_idx,
        vector<Token> &tokens,
        int &indent) {

    int idx = start_idx;

    // trim left and indent
    bool need_update_indent = false;
    for (; true; ++idx) {
        const char c = text[idx];
        if (c == 0) {
            return idx;
        }if (c == '\n') {
            indent = 0;
            need_update_indent = true;
        }else if (c == ' ') {
            if (need_update_indent == true)
                ++indent;
        }else if (c == '\r') {
            ;
        }else {
            break;
        }
    }
    int first_idx = idx;


    // tokenize
    for (; true; ++idx) {
        const char c = text[idx];
        switch (c) {
            case 0:
                return idx;
			case '\r':
            case ' ': {
				if (idx > first_idx) {
					auto is_ok = _addWordToken(text, first_idx, idx - 1, indent, tokens);
					if (is_ok == false)
						return -1;
				}
                return idx + 1;
            }

			case '#': {
				if (idx > first_idx) {
					auto is_ok = _addWordToken(text, first_idx, idx - 1, indent, tokens);
					if (is_ok == false)
						return -1;
				}
				return _findAndSkip_comment(text, idx + 1);
			}

            case '\n': {
				if (idx > first_idx) {
					auto is_ok = _addWordToken(text, first_idx, idx - 1, indent, tokens);
					if (is_ok == false)
						return -1;
				}
                return idx;
            }
            case ',': {
                if (idx > first_idx) {
                    auto is_ok = _addWordToken(text, first_idx, idx - 1, indent, tokens);
                    if (is_ok == false)
                        return -1;
                }

                tokens.push_back(Token(TokenType::COMMA, idx, idx, indent));

                return idx + 1;
            }
            case ':': {
                if (idx > first_idx) {
                    auto is_ok = _addWordToken(text, first_idx, idx - 1, indent, tokens);
                    if (is_ok == false)
                        return -1;
                }

                tokens.push_back(Token(TokenType::COLON, idx, idx, indent));

                indent += 4;

                return idx + 1;
            }
            case '-': {
                if (idx > first_idx) {
                    auto is_ok = _addWordToken(text, first_idx, idx - 1, indent, tokens);
                    if (is_ok == false)
                        return -1;
                }

                tokens.push_back(Token(TokenType::DASH, idx, idx, indent));

                return idx + 1;
            }
            case '\"': {
                if (idx > first_idx) {
                    auto is_ok = _addWordToken(text, first_idx, idx - 1, indent, tokens);
                    if (is_ok == false)
                        return -1;
                }

                return _findAndAddToken_quoted('\"', text, idx + 1, indent, tokens);
            }
            case '\'': {
                if (idx > first_idx) {
                    auto is_ok = _addWordToken(text, first_idx, idx - 1, indent, tokens);
                    if (is_ok == false)
                        return -1;
                }

                return _findAndAddToken_quoted('\'', text, idx + 1, indent, tokens);
            }
            case '{': {
                if (idx > first_idx) {
                    auto is_ok = _addWordToken(text, first_idx, idx - 1, indent, tokens);
                    if (is_ok == false)
                        return -1;
                }

                tokens.push_back(Token(TokenType::CURLY_OPEN, idx, idx, indent));

                return idx + 1;
            }
            case '}': {
                if (idx > first_idx) {
                    auto is_ok = _addWordToken(text, first_idx, idx - 1, indent, tokens);
                    if (is_ok == false)
                        return -1;
                }

                tokens.push_back(Token(TokenType::CURLY_CLOSE, idx, idx, indent));

                return idx + 1;
            }
            case '[': {
                if (idx > first_idx) {
                    auto is_ok = _addWordToken(text, first_idx, idx - 1, indent, tokens);
                    if (is_ok == false)
                        return -1;
                }

                tokens.push_back(Token(TokenType::SQUARE_OPEN, idx, idx, indent));

                return idx + 1;
            }
            case ']': {
                if (idx > first_idx) {
                    auto is_ok = _addWordToken(text, first_idx, idx - 1, indent, tokens);
                    if (is_ok == false)
                        return -1;
                }

                tokens.push_back(Token(TokenType::SQUARE_CLOSE, idx, idx, indent));

                return idx + 1;
            }
        }
    }

    return idx + 1;
}

bool PawPrintLoader::tokenize (const char *text, vector<Token> &tokens) {
    int idx = 0;

    // trim left and indent
    int indent = 0;
    for (; true; ++idx) {
        const char c = text[idx];
        if (c == 0) {
            return idx - 1;
        }if (c == '\n') {
            indent = 0;
        }else if (c == ' ') {
            ++indent;
        }else if (c == '\r') {
            ;
        }else {
            break;
        }
    }


    // find tokens
    while (text[idx] != 0) {
        idx = _findAndAddToken(text, idx, tokens, indent);
        if (idx < 0)
            return false;
    }

    return true;
}

bool PawPrintLoader::addIndentTokens (const vector<Token> &tokens, vector<Token> &indented) {
    if (tokens.size() <= 0)
        return false;

    indented.push_back(tokens[0]);

    stack<int> block_stack;
    stack<int> indent_stack;
    
    int pre_t_idx = 0;
    for (int ti=1; ti<tokens.size(); ++ti) {
        auto &pre_t = tokens[pre_t_idx];
        auto &t = tokens[ti];

        // close block
        switch (t.type) {
            case TokenType::SQUARE_CLOSE:
                if (block_stack.top() != TokenType::SQUARE_OPEN) {
                    // TODO err: block is not matched {t.first_idx}
                    cout << "err: block is not matched. idx : " << t.first_idx << endl;
                    return false;
                }
                block_stack.pop();
                break;
            case TokenType::CURLY_CLOSE:
                if (block_stack.top() != TokenType::CURLY_OPEN) {
                    // TODO err: block is not matched {t.first_idx}
                    cout << "err: block is not matched. idx : " << t.first_idx << endl;
                    return false;
                }
                block_stack.pop();
                break;
        }

        // insert indent/dedent
        if (block_stack.empty() == true) {
            if (t.indent > pre_t.indent) {
                indented.push_back(Token(TokenType::INDENT, t.first_idx, t.last_idx, t.indent));
                indent_stack.push(t.indent);
            }else if (t.indent < pre_t.indent){
                while (t.indent < indent_stack.top()) {
                    indented.push_back(Token(TokenType::DEDENT, t.first_idx, t.last_idx, t.indent));
                    indent_stack.pop();
                }
            }
        }

        // open block
        switch (t.type) {
            case TokenType::SQUARE_OPEN:
            case TokenType::CURLY_OPEN:
                block_stack.push(t.type);
                break;
        }

        indented.push_back(t);
        pre_t_idx = ti;
    }

    if (block_stack.empty() == false) {
        // TODO err: block is not ended
        cout << "err: block is not ended." << endl;
        return false;
    }

    auto &last_t = tokens[tokens.size() - 1];
    while (indent_stack.empty() == false) {
        indented.push_back(Token(TokenType::DEDENT, last_t.first_idx, last_t.last_idx, last_t.indent));
        indent_stack.pop();
    }

    indented.push_back(Token(TokenType::END_OF_FILE, last_t.first_idx, last_t.last_idx, last_t.indent));

    return true;
}

void PawPrintLoader::_initParsingTable () {
    if (parsing_table_ != null)
        return;

    // load table binary data
	ifstream f("paw_print.tab", ifstream::binary);

	// get length of file
	f.seekg(0, f.end);
	int size = f.tellg();
	f.seekg(0, f.beg);
    
	// allocate memory
    vector<unsigned char> binary;
    binary.resize(size);

	// read data
	f.read((char*)binary.data(), size);
	f.close();


    parsing_table_ = make_shared<ParsingTable>(binary);


    Token::to_string_func = [](const char *text, const Token *t) {
        stringstream ss;
        ss << "Token(";
        switch (t->type) {
            case TokenType::INDENT      : ss << "INDENT)"       ; return ss.str();
            case TokenType::DEDENT      : ss << "DEDENT)"       ; return ss.str();
            case TokenType::END_OF_FILE : ss << "END_OF_FILE)"  ; return ss.str();
            case TokenType::INT         : ss << "INT, "         ; break;
            case TokenType::DOUBLE      : ss << "DOUBLE, "      ; break;
            case TokenType::STRING      : ss << "STRING, "      ; break;
            case TokenType::COLON       : ss << "COLON, "       ; break;
            case TokenType::COMMA       : ss << "COMMA, "       ; break;
            case TokenType::DASH        : ss << "DASH, "        ; break;
            case TokenType::SHARP       : ss << "SHARP, "       ; break;
            case TokenType::SQUARE_OPEN : ss << "SQUARE_OPEN, " ; break;
            case TokenType::SQUARE_CLOSE: ss << "SQUARE_CLOSE, "; break;
            case TokenType::CURLY_OPEN  : ss << "CURLY_OPEN, "  ; break;
            case TokenType::CURLY_CLOSE : ss << "CURLY_CLOSE, " ; break;
        }

        auto size = t->last_idx - t->first_idx + 2;
        auto str = new char [size];
        memcpy(str, &text[t->first_idx], size-1);
        str[size-1] = 0;
        ss << t->first_idx << ", " << t->last_idx << ", indent:" << t->indent
                << ", \"" << str << "\")";
        delete[] str;
        return ss.str();
    };
}

shared_ptr<PawPrint> PawPrintLoader::loadText (const char *text) {
    _initParsingTable();

    // tokenize
    vector<Token> tokens;
    auto is_tokenize_ok = tokenize(text, tokens);
    if (is_tokenize_ok == false)
        return null;

    vector<Token> indented;
    addIndentTokens(tokens, indented);

    for (auto &t : indented)
        cout << t.toString(text) << endl;


    // parse
    auto root = parsing_table_->generateParseTree(text, indented);
    cout << root->toString(text, 0, true) << endl;

    auto paw = make_shared<PawPrint>();

    return paw;
}

}
