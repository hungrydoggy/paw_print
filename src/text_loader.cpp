#include "./paw_print.h"

#include <iostream>


using std::cout;
using std::endl;



namespace paw_print {


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
                return false;
            }
            point_idx = ci;
        }else if (c < '0' || c > '9') {
            // TODO err not number
            return false;
        }
    }


    auto is_int = point_idx > last_idx;
    auto type = (is_int == true)? Token::INT : Token::DOUBLE;

    tokens.push_back(Token(type, first_idx, last_idx, indent));

    return true;
}

static bool _addStringToken (
        const char *text,
        int first_idx,
        int last_idx,
        int indent,
        vector<Token> &tokens) {

    tokens.push_back(Token(Token::STRING, first_idx, last_idx, indent));

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
                return -1;
            case '\n':
                //TODO err text is newlined without closing quote
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

                tokens.push_back(Token(Token::COMMA, idx, idx, indent));

                return idx + 1;
            }
            case ':': {
                if (idx > first_idx) {
                    auto is_ok = _addWordToken(text, first_idx, idx - 1, indent, tokens);
                    if (is_ok == false)
                        return -1;
                }

                tokens.push_back(Token(Token::COLON, idx, idx, indent));

                indent += 4;

                return idx + 1;
            }
            case '-': {
                if (idx > first_idx) {
                    auto is_ok = _addWordToken(text, first_idx, idx - 1, indent, tokens);
                    if (is_ok == false)
                        return -1;
                }

                tokens.push_back(Token(Token::DASH, idx, idx, indent));

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

                tokens.push_back(Token(Token::CURLY_OPEN, idx, idx, indent));

                return idx + 1;
            }
            case '}': {
                if (idx > first_idx) {
                    auto is_ok = _addWordToken(text, first_idx, idx - 1, indent, tokens);
                    if (is_ok == false)
                        return -1;
                }

                tokens.push_back(Token(Token::CURLY_CLOSE, idx, idx, indent));

                return idx + 1;
            }
            case '[': {
                if (idx > first_idx) {
                    auto is_ok = _addWordToken(text, first_idx, idx - 1, indent, tokens);
                    if (is_ok == false)
                        return -1;
                }

                tokens.push_back(Token(Token::SQUARE_OPEN, idx, idx, indent));

                return idx + 1;
            }
            case ']': {
                if (idx > first_idx) {
                    auto is_ok = _addWordToken(text, first_idx, idx - 1, indent, tokens);
                    if (is_ok == false)
                        return -1;
                }

                tokens.push_back(Token(Token::SQUARE_CLOSE, idx, idx, indent));

                return idx + 1;
            }
        }
    }

    return idx + 1;
}

bool PawPrint::tokenize (const char *text, vector<Token> &tokens) {
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

// return next idx
int PawPrint::_parse_step (const char *text, const vector<Token> &tokens, int start_idx) {
    auto idx = start_idx;
    auto &t = tokens[idx];
    switch (t.type) {
        case Token::INT:
            return idx + 1;
        case Token::DOUBLE:
            return idx + 1;
        case Token::STRING:
            return idx + 1;
        case Token::COLON:
            return idx + 1;
        case Token::COMMA:
            return idx + 1;
        case Token::DASH:
            return idx + 1;
        case Token::CURLY_OPEN:
            curly_open_idx_stack_.push(idx);
            beginMap();
            return idx + 1;
        case Token::CURLY_CLOSE:
            curly_open_idx_stack_.pop();
            endMap();
            return idx + 1;
        case Token::SQUARE_OPEN:
            square_open_idx_stack_.push(idx);
            beginSequence();
            return idx + 1;
        case Token::SQUARE_CLOSE:
            square_open_idx_stack_.pop();
            endSequence();
            return idx + 1;
    }

    return idx + 1;
}

bool PawPrint::addIndentTokens (const vector<Token> &tokens, vector<Token> &indented) {
    if (tokens.size() <= 0)
        return false;

    indented.push_back(tokens[0]);

    stack<Token::Type> block_stack;
    stack<int> indent_stack;
    
    int pre_t_idx = 0;
    for (int ti=1; ti<tokens.size(); ++ti) {
        auto &pre_t = tokens[pre_t_idx];
        auto &t = tokens[ti];

        // close block
        switch (t.type) {
            case Token::SQUARE_CLOSE:
                if (block_stack.top() != Token::SQUARE_CLOSE) {
                    // TODO err: block is not matched {t.first_idx}
                    return false;
                }
                block_stack.pop();
                break;
            case Token::CURLY_CLOSE:
                if (block_stack.top() != Token::CURLY_CLOSE) {
                    // TODO err: block is not matched {t.first_idx}
                    return false;
                }
                block_stack.pop();
                break;
        }

        // insert indent/dedent
        if (block_stack.empty() == true) {
            if (t.indent > pre_t.indent) {
                indented.push_back(Token(Token::INDENT, t.first_idx, t.last_idx, t.indent));
                indent_stack.push(t.indent);
            }else if (t.indent < pre_t.indent){
                while (t.indent < indent_stack.top()) {
                    indented.push_back(Token(Token::DEDENT, t.first_idx, t.last_idx, t.indent));
                    indent_stack.pop();
                }
            }
        }

        // open block
        switch (t.type) {
            case Token::SQUARE_OPEN:
            case Token::CURLY_OPEN:
                block_stack.push(t.type);
                break;
        }

        indented.push_back(t);
        pre_t_idx = ti;
    }

    if (block_stack.empty() == false) {
        // TODO err: block is not ended
        return false;
    }

    auto &last_t = tokens[tokens.size() - 1];
    while (indent_stack.empty() == false) {
        indented.push_back(Token(Token::DEDENT, last_t.first_idx, last_t.last_idx, last_t.indent));
        indent_stack.pop();
    }

    return true;
}

bool PawPrint::loadText (const char *text) {
    raw_data_.clear();

    vector<Token> tokens;
    auto is_tokenize_ok = tokenize(text, tokens);
    if (is_tokenize_ok == false)
        return false;

    vector<Token> indented;
    addIndentTokens(tokens, indented);

    for (auto &t : indented)
        cout << t.toString(text) << endl;

    // parse


	// write raw_data

    return true;
}

}
