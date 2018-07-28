
#include <assert.h>
#include <iostream>
#include <stdio.h>

#include "./src/paw_print.h"


using namespace paw_print;

using std::cout;
using std::endl;

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

void _t_load_simple_obj () {
    FILE* f = fopen("../paw/simple.obj", "r");
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    char* text = new char[size];
    rewind(f);
    fread(text, sizeof(char), size, f);

    PawPrint paw;
    paw.loadText(text);

    delete[] text;
}

void _t_load_boss_appear_snake_obj () {

    FILE* f = fopen("../paw/boss_appear_snake.obj", "r");
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    char* text = new char[size];
    rewind(f);
    fread(text, sizeof(char), size, f);


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

int main () {
    _t_basic();
    _t_load_simple_obj();
    _t_load_boss_appear_snake_obj();
    return 0;
}
