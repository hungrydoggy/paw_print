
#include <assert.h>
#include <fstream>
#include <iostream>
#include <stdio.h>

#include "./src/paw_print.h"
#include "./src/parse_table.h"


using namespace paw_print;

using std::cout;
using std::endl;
using std::ifstream;
using std::to_string;

static void _t_basic () {
    PawPrint pp;

    /*
     * abc:
     *      a: 1
     *      b: 2
     *      c: 3
     *      d: ['i', 'j', 'k', 5, 6, 7]
     *      e:
     *          x: 9.0
     *          y: 8.0
     *          z: 7.0
	 *		f:
	 *			- first:
	 *				name: 'first'
	 *				value: 1
	 *			- second:
	 *				name: 'second'
	 *				value: 2
     */

    pp.beginMap();
        pp.pushKey("abc");
        pp.beginMap();
            pp.pushKey("a"); pp.pushInt(1);
            pp.pushKey("b"); pp.pushInt(2);
            pp.pushKey("c"); pp.pushInt(3);

            pp.pushKey("d");
            pp.beginSequence();
                pp.pushString("i");
                pp.pushString("j");
                pp.pushString("k");
                pp.pushInt(5);
                pp.pushInt(6);
                pp.pushInt(7);
            pp.endSequence();

            pp.pushKey("e");
            pp.beginMap();
                pp.pushKey("x"); pp.pushDouble(9.0);
                pp.pushKey("y"); pp.pushDouble(8.0);
                pp.pushKey("z"); pp.pushDouble(7.0);
            pp.endMap();

			pp.pushKey("f");
			pp.beginSequence();
				pp.beginMap();
					pp.pushKey("first");
					pp.beginMap();
						pp.pushKey("name"); pp.pushString("first");
						pp.pushKey("value"); pp.pushInt(1);
					pp.endMap();
				pp.endMap();
				pp.beginMap();
					pp.pushKey("second");
					pp.beginMap();
						pp.pushKey("name"); pp.pushString("second");
						pp.pushKey("value"); pp.pushInt(2);
					pp.endMap();
				pp.endMap();
			pp.endSequence();

        pp.endMap();
    pp.endMap();

	/*
    std::cout << "size : " << pp.raw_data().size() << std::endl;
    for (auto &c : pp.raw_data())
        std::cout << (int)c << ",";
    std::cout << std::endl;
	exit(0);//*/

	vector<unsigned char> correct_raw_data = { 6,8,3,4,0,97,98,99,0,6,8,3,2,0,97,0,1,1,0,0,0,8,3,2,0,98,0,1,2,0,0,0,8,3,2,0,99,0,1,3,0,0,0,8,3,2,0,100,0,4,3,2,0,105,0,3,2,0,106,0,3,2,0,107,0,1,5,0,0,0,1,6,0,0,0,1,7,0,0,0,5,8,3,2,0,101,0,6,8,3,2,0,120,0,2,0,0,0,0,0,0,34,64,8,3,2,0,121,0,2,0,0,0,0,0,0,32,64,8,3,2,0,122,0,2,0,0,0,0,0,0,28,64,7,8,3,2,0,102,0,4,6,8,3,6,0,102,105,114,115,116,0,6,8,3,5,0,110,97,109,101,0,3,6,0,102,105,114,115,116,0,8,3,6,0,118,97,108,117,101,0,1,1,0,0,0,7,7,6,8,3,7,0,115,101,99,111,110,100,0,6,8,3,5,0,110,97,109,101,0,3,7,0,115,101,99,111,110,100,0,8,3,6,0,118,97,108,117,101,0,1,2,0,0,0,7,7,5,7,7, };
	if (pp.raw_data().size() != correct_raw_data.size()) {
		std::cout << "err: raw_data size error" << std::endl;
		exit(-1);
	}

	for (int ri = 0; ri < correct_raw_data.size(); ++ri) {
		if (pp.raw_data()[ri] != correct_raw_data[ri]) {
			std::cout << "err: raw_data data error" << std::endl;
			exit(-1);
		}
	}

	
    /*
     * abc:
     *      a: 1
     *      b: 2
     *      c: 3
     *      d: ['i', 'j', 'k', 5, 6, 7]
     *      e:
     *          x: 9.0
     *          y: 8.0
     *          z: 7.0
	 *		f:
	 *			- first:
	 *				name: 'first'
	 *				value: 1
	 *			- second:
	 *				name: 'second'
	 *				value: 2
     */
    auto root = pp.root();
	assert(root["xyz"].type() == 0);
	assert(root["abc"].type() == 6);
	assert(root["abc"]["a"].type() == 1);
	assert(root["abc"]["a"].get(-1) == 1);
	assert(root["abc"]["b"].get(-1) == 2);
	assert(root["abc"]["c"].get(-1) == 3);
	assert(root["abc"]["d"].type() == 4);
	assert(strcmp(root["abc"]["d"][0].get(""), "i") == 0);
	assert(strcmp(root["abc"]["d"][1].get(""), "j") == 0);
	assert(strcmp(root["abc"]["d"][2].get(""), "k") == 0);
	assert(strcmp(root["abc"]["d"][3].get(""), "") == 0);
	assert(root["abc"]["d"][3].get(-1.0) == -1.0);
	assert(root["abc"]["d"][3].get(-1) == 5);
	assert(root["abc"]["d"][4].get(-2) == 6);
	assert(root["abc"]["d"][5].get(-3) == 7);
	assert(root["abc"]["e"].type() == 6);
	assert(root["abc"]["e"]["x"].get(-1) == -1);
	assert(root["abc"]["e"]["x"].get(-1.0) == 9.0);
	assert(root["abc"]["e"]["y"].get(-1.0) == 8.0);
	assert(root["abc"]["e"]["z"].get(-1.0) == 7.0);
	assert(root["abc"]["f"].type() == PawPrint::Data::TYPE_SEQUENCE);
	assert(root["abc"]["f"][0].type() == PawPrint::Data::TYPE_MAP);
	assert(root["abc"]["f"][0]["first"].type() == PawPrint::Data::TYPE_MAP);
	assert(root["abc"]["f"][0]["first"]["name"].type() == PawPrint::Data::TYPE_STRING);
	assert(root["abc"]["f"][0]["first"]["value"].type() == PawPrint::Data::TYPE_INT);
	assert(strcmp(root["abc"]["f"][0]["first"]["name"].get(""), "first") == 0);
	assert(root["abc"]["f"][0]["first"]["value"].get(-1) == 1);
	assert(strcmp(root["abc"]["f"][1]["second"]["name"].get(""), "second") == 0);
	assert(root["abc"]["f"][1]["second"]["value"].get(-1) == 2);
}

static bool _loadPaw(const string &path, PawPrint &paw) {
	ifstream is(path, std::ifstream::binary);

	// get length of file:
	is.seekg(0, is.end);
	int size = is.tellg();
	is.seekg(0, is.beg);
	// allocate memory:
	char* text = new char[size + 2];
	// read data as a block:
	is.read(text, size);
	text[size] = '\n';
	text[size + 1] = 0;
	is.close();

	auto result = paw.loadText(text);

	delete[] text;

	return result;
}

void _t_load_map_paws() {
	for (int pi = 0; pi <= 4; ++pi) {
		PawPrint paw;
		cout << "load_map_paw : " << pi << endl;
		auto is_ok = _loadPaw("../../paw/map_0" + to_string(pi) + ".paw", paw);
		assert(is_ok == true);
	}
}

void _t_load_boss_appear_snake_obj () {

	ifstream is("../../paw/boss_appear_snake.obj", std::ios::binary);
	// get length of file:
	is.seekg(0, is.end);
	int size = is.tellg();
	is.seekg(0, is.beg);
	// allocate memory:
	char* text = new char[size + 2];
	// read data as a block:
	is.read(text, size);
	text[size] = '\n';
	text[size + 1] = 0;
	is.close();


    PawPrint paw;
    vector<Token> tokens;
    paw.tokenize(text, tokens);
    vector<string> results = {
        "Token(STRING, 1, 33, indent:0)        @objects/ui/popup/boss_appear.obj",
        "Token(COLON, 35, 35, indent:0)        :",
        "Token(STRING, 41, 43, indent:4)        ref",
        "Token(COLON, 44, 44, indent:4)        :",
        "Token(STRING, 55, 62, indent:8)        boss_img",
        "Token(COLON, 64, 64, indent:8)        :",
        "Token(STRING, 67, 95, indent:12)        images/ui/popup/snake_art.png",
        "Token(STRING, 107, 114, indent:8)        icon_img",
        "Token(COLON, 116, 116, indent:8)        :",
        "Token(STRING, 119, 164, indent:12)        images/status/icons/inven_ico_snake_poison.png",
        "Token(STRING, 176, 183, indent:8)        icon_siz",
        "Token(COLON, 185, 185, indent:8)        :",
        "Token(SQUARE_OPEN, 187, 187, indent:12)        [",
        "Token(INT, 188, 189, indent:12)        79",
        "Token(COMMA, 190, 190, indent:12)        ,",
        "Token(INT, 192, 192, indent:12)        0",
        "Token(SQUARE_CLOSE, 193, 193, indent:12)        ]",
        "Token(STRING, 204, 211, indent:8)        icon_pos",
        "Token(COLON, 213, 213, indent:8)        :",
        "Token(DASH, 227, 227, indent:12)        -",
        "Token(INT, 229, 229, indent:12)        0",
        "Token(DASH, 243, 243, indent:12)        -",
        "Token(DOUBLE, 245, 248, indent:12)        10.5",
        "Token(DASH, 262, 262, indent:12)        -",
        "Token(INT, 264, 264, indent:12)        1",
    };
    assert(tokens.size() == results.size());

    for (int ti=0; ti<tokens.size(); ++ti) {
        auto s = tokens[ti].toString(text);
        assert(s == results[ti]);
    }
    

    paw.loadText(text);


    delete[] text;
}

static void _t_generateParsingTable () {

	auto term_int = make_shared<Terminal>("int", Token::INT);
	auto term_double = make_shared<Terminal>("double", Token::DOUBLE);
	auto term_string = make_shared<Terminal>("string", Token::STRING);
	auto term_colon = make_shared<Terminal>("colon", Token::COLON);
	auto term_comma = make_shared<Terminal>("comma", Token::COMMA);
	auto term_curly_open = make_shared<Terminal>("curly_open", Token::CURLY_OPEN);
	auto term_curly_close = make_shared<Terminal>("curly_close", Token::CURLY_CLOSE);

	auto non_kv = make_shared<Nonterminal>("KV");
	auto non_map = make_shared<Nonterminal>("MAP");

	auto non_kv_for_blocked_content = make_shared<Nonterminal>("KV_FOR_BLOCKED_CONTENTS");
	auto non_map_blocked_content    = make_shared<Nonterminal>("MAP_BLOCKED_CONTENTS"   );

	auto non_sequence = make_shared<Nonterminal>("SEQUENCE");

	auto non_node = make_shared<Nonterminal>("NODE");

	auto start = make_shared<Nonterminal>("S");

	ParsingTableGenerator generator;
	generator.addSymbol(start, true);
	generator.addSymbol(non_kv);
	generator.addSymbol(non_map);
	generator.addSymbol(non_kv_for_blocked_content);
	generator.addSymbol(non_map_blocked_content);
	generator.addSymbol(non_sequence);
	generator.addSymbol(non_node);

	// KV
	non_kv->rules.push_back(
		Rule(non_kv, {
			RuleElem(non_node  , RuleElem::ANY),
			RuleElem(term_colon, RuleElem::SAME),
			RuleElem(non_node  , RuleElem::BIGGER),
			}));

	// MAP
	non_map->rules.push_back(
		Rule(non_map, {
			RuleElem(term_curly_open        , RuleElem::ANY),
			RuleElem(non_map_blocked_content, RuleElem::ANY),
			RuleElem(term_curly_close       , RuleElem::ANY),
			}));
	non_map->rules.push_back(
		Rule(non_map, {
			RuleElem(term_curly_open , RuleElem::ANY),
			RuleElem(term_curly_close, RuleElem::ANY),
			}));
	non_map->rules.push_back(
		Rule(non_map, {
			RuleElem(non_kv , RuleElem::ANY),
			RuleElem(non_map, RuleElem::SAME),
			}));
	non_map->rules.push_back(
		Rule(non_map, {
			RuleElem(non_kv, RuleElem::ANY),
			}));

	// KV_FOR_BLOCKED_CONTENT
	non_kv_for_blocked_content->rules.push_back(
		Rule(non_kv_for_blocked_content, {
			RuleElem(non_node  , RuleElem::ANY),
			RuleElem(term_colon, RuleElem::ANY),
			RuleElem(non_node  , RuleElem::ANY),
			}));

	// MAP_BLOCKED_CONTENT
	non_map_blocked_content->rules.push_back(
		Rule(non_map_blocked_content, {
			RuleElem(non_kv_for_blocked_content, RuleElem::ANY),
			RuleElem(term_comma                , RuleElem::ANY),
			RuleElem(non_map_blocked_content   , RuleElem::ANY),
			}));
	non_map_blocked_content->rules.push_back(
		Rule(non_map_blocked_content, {
			RuleElem(non_kv_for_blocked_content, RuleElem::ANY),
			}));

	// SEQUENCE


	// NODE
	non_node->rules.push_back(
		Rule(non_node, { RuleElem(term_int    , RuleElem::ANY), }));
	non_node->rules.push_back(
		Rule(non_node, { RuleElem(term_double , RuleElem::ANY), }));
	non_node->rules.push_back(
		Rule(non_node, { RuleElem(term_string , RuleElem::ANY), }));
	non_node->rules.push_back(
		Rule(non_node, { RuleElem(non_map     , RuleElem::ANY), }));


	start->rules.push_back(
		Rule(start, { RuleElem(non_node, RuleElem::ANY) }));


	auto parsing_table = generator.generateTable();
}

int main () {
    _t_basic();
	_t_generateParsingTable();
	//_t_load_map_paws();
    //_t_load_boss_appear_snake_obj();
    return 0;
}
