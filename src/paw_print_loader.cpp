#include "./paw_print_loader.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "./paw_print.h"


using std::cout;
using std::endl;
using std::ifstream;
using std::istringstream;
using std::stringstream;


namespace paw_print {


shared_ptr<ParsingTable> PawPrintLoader::parsing_table_;


static void _parseNode (
        const char *text,
		const shared_ptr<PawPrint> &paw,
		const shared_ptr<Node> &node);


static bool _addNumberToken (
        const char *text,
        int first_idx,
        int last_idx,
        unsigned short indent,
        unsigned short column,
        unsigned short line,
        vector<Token> &tokens) {

    auto num_first_idx = first_idx;
    if (text[first_idx] == '-')
        ++num_first_idx;

    // find point idx and check number format
    auto point_idx = last_idx + 1;
    for (int ci=num_first_idx; ci<=last_idx; ++ci) {
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

    tokens.push_back(Token(type, first_idx, last_idx, indent, column, line));

    return true;
}

static bool _addStringToken (
        const char *text,
        int first_idx,
        int last_idx,
        unsigned short indent,
        unsigned short column,
        unsigned short line,
		bool check_bool,
        vector<Token> &tokens) {

	if (check_bool == true) {
		auto str = string(text + first_idx, last_idx - first_idx + 1);
		if (str == "true" || str == "false") {
			tokens.push_back(Token(TokenType::BOOL, first_idx, last_idx, indent, column, line));
			return true;
		}
	}

    tokens.push_back(Token(TokenType::STRING, first_idx, last_idx, indent, column, line));

    return true;
}

static bool _addWordToken (
        const char *text,
        int first_idx,
        int last_idx,
        unsigned short indent,
        unsigned short column,
        unsigned short line,
        vector<Token> &tokens) {

    if (text[first_idx] >= '0' && text[first_idx] <= '9')
        return _addNumberToken(text, first_idx, last_idx, indent, column, line, tokens);
    else if (text[first_idx] == '-' && text[first_idx+1] >= '0' && text[first_idx+1] <= '9')
        return _addNumberToken(text, first_idx, last_idx, indent, column, line, tokens);
    else
        return _addStringToken(text, first_idx, last_idx, indent, column, line, true, tokens);
}

// return next idx
static int _findAndAddToken_quoted(
        char quote_char,
        const char *text,
        int start_idx,
        unsigned short indent,
        unsigned short column,
        unsigned short line,
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
                    auto is_ok = _addStringToken(
                            text, start_idx, idx - 1, indent, column, line, false, tokens);
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
        unsigned short &indent,
        unsigned short &column,
        unsigned short &line) {

    int idx = start_idx;

    // trim left and indent
    bool need_update_indent = false;
    for (; true; ++idx, ++column) {
        const char c = text[idx];
        if (c == 0) {
            return idx;
        }if (c == '\n') {
            tokens.push_back(
                    Token(
                        TokenType::NEW_LINE,
                        idx,
                        idx,
                        indent,
                        column,
                        line));
            indent = 0;
            column = 0;
            ++line;
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
    for (; true; ++idx, ++column) {
        const char c = text[idx];
        switch (c) {
            case 0:
				if (idx > first_idx) {
					auto is_ok = _addWordToken(
						text, first_idx, idx - 1, indent, column, line, tokens);
					if (is_ok == false)
						return -1;
				}
                return idx;
			case '\r':
            case ' ': {
				if (idx > first_idx) {
					auto is_ok = _addWordToken(
                            text, first_idx, idx - 1, indent, column, line, tokens);
					if (is_ok == false)
						return -1;
				}
                return idx + 1;
            }

			case '#': {
				if (idx > first_idx) {
					auto is_ok = _addWordToken(
                            text, first_idx, idx - 1, indent, column, line, tokens);
					if (is_ok == false)
						return -1;
				}
				auto next_idx = _findAndSkip_comment(text, idx + 1);
                column += next_idx - (idx + 1);
                return next_idx;
			}

            case '\n': {
				if (idx > first_idx) {
					auto is_ok = _addWordToken(
                            text, first_idx, idx - 1, indent, column, line, tokens);
					if (is_ok == false)
						return -1;
				}
                return idx;
            }
            case ',': {
                if (idx > first_idx) {
                    auto is_ok = _addWordToken(
                            text, first_idx, idx - 1, indent, column, line, tokens);
                    if (is_ok == false)
                        return -1;
                }

                tokens.push_back(
                        Token(TokenType::COMMA, idx, idx, indent, column, line));

                return idx + 1;
            }
            case ':': {
                if (idx > first_idx) {
                    auto is_ok = _addWordToken(
                            text, first_idx, idx - 1, indent, column, line, tokens);
                    if (is_ok == false)
                        return -1;
                }

                tokens.push_back(Token(TokenType::COLON, idx, idx, indent, column, line));

                return idx + 1;
            }
            case '-': {
                auto next = text[idx + 1];
                if (next != ' ' && next != '\r' && next != '\n')
                    continue;

                if (idx > first_idx) {
                    auto is_ok = _addWordToken(
                            text, first_idx, idx - 1, indent, column, line, tokens);
                    if (is_ok == false)
                        return -1;
                }

                tokens.push_back(Token(TokenType::DASH, idx, idx, indent, column, line));

                return idx + 1;
            }
            case '\"': {
                if (idx > first_idx) {
                    auto is_ok = _addWordToken(
                            text, first_idx, idx - 1, indent, column, line, tokens);
                    if (is_ok == false)
                        return -1;
                }

                auto next_idx = _findAndAddToken_quoted(
                        '\"', text, idx + 1, indent, column, line, tokens);
                column += next_idx - (idx + 1);
                return next_idx;
            }
            case '\'': {
                if (idx > first_idx) {
                    auto is_ok = _addWordToken(
                            text, first_idx, idx - 1, indent, column, line, tokens);
                    if (is_ok == false)
                        return -1;
                }

                auto next_idx = _findAndAddToken_quoted(
                        '\'', text, idx + 1, indent, column, line, tokens);
                column += next_idx - (idx + 1);
                return next_idx;
            }
            case '{': {
                if (idx > first_idx) {
                    auto is_ok = _addWordToken(
                            text, first_idx, idx - 1, indent, column, line, tokens);
                    if (is_ok == false)
                        return -1;
                }

                tokens.push_back(
                        Token(TokenType::CURLY_OPEN, idx, idx, indent, column, line));

                return idx + 1;
            }
            case '}': {
                if (idx > first_idx) {
                    auto is_ok = _addWordToken(
                            text, first_idx, idx - 1, indent, column, line, tokens);
                    if (is_ok == false)
                        return -1;
                }

                tokens.push_back(
                        Token(TokenType::CURLY_CLOSE, idx, idx, indent, column, line));

                return idx + 1;
            }
            case '[': {
                if (idx > first_idx) {
                    auto is_ok = _addWordToken(
                            text, first_idx, idx - 1, indent, column, line, tokens);
                    if (is_ok == false)
                        return -1;
                }

                tokens.push_back(
                        Token(TokenType::SQUARE_OPEN, idx, idx, indent, column, line));

                return idx + 1;
            }
            case ']': {
                if (idx > first_idx) {
                    auto is_ok = _addWordToken(
                            text, first_idx, idx - 1, indent, column, line, tokens);
                    if (is_ok == false)
                        return -1;
                }

                tokens.push_back(Token(
                        TokenType::SQUARE_CLOSE, idx, idx, indent, column, line));

                return idx + 1;
            }
        }
    }

    return idx + 1;
}

bool PawPrintLoader::tokenize (const char *text, vector<Token> &tokens) {
    int idx = 0;

	// remove BOM
	if (text[0] == -17 && text[1] == -69, text[2] == -65)
		idx = 3;

    // trim left and indent
    unsigned short indent = 0;
    unsigned short column = 1;
    unsigned short line = 1;
    for (; true; ++idx, ++column) {
        const char c = text[idx];
        if (c == 0) {
            return idx - 1;
        }if (c == '\n') {
            indent = 0;
            column = 0;
            ++line;
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
        idx = _findAndAddToken(text, idx, tokens, indent, column, line);
        if (idx < 0)
            return false;
    }

    return true;
}

bool PawPrintLoader::preprocessTokens (
        const vector<Token> &tokens, vector<Token> &preprocessed) {

    if (tokens.size() <= 0)
        return false;

    preprocessed.clear();


    for (int ti=0; ti<tokens.size(); ++ti) {
        auto &t = tokens[ti];

		// skip newline of first line
		if (ti == 0 && t.type == TokenType::NEW_LINE)
			continue;

        // reduce newline chains
        if (t.type == TokenType::NEW_LINE &&
                ti - 1 >= 0 &&
                tokens[ti-1].type == TokenType::NEW_LINE) {
            continue;
        }
        
        preprocessed.push_back(t);
    }


    // make tokens end with NEW_LINE
    auto &last_t = preprocessed[preprocessed.size() - 1];
    if (last_t.type != TokenType::NEW_LINE) {
        preprocessed.push_back(
                Token(
                    TokenType::NEW_LINE,
                    last_t.first_idx,
                    last_t.last_idx,
                    last_t.indent,
                    last_t.column,
                    last_t.line));
    }

    return true;
}

bool PawPrintLoader::addIndentTokens (const vector<Token> &tokens, vector<Token> &indented) {
    if (tokens.size() <= 0)
        return false;

    stack<int> block_stack;
    stack<int> indent_stack;
    
    int pre_t_idx = -1;
    for (int ti=0; ti<tokens.size(); ++ti) {
        auto &t = tokens[ti];

        // close block
        switch (t.type) {
            case TokenType::SQUARE_CLOSE:
                if (block_stack.empty() == true || block_stack.top() != TokenType::SQUARE_OPEN) {
                    // TODO err: block is not matched {t.first_idx}
                    cout << "err: block [] is not matched. idx : " << t.first_idx << endl;
                    return false;
                }
                block_stack.pop();
                break;
            case TokenType::CURLY_CLOSE:
                if (block_stack.empty() == true || block_stack.top() != TokenType::CURLY_OPEN) {
                    // TODO err: block is not matched {t.first_idx}
                    cout << "err: block {} is not matched. idx : " << t.first_idx << endl;
                    return false;
                }
                block_stack.pop();
                break;
        }

        // insert indent/dedent
        //if (block_stack.empty() == true && pre_t_idx >= 0) {
        if (pre_t_idx >= 0) {
			auto &pre_t = tokens[pre_t_idx];
            if (t.indent > pre_t.indent) {
                indented.push_back(
                        Token(
                            TokenType::INDENT,
                            t.first_idx,
                            t.last_idx,
                            t.indent,
                            t.column,
                            t.line));
                indent_stack.push(t.indent);
            }else if (t.indent < pre_t.indent){
                while (t.indent < indent_stack.top()) {
                    indented.push_back(
                            Token(
                                TokenType::DEDENT,
                                t.first_idx,
                                t.last_idx,
                                t.indent,
                                t.column,
                                t.line));
                    indent_stack.pop();

					if (indent_stack.empty() == true) {
						if (t.indent == 0) {
							break;
						}else {
							cout << "err: indentation error on line:" << t.line
								<< ", column:" << t.column << endl;
							return false;
						}
					}
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
        indented.push_back(
                Token(
                    TokenType::DEDENT,
                    last_t.first_idx,
                    last_t.last_idx,
                    last_t.indent,
                    last_t.column,
                    last_t.line));
        indent_stack.pop();
    }

    indented.push_back(
            Token(
                TokenType::END_OF_FILE,
                last_t.first_idx,
                last_t.last_idx,
                last_t.indent,
                last_t.column,
                last_t.line));

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
            case TokenType::INDENT      : ss << "INDENT, "      ; break;
            case TokenType::DEDENT      : ss << "DEDENT, "      ; break;
            case TokenType::END_OF_FILE : ss << "END_OF_FILE, " ; break;
            case TokenType::BOOL        : ss << "BOOL, "        ; break;
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
            case TokenType::NEW_LINE    : ss << "NEW_LINE, "    ; break;
        }

        auto size = t->last_idx - t->first_idx + 2;
        auto str = new char [size];
        memcpy(str, &text[t->first_idx], size-1);
        str[size-1] = 0;
        ss << t->first_idx << ", " << t->last_idx << ", indent:" << t->indent
                << ", line:" << t->line
                << ", column:" << t->column
                << ", \"" << str << "\")";
        delete[] str;
        return ss.str();
    };
}



static string _makeEscapedString (const string &input_str) {
	string result;
	result.resize(input_str.size());

	auto src =        input_str.data();
	auto dst = (char*)result   .data();

	int di = 0;
	for (int si = 0; si < input_str.size(); ++si, ++di) {
		if (src[si] != '\\') {
			dst[di] = src[si];
			continue;
		}

		++si;
		switch (src[si]) {
		case 'b' : dst[di] = '\b'; break;
		case 'f' : dst[di] = '\f'; break;
		case 'n' : dst[di] = '\n'; break;
		case 'r' : dst[di] = '\r'; break;
		case 't' : dst[di] = '\t'; break;
		case '\\': dst[di] = '\\'; break;
		case '\'': dst[di] = '\''; break;
		case '\"': dst[di] = '\"'; break;
		default:
			dst[di] = '\\';
			++di;
			dst[di] = src[si];
			break;
		}
	}

	result.resize(di);
	return result;
}

using LoaderFunc = function<
        void (
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node)>;
vector<LoaderFunc> loader_funcs;

static void _initLoaders () {
    if (loader_funcs.size() > 0)
        return;


    //# Rule 0 : S' -> S 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        auto &children = node->children();
        _parseNode(text, paw, children[0]);
    });

    //# Rule 1 : S -> NODE 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        auto &children = node->children();
        _parseNode(text, paw, children[0]);
    });

    //# Rule 2 : KEY -> bool 
    auto key_func = [](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        auto &children = node->children();
		auto key_token = children[0]->token();
		paw->pushKey(
				string(
					text + key_token->first_idx,
					key_token->last_idx - key_token->first_idx + 1),
                key_token->column,
                key_token->line);
    };
    loader_funcs.push_back(key_func);

    //# Rule 3 : KEY -> int 
    loader_funcs.push_back(key_func);

    //# Rule 4 : KEY -> double 
    loader_funcs.push_back(key_func);

    //# Rule 5 : KEY -> string 
    loader_funcs.push_back(key_func);

    //# Rule 6 : KV -> KEY colon NODE #new_line 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        auto &children = node->children();

        _parseNode(text, paw, children[0]);
        _parseNode(text, paw, children[2]);
    });

    //# Rule 7 : KV -> KEY colon #new_line #indent NODE #new_line #dedent 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        auto &children = node->children();

        _parseNode(text, paw, children[0]);
        _parseNode(text, paw, children[4]);
    });

    //# Rule 8 : KV -> KEY colon #new_line 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        auto &children = node->children();

        _parseNode(text, paw, children[0]);
        paw->pushNull();
    });

    //# Rule 9 : MAP -> KV MAP 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        auto need_wrap = node->parent() == null || node->parent()->reduced_rule_idx() != 9;

        if (need_wrap == true)
            paw->beginMap();

        auto &children = node->children();
        _parseNode(text, paw, children[0]);
        _parseNode(text, paw, children[1]);

        if (need_wrap == true)
            paw->endMap();
    });


    //# Rule 10 : MAP -> KV 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        auto need_wrap = node->parent() == null || node->parent()->reduced_rule_idx() != 9;

        if (need_wrap == true)
            paw->beginMap();

        auto &children = node->children();
        _parseNode(text, paw, children[0]);

        if (need_wrap == true)
            paw->endMap();
    });

    //# Rule 11 : CURL_MAP -> curly_open curly_close 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        paw->beginMap();
        paw->endMap();
    });

    //# Rule 12 : CURL_MAP -> curly_open #new_line curly_close 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        paw->beginMap();
        paw->endMap();
    });

    //# Rule 13 : CURL_MAP -> curly_open MAP_BLOCKED curly_close 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        paw->beginMap();

        auto &children = node->children();
        _parseNode(text, paw, children[1]);

        paw->endMap();
    });

    //# Rule 14 : CURL_MAP -> curly_open #new_line #indent MAP_BLOCKED #dedent curly_close 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        paw->beginMap();

        auto &children = node->children();
        _parseNode(text, paw, children[3]);

        paw->endMap();
    });
    
    //# Rule 15 : KV_BLOCKED -> KEY colon NODE 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        auto &children = node->children();

        _parseNode(text, paw, children[0]);
        _parseNode(text, paw, children[2]);
    });

    //# Rule 16 : KV_BLOCKED -> KEY colon 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        auto &children = node->children();

        _parseNode(text, paw, children[0]);
        paw->pushNull();
    });

    //# Rule 17 : MAP_BLOCKED -> KV_BLOCKED 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        auto &children = node->children();
        _parseNode(text, paw, children[0]);
    });

    //# Rule 18 : MAP_BLOCKED -> KV_BLOCKED comma MAP_BLOCKED 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        auto &children = node->children();
        _parseNode(text, paw, children[0]);
        _parseNode(text, paw, children[2]);
    });

    //# Rule 19 : MAP_BLOCKED -> KV_BLOCKED comma #new_line MAP_BLOCKED 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        auto &children = node->children();
        _parseNode(text, paw, children[0]);
        _parseNode(text, paw, children[3]);
    });

    //# Rule 20 : SEQ_ELEM -> dash NODE 
	loader_funcs.push_back([](
		const char *text,
		const shared_ptr<PawPrint> &paw,
		const shared_ptr<Node> &node) {

		auto &children = node->children();
		_parseNode(text, paw, children[1]);
	});

    //# Rule 21 : SEQ_ELEM -> dash #new_line #indent NODE #new_line #dedent 
	loader_funcs.push_back([](
		const char *text,
		const shared_ptr<PawPrint> &paw,
		const shared_ptr<Node> &node) {

		auto &children = node->children();
		_parseNode(text, paw, children[3]);
	});

    //# Rule 22 : SEQUENCE -> SEQ_ELEM SEQUENCE 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        auto need_wrap = node->parent() == null || node->parent()->reduced_rule_idx() != 22;

        if (need_wrap == true)
            paw->beginSequence();

        auto &children = node->children();
        _parseNode(text, paw, children[0]);
        _parseNode(text, paw, children[1]);

        if (need_wrap == true)
            paw->endSequence();
    });

    //# Rule 23 : SEQUENCE -> SEQ_ELEM 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        auto need_wrap = node->parent() == null || node->parent()->reduced_rule_idx() != 22;

        if (need_wrap == true)
            paw->beginSequence();

        auto &children = node->children();
        _parseNode(text, paw, children[0]);

        if (need_wrap == true)
            paw->endSequence();
    });

    //# Rule 24 : SQUARE_SEQ -> square_open square_close 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        paw->beginSequence();
        paw->endSequence();
    });

    //# Rule 25 : SQUARE_SEQ -> square_open SEQ_BLOCKED square_close 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        paw->beginSequence();

        auto &children = node->children();
        _parseNode(text, paw, children[1]);

        paw->endSequence();
    });

    //# Rule 26 : SQUARE_SEQ -> square_open #new_line square_close 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        paw->beginSequence();
        paw->endSequence();
    });

    //# Rule 27 : SQUARE_SEQ -> square_open #new_line #indent SEQ_BLOCKED #dedent #new_line square_close 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        paw->beginSequence();

        auto &children = node->children();
        _parseNode(text, paw, children[3]);

        paw->endSequence();
    });

    //# Rule 28 : SEQ_BLOCKED -> NODE comma SEQ_BLOCKED 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        auto &children = node->children();
        _parseNode(text, paw, children[0]);
        _parseNode(text, paw, children[2]);
    });
    
    //# Rule 29 : SEQ_BLOCKED -> NODE comma #new_line SEQ_BLOCKED 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        auto &children = node->children();
        _parseNode(text, paw, children[0]);
        _parseNode(text, paw, children[3]);
    });
    
    //# Rule 30 : SEQ_BLOCKED -> NODE 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        auto &children = node->children();
        _parseNode(text, paw, children[0]);
    });

    //# Rule 31 : NODE -> bool 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        auto &children = node->children();
        auto token = children[0]->token();
        auto str = string(
                text + token->first_idx,
                token->last_idx - token->first_idx + 1);
        paw->pushBool((str == "true")? true: false, token->column, token->line);
    });

    //# Rule 32 : NODE -> bool #new_line 
    loader_funcs.push_back(loader_funcs[loader_funcs.size()-1]);

    //# Rule 33 : NODE -> int 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        auto &children = node->children();
        auto token = children[0]->token();
        istringstream iss(
                string(
                    text + token->first_idx,
                    token->last_idx - token->first_idx + 1));
        int v = 0;
        iss >> v;
        paw->pushInt(v, token->column, token->line);
    });

    //# Rule 34 : NODE -> int #new_line 
    loader_funcs.push_back(loader_funcs[loader_funcs.size()-1]);

    //# Rule 35 : NODE -> double 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        auto &children = node->children();
        auto token = children[0]->token();
        istringstream iss(
                string(
                    text + token->first_idx,
                    token->last_idx - token->first_idx + 1));
        double v = 0;
        iss >> v;
        paw->pushDouble(v, token->column, token->line);
    });

    //# Rule 36 : NODE -> double #new_line 
    loader_funcs.push_back(loader_funcs[loader_funcs.size()-1]);

    //# Rule 37 : NODE -> string 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        auto &children = node->children();
        auto token = children[0]->token();
        paw->pushString(
                _makeEscapedString(
					string(
						text + token->first_idx,
						token->last_idx - token->first_idx + 1)),
                token->column,
                token->line);
    });

    //# Rule 38 : NODE -> string #new_line 
    loader_funcs.push_back(loader_funcs[loader_funcs.size()-1]);

    //# Rule 39 : NODE -> MAP 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        auto &children = node->children();
        _parseNode(text, paw, children[0]);
    });

    //# Rule 40 : NODE -> CURL_MAP 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        auto &children = node->children();
        _parseNode(text, paw, children[0]);
    });

    //# Rule 41 : NODE -> CURL_MAP #new_line 
    loader_funcs.push_back(loader_funcs[loader_funcs.size()-1]);

    //# Rule 42 : NODE -> SEQUENCE 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        auto &children = node->children();
        _parseNode(text, paw, children[0]);
    });

    //# Rule 43 : NODE -> SQUARE_SEQ 
    loader_funcs.push_back([](
            const char *text,
            const shared_ptr<PawPrint> &paw,
            const shared_ptr<Node> &node){

        auto &children = node->children();
        _parseNode(text, paw, children[0]);
    });

    //# Rule 44 : NODE -> SQUARE_SEQ #new_line 
    loader_funcs.push_back(loader_funcs[loader_funcs.size()-1]);


    //##### Rules
    //# Rule 0 : S' -> S 
    //# Rule 1 : S -> NODE 
    //# Rule 2 : KEY -> bool 
    //# Rule 3 : KEY -> int 
    //# Rule 4 : KEY -> double 
    //# Rule 5 : KEY -> string 
    //# Rule 6 : KV -> KEY colon NODE 
    //# Rule 7 : KV -> KEY colon #new_line #indent NODE #dedent 
    //# Rule 8 : KV -> KEY colon #new_line 
    //# Rule 9 : MAP -> KV MAP 
    //# Rule 10 : MAP -> KV 
    //# Rule 11 : CURL_MAP -> curly_open curly_close 
    //# Rule 12 : CURL_MAP -> curly_open #new_line curly_close 
    //# Rule 13 : CURL_MAP -> curly_open MAP_BLOCKED curly_close 
    //# Rule 14 : CURL_MAP -> curly_open #new_line #indent MAP_BLOCKED #dedent curly_close 
    //# Rule 15 : KV_BLOCKED -> KEY colon NODE 
    //# Rule 16 : KV_BLOCKED -> KEY colon 
    //# Rule 17 : MAP_BLOCKED -> KV_BLOCKED 
    //# Rule 18 : MAP_BLOCKED -> KV_BLOCKED comma MAP_BLOCKED 
    //# Rule 19 : MAP_BLOCKED -> KV_BLOCKED comma #new_line MAP_BLOCKED 
    //# Rule 20 : SEQ_ELEM -> dash NODE 
    //# Rule 21 : SEQ_ELEM -> dash #new_line #indent NODE #dedent 
    //# Rule 22 : SEQUENCE -> SEQ_ELEM SEQUENCE 
    //# Rule 23 : SEQUENCE -> SEQ_ELEM 
    //# Rule 24 : SQUARE_SEQ -> square_open square_close 
    //# Rule 25 : SQUARE_SEQ -> square_open SEQ_BLOCKED square_close 
    //# Rule 26 : SQUARE_SEQ -> square_open #new_line square_close 
    //# Rule 27 : SQUARE_SEQ -> square_open #new_line #indent SEQ_BLOCKED #dedent square_close 
    //# Rule 28 : SEQ_BLOCKED -> NODE comma SEQ_BLOCKED 
    //# Rule 29 : SEQ_BLOCKED -> NODE comma #new_line SEQ_BLOCKED 
    //# Rule 30 : SEQ_BLOCKED -> NODE 
    //# Rule 31 : NODE -> bool 
    //# Rule 32 : NODE -> bool #new_line 
    //# Rule 33 : NODE -> int 
    //# Rule 34 : NODE -> int #new_line 
    //# Rule 35 : NODE -> double 
    //# Rule 36 : NODE -> double #new_line 
    //# Rule 37 : NODE -> string 
    //# Rule 38 : NODE -> string #new_line 
    //# Rule 39 : NODE -> MAP 
    //# Rule 40 : NODE -> CURL_MAP 
    //# Rule 41 : NODE -> CURL_MAP #new_line 
    //# Rule 42 : NODE -> SEQUENCE 
    //# Rule 43 : NODE -> SQUARE_SEQ 
    //# Rule 44 : NODE -> SQUARE_SEQ #new_line 

}

static LoaderFunc getLoader (int rule_idx) {
    _initLoaders();

    if (rule_idx < 0 || rule_idx >= loader_funcs.size())
        return null;
    return loader_funcs[rule_idx];
}

static void _parseNode (
        const char *text,
		const shared_ptr<PawPrint> &paw,
		const shared_ptr<Node> &node) {

    if (node->termnon()->isTerminal() == true)
        return;

	auto loader = getLoader(node->reduced_rule_idx());
    if (loader == null) {
        // TODO err: can not parse node
        cout << "err: can not parse node \""
                << node->toString(text) << "\" by rule_idx : "
                << node->reduced_rule_idx()
                << endl;
        return;
    }
    
    return loader(text, paw, node);
}

shared_ptr<PawPrint> PawPrintLoader::loadText (const string &name, const char *text) {
    _initParsingTable();

    // tokenize
    vector<Token> tokens;
    auto is_tokenize_ok = tokenize(text, tokens);
    if (is_tokenize_ok == false)
        return null;

    /*
    cout << "##### raw tokens" << endl;
    for (auto &t: tokens)
        cout << t.toString(text) << endl; //*/

    vector<Token> preprocessed;
    preprocessTokens(tokens, preprocessed);

    /*
    cout << "##### preprocessed" << endl;
    for (auto &t: preprocessed)
        cout << t.toString(text) << endl; //*/

    vector<Token> indented;
    addIndentTokens(preprocessed, indented);

    /*
    cout << "##### indented" << endl;
    for (auto &t: indented)
        cout << t.toString(text) << endl; //*/

    // parse
    auto root = parsing_table_->generateParseTree(text, indented);
	if (root == null)
		return null;

    /*
    cout << root->toString(text, 0, true) << endl; //*/

    auto paw = make_shared<PawPrint>(name);
	_parseNode(text, paw, root);

    return paw;
}

}
