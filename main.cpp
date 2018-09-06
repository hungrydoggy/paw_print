
#include <assert.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>

#include <parse_table.h>

#include "./src/paw_print.h"
#include "./src/paw_print_loader.h"


using namespace paw_print;
using namespace parse_table;

using std::cout;
using std::endl;
using std::ifstream;
using std::stringstream;
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

	vector<unsigned char> correct_raw_data = { 7,9,4,4,0,97,98,99,0,7,9,4,2,0,97,0,2,1,0,0,0,9,4,2,0,98,0,2,2,0,0,0,9,4,2,0,99,0,2,3,0,0,0,9,4,2,0,100,0,5,4,2,0,105,0,4,2,0,106,0,4,2,0,107,0,2,5,0,0,0,2,6,0,0,0,2,7,0,0,0,6,9,4,2,0,101,0,7,9,4,2,0,120,0,3,0,0,0,0,0,0,34,64,9,4,2,0,121,0,3,0,0,0,0,0,0,32,64,9,4,2,0,122,0,3,0,0,0,0,0,0,28,64,8,9,4,2,0,102,0,5,7,9,4,6,0,102,105,114,115,116,0,7,9,4,5,0,110,97,109,101,0,4,6,0,102,105,114,115,116,0,9,4,6,0,118,97,108,117,101,0,2,1,0,0,0,8,8,7,9,4,7,0,115,101,99,111,110,100,0,7,9,4,5,0,110,97,109,101,0,4,7,0,115,101,99,111,110,100,0,9,4,6,0,118,97,108,117,101,0,2,2,0,0,0,8,8,6,8,8, };
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
	assert(root["xyz"].type() == PawPrint::Data::TYPE_NONE);
	assert(root["abc"].type() == 7);
	assert(root["abc"]["a"].type() == 2);
	assert(root["abc"]["a"].get(-1) == 1);
	assert(root["abc"]["b"].get(-1) == 2);
	assert(root["abc"]["c"].get(-1) == 3);
	assert(root["abc"]["d"].type() == 5);
	assert(root["abc"]["d"][0].get("") == "i");
	assert(root["abc"]["d"][1].get("") == "j");
	assert(root["abc"]["d"][2].get("") == "k");
	assert(root["abc"]["d"][3].get("") == "5");
	assert(root["abc"]["d"][3].get(-1) == 5);
	assert(root["abc"]["d"][4].get(-2) == 6);
	assert(root["abc"]["d"][5].get(-3) == 7);
	assert(root["abc"]["e"].type() == 7);
	assert(root["abc"]["e"]["x"].get(-1) == -1);
	assert(root["abc"]["e"]["x"].get(-1.0) == 9.0);
	assert(root["abc"]["e"]["y"].get(-1.0) == 8.0);
	assert(root["abc"]["e"]["z"].get(-1.0) == 7.0);
	assert(root["abc"]["f"].type() == PawPrint::Data::TYPE_SEQUENCE);
	assert(root["abc"]["f"][0].type() == PawPrint::Data::TYPE_MAP);
	assert(root["abc"]["f"][0]["first"].type() == PawPrint::Data::TYPE_MAP);
	assert(root["abc"]["f"][0]["first"]["name"].type() == PawPrint::Data::TYPE_STRING);
	assert(root["abc"]["f"][0]["first"]["value"].type() == PawPrint::Data::TYPE_INT);
	assert(root["abc"]["f"][0]["first"]["name"].get("") == "first");
	assert(root["abc"]["f"][0]["first"]["value"].get(-1) == 1);
	assert(root["abc"]["f"][1]["second"]["name"].get("") == "second");
	assert(root["abc"]["f"][1]["second"]["value"].get(-1) == 2);
}

static shared_ptr<PawPrint> _loadPaw(const string &path) {
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

	auto result = PawPrintLoader::loadText(text);

	delete[] text;

	return result;
}

void _t_load_map_paws() {
    string correct[6] = {
		"a :\n" \
		"  \"abc\"\n",
		"a :\n" \
		"  b :\n" \
		"    \"abc\"\n",
		"a :\n" \
		"  b :\n" \
		"    \"abc\"\n" \
		"  c :\n" \
		"    x :\n" \
		"      1.00000000\n" \
		"    y :\n" \
		"      2.00000000\n" \
		"    z :\n" \
		"      i :\n" \
		"        \"1\"\n" \
		"      j :\n" \
		"        \"2\"\n" \
		"      k :\n" \
		"        \"3\"\n",
		"i :\n" \
		"  \"1\"\n" \
		"j :\n" \
		"  \"2\"\n" \
		"k :\n" \
		"  \"3\"\n",
        "a :\n" \
        "  { }\n",
		"a :\n" \
		"  b :\n" \
		"    \"abc\"\n" \
		"  c :\n" \
		"    x :\n" \
		"      1.00000000\n" \
		"    y :\n" \
		"      2.00000000\n" \
		"    z :\n" \
		"      i :\n" \
		"        \"1\"\n" \
		"      j :\n" \
		"        \"2\"\n" \
		"      k :\n" \
		"        \"3\"\n" \
		"  d :\n" \
		"    13\n",
    };

	for (int pi = 0; pi <= 5; ++pi) {
	//for (int pi = 2; pi <= 2; ++pi) {
		cout << "load_map_paw : " << pi << endl;
#if _WINDOWS
		auto paw = _loadPaw("../../paw/map_0" + to_string(pi) + ".paw");
#else
		auto paw = _loadPaw("../paw/map_0" + to_string(pi) + ".paw");
#endif
		assert(paw != null);

        //cout << paw->root().toString() << endl;
        assert(paw->root().toString() == correct[pi]);
	}
}

static void _t_loadParsingTree () {
#if _WINDOWS
	ifstream f("../../paw_print.tab", std::ifstream::binary);
#else
	ifstream f("../paw_print.tab", std::ifstream::binary);
#endif

	// get length of file:
	f.seekg(0, f.end);
	int size = f.tellg();
	f.seekg(0, f.beg);
    
	// allocate memory:
    vector<unsigned char> binary;
    binary.resize(size);

	// read data as a block:
	f.read((char*)binary.data(), size);
	f.close();

    auto table = ParsingTable(binary);
    //cout << table.toString();
    auto table_correct = 
        "##### Rules\n" \
        "# Rule 0 : S' -> S \n" \
        "# Rule 1 : S -> NODE \n" \
        "# Rule 2 : KEY -> bool \n" \
        "# Rule 3 : KEY -> int \n" \
        "# Rule 4 : KEY -> double \n" \
        "# Rule 5 : KEY -> string \n" \
        "# Rule 6 : KV -> KEY colon NODE \n" \
        "# Rule 7 : KV -> KEY colon #new_line #indent NODE #dedent \n" \
        "# Rule 8 : KV -> KEY colon #new_line \n" \
        "# Rule 9 : MAP -> KV MAP \n" \
        "# Rule 10 : MAP -> KV \n" \
        "# Rule 11 : CURL_MAP -> curly_open curly_close \n" \
        "# Rule 12 : CURL_MAP -> curly_open #new_line curly_close \n" \
        "# Rule 13 : CURL_MAP -> curly_open MAP_BLOCKED curly_close \n" \
        "# Rule 14 : CURL_MAP -> curly_open #new_line #indent MAP_BLOCKED #dedent curly_close \n" \
        "# Rule 15 : KV_BLOCKED -> KEY colon NODE \n" \
        "# Rule 16 : KV_BLOCKED -> KEY colon \n" \
        "# Rule 17 : MAP_BLOCKED -> KV_BLOCKED \n" \
        "# Rule 18 : MAP_BLOCKED -> KV_BLOCKED comma MAP_BLOCKED \n" \
        "# Rule 19 : MAP_BLOCKED -> KV_BLOCKED comma #new_line MAP_BLOCKED \n" \
        "# Rule 20 : SEQ_ELEM -> dash NODE \n" \
        "# Rule 21 : SEQ_ELEM -> dash #new_line #indent NODE #dedent \n" \
        "# Rule 22 : SEQUENCE -> SEQ_ELEM SEQUENCE \n" \
        "# Rule 23 : SEQUENCE -> SEQ_ELEM \n" \
        "# Rule 24 : SQUARE_SEQ -> square_open square_close \n" \
        "# Rule 25 : SQUARE_SEQ -> square_open SEQ_BLOCKED square_close \n" \
        "# Rule 26 : SQUARE_SEQ -> square_open #new_line square_close \n" \
        "# Rule 27 : SQUARE_SEQ -> square_open #new_line #indent SEQ_BLOCKED #dedent square_close \n" \
        "# Rule 28 : SEQ_BLOCKED -> NODE comma SEQ_BLOCKED \n" \
        "# Rule 29 : SEQ_BLOCKED -> NODE comma #new_line SEQ_BLOCKED \n" \
        "# Rule 30 : SEQ_BLOCKED -> NODE \n" \
        "# Rule 31 : NODE -> bool \n" \
        "# Rule 32 : NODE -> bool #new_line \n" \
        "# Rule 33 : NODE -> int \n" \
        "# Rule 34 : NODE -> int #new_line \n" \
        "# Rule 35 : NODE -> double \n" \
        "# Rule 36 : NODE -> double #new_line \n" \
        "# Rule 37 : NODE -> string \n" \
        "# Rule 38 : NODE -> string #new_line \n" \
        "# Rule 39 : NODE -> MAP \n" \
        "# Rule 40 : NODE -> CURL_MAP \n" \
        "# Rule 41 : NODE -> CURL_MAP #new_line \n" \
        "# Rule 42 : NODE -> SEQUENCE \n" \
        "# Rule 43 : NODE -> SQUARE_SEQ \n" \
        "# Rule 44 : NODE -> SQUARE_SEQ #new_line \n" \
        "##### Table\n" \
        "    |   $ | #dedent | #indent | #new_line | bool | colon | comma | curly_close | curly_open | dash | double | int | square_close | square_open | string | CURL_MAP |  KEY |   KV | KV_BLOCKED |  MAP | MAP_BLOCKED | NODE |   S | SEQUENCE | SEQ_BLOCKED | SEQ_ELEM | SQUARE_SEQ | \n" \
        "-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n" \
        "  0 |     |         |         |           |   s5 |       |       |             |         s3 |   s6 |    s15 |  s4 |              |          s8 |    s16 |     go14 |  go1 | go10 |            | go12 |             |  go2 | go7 |      go9 |             |     go11 |       go13 | \n" \
        "  1 |     |         |         |           |      |   s17 |       |             |            |      |        |     |              |             |        |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        "  2 |  r1 |         |         |           |      |       |       |             |            |      |        |     |              |             |        |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        "  3 |     |         |         |       s20 |  s21 |       |       |         s22 |            |      |    s25 | s19 |              |             |    s26 |          | go18 |      |       go23 |      |        go24 |      |     |          |             |          |            | \n" \
        "  4 | r33 |     r33 |         |       s27 |  r33 |    r3 |   r33 |         r33 |            |  r33 |    r33 | r33 |          r33 |             |    r33 |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        "  5 | r31 |     r31 |         |       s28 |  r31 |    r2 |   r31 |         r31 |            |  r31 |    r31 | r31 |          r31 |             |    r31 |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        "  6 |     |         |         |       s30 |   s5 |       |       |             |         s3 |   s6 |    s15 |  s4 |              |          s8 |    s16 |     go14 |  go1 | go10 |            | go12 |             | go29 |     |      go9 |             |     go11 |       go13 | \n" \
        "  7 | acc |         |         |           |      |       |       |             |            |      |        |     |              |             |        |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        "  8 |     |         |         |       s32 |   s5 |       |       |             |         s3 |   s6 |    s15 |  s4 |          s33 |          s8 |    s16 |     go14 |  go1 | go10 |            | go12 |             | go31 |     |      go9 |        go34 |     go11 |       go13 | \n" \
        "  9 | r42 |     r42 |         |           |  r42 |       |   r42 |         r42 |            |  r42 |    r42 | r42 |          r42 |             |    r42 |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 10 | r10 |     r10 |         |           |  s21 |       |   r10 |         r10 |            |  r10 |    s25 | s19 |          r10 |             |    s26 |          |  go1 | go10 |            | go35 |             |      |     |          |             |          |            | \n" \
        " 11 | r23 |     r23 |         |           |  r23 |       |   r23 |         r23 |            |   s6 |    r23 | r23 |          r23 |             |    r23 |          |      |      |            |      |             |      |     |     go36 |             |     go11 |            | \n" \
        " 12 | r39 |     r39 |         |           |  r39 |       |   r39 |         r39 |            |  r39 |    r39 | r39 |          r39 |             |    r39 |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 13 | r43 |     r43 |         |       s37 |  r43 |       |   r43 |         r43 |            |  r43 |    r43 | r43 |          r43 |             |    r43 |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 14 | r40 |     r40 |         |       s38 |  r40 |       |   r40 |         r40 |            |  r40 |    r40 | r40 |          r40 |             |    r40 |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 15 | r35 |     r35 |         |       s39 |  r35 |    r4 |   r35 |         r35 |            |  r35 |    r35 | r35 |          r35 |             |    r35 |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 16 | r37 |     r37 |         |       s40 |  r37 |    r5 |   r37 |         r37 |            |  r37 |    r37 | r37 |          r37 |             |    r37 |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 17 |     |         |         |       s42 |   s5 |       |       |             |         s3 |   s6 |    s15 |  s4 |              |          s8 |    s16 |     go14 |  go1 | go10 |            | go12 |             | go41 |     |      go9 |             |     go11 |       go13 | \n" \
        " 18 |     |         |         |           |      |   s43 |       |             |            |      |        |     |              |             |        |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 19 |     |         |         |           |      |    r3 |       |             |            |      |        |     |              |             |        |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 20 |     |         |     s44 |           |      |       |       |         s45 |            |      |        |     |              |             |        |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 21 |     |         |         |           |      |    r2 |       |             |            |      |        |     |              |             |        |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 22 | r11 |     r11 |         |       r11 |  r11 |       |   r11 |         r11 |            |  r11 |    r11 | r11 |          r11 |             |    r11 |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 23 |     |     r17 |         |           |      |       |   s46 |         r17 |            |      |        |     |              |             |        |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 24 |     |         |         |           |      |       |       |         s47 |            |      |        |     |              |             |        |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 25 |     |         |         |           |      |    r4 |       |             |            |      |        |     |              |             |        |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 26 |     |         |         |           |      |    r5 |       |             |            |      |        |     |              |             |        |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 27 | r34 |     r34 |         |           |  r34 |       |   r34 |         r34 |            |  r34 |    r34 | r34 |          r34 |             |    r34 |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 28 | r32 |     r32 |         |           |  r32 |       |   r32 |         r32 |            |  r32 |    r32 | r32 |          r32 |             |    r32 |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 29 | r20 |     r20 |         |           |  r20 |       |   r20 |         r20 |            |  r20 |    r20 | r20 |          r20 |             |    r20 |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 30 |     |         |     s48 |           |      |       |       |             |            |      |        |     |              |             |        |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 31 |     |     r30 |         |           |      |       |   s49 |             |            |      |        |     |          r30 |             |        |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 32 |     |         |     s50 |           |      |       |       |             |            |      |        |     |          s51 |             |        |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 33 | r24 |     r24 |         |       r24 |  r24 |       |   r24 |         r24 |            |  r24 |    r24 | r24 |          r24 |             |    r24 |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 34 |     |         |         |           |      |       |       |             |            |      |        |     |          s52 |             |        |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 35 |  r9 |      r9 |         |           |   r9 |       |    r9 |          r9 |            |   r9 |     r9 |  r9 |           r9 |             |     r9 |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 36 | r22 |     r22 |         |           |  r22 |       |   r22 |         r22 |            |  r22 |    r22 | r22 |          r22 |             |    r22 |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 37 | r44 |     r44 |         |           |  r44 |       |   r44 |         r44 |            |  r44 |    r44 | r44 |          r44 |             |    r44 |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 38 | r41 |     r41 |         |           |  r41 |       |   r41 |         r41 |            |  r41 |    r41 | r41 |          r41 |             |    r41 |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 39 | r36 |     r36 |         |           |  r36 |       |   r36 |         r36 |            |  r36 |    r36 | r36 |          r36 |             |    r36 |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 40 | r38 |     r38 |         |           |  r38 |       |   r38 |         r38 |            |  r38 |    r38 | r38 |          r38 |             |    r38 |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 41 |  r6 |      r6 |         |           |   r6 |       |    r6 |          r6 |            |   r6 |     r6 |  r6 |           r6 |             |     r6 |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 42 |  r8 |      r8 |     s53 |           |   r8 |       |    r8 |          r8 |            |   r8 |     r8 |  r8 |           r8 |             |     r8 |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 43 |     |     r16 |         |           |   s5 |       |   r16 |         r16 |         s3 |   s6 |    s15 |  s4 |              |          s8 |    s16 |     go14 |  go1 | go10 |            | go12 |             | go54 |     |      go9 |             |     go11 |       go13 | \n" \
        " 44 |     |         |         |           |  s21 |       |       |             |            |      |    s25 | s19 |              |             |    s26 |          | go18 |      |       go23 |      |        go55 |      |     |          |             |          |            | \n" \
        " 45 | r12 |     r12 |         |       r12 |  r12 |       |   r12 |         r12 |            |  r12 |    r12 | r12 |          r12 |             |    r12 |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 46 |     |         |         |       s56 |  s21 |       |       |             |            |      |    s25 | s19 |              |             |    s26 |          | go18 |      |       go23 |      |        go57 |      |     |          |             |          |            | \n" \
        " 47 | r13 |     r13 |         |       r13 |  r13 |       |   r13 |         r13 |            |  r13 |    r13 | r13 |          r13 |             |    r13 |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 48 |     |         |         |           |   s5 |       |       |             |         s3 |   s6 |    s15 |  s4 |              |          s8 |    s16 |     go14 |  go1 | go10 |            | go12 |             | go58 |     |      go9 |             |     go11 |       go13 | \n" \
        " 49 |     |         |         |       s59 |   s5 |       |       |             |         s3 |   s6 |    s15 |  s4 |              |          s8 |    s16 |     go14 |  go1 | go10 |            | go12 |             | go31 |     |      go9 |        go60 |     go11 |       go13 | \n" \
        " 50 |     |         |         |           |   s5 |       |       |             |         s3 |   s6 |    s15 |  s4 |              |          s8 |    s16 |     go14 |  go1 | go10 |            | go12 |             | go31 |     |      go9 |        go61 |     go11 |       go13 | \n" \
        " 51 | r26 |     r26 |         |       r26 |  r26 |       |   r26 |         r26 |            |  r26 |    r26 | r26 |          r26 |             |    r26 |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 52 | r25 |     r25 |         |       r25 |  r25 |       |   r25 |         r25 |            |  r25 |    r25 | r25 |          r25 |             |    r25 |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 53 |     |         |         |           |   s5 |       |       |             |         s3 |   s6 |    s15 |  s4 |              |          s8 |    s16 |     go14 |  go1 | go10 |            | go12 |             | go62 |     |      go9 |             |     go11 |       go13 | \n" \
        " 54 |     |     r15 |         |           |      |       |   r15 |         r15 |            |      |        |     |              |             |        |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 55 |     |     s63 |         |           |      |       |       |             |            |      |        |     |              |             |        |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 56 |     |         |         |           |  s21 |       |       |             |            |      |    s25 | s19 |              |             |    s26 |          | go18 |      |       go23 |      |        go64 |      |     |          |             |          |            | \n" \
        " 57 |     |     r18 |         |           |      |       |       |         r18 |            |      |        |     |              |             |        |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 58 |     |     s65 |         |           |      |       |       |             |            |      |        |     |              |             |        |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 59 |     |         |         |           |   s5 |       |       |             |         s3 |   s6 |    s15 |  s4 |              |          s8 |    s16 |     go14 |  go1 | go10 |            | go12 |             | go31 |     |      go9 |        go66 |     go11 |       go13 | \n" \
        " 60 |     |     r28 |         |           |      |       |       |             |            |      |        |     |          r28 |             |        |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 61 |     |     s67 |         |           |      |       |       |             |            |      |        |     |              |             |        |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 62 |     |     s68 |         |           |      |       |       |             |            |      |        |     |              |             |        |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 63 |     |         |         |           |      |       |       |         s69 |            |      |        |     |              |             |        |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 64 |     |     r19 |         |           |      |       |       |         r19 |            |      |        |     |              |             |        |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 65 | r21 |     r21 |         |           |  r21 |       |   r21 |         r21 |            |  r21 |    r21 | r21 |          r21 |             |    r21 |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 66 |     |     r29 |         |           |      |       |       |             |            |      |        |     |          r29 |             |        |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 67 |     |         |         |           |      |       |       |             |            |      |        |     |          s70 |             |        |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 68 |  r7 |      r7 |         |           |   r7 |       |    r7 |          r7 |            |   r7 |     r7 |  r7 |           r7 |             |     r7 |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 69 | r14 |     r14 |         |       r14 |  r14 |       |   r14 |         r14 |            |  r14 |    r14 | r14 |          r14 |             |    r14 |          |      |      |            |      |             |      |     |          |             |          |            | \n" \
        " 70 | r27 |     r27 |         |       r27 |  r27 |       |   r27 |         r27 |            |  r27 |    r27 | r27 |          r27 |             |    r27 |          |      |      |            |      |             |      |     |          |             |          |            | \n";
    assert(table.toString() == table_correct);

}

static void _t_load_boss_appear_snake () {
    cout << "load_boss_appear_snake.obj" << endl;
#if _WINDOWS
    auto paw = _loadPaw("../../paw/boss_appear_snake.obj");
#else
    auto paw = _loadPaw("../paw/boss_appear_snake.obj");
#endif
    assert(paw != null);

    auto correct =
        "@objects/ui/popup/boss_appear.obj :\n" \
        "  ref :\n" \
        "    boss_img :\n" \
        "      \"images/ui/popup/snake_art.png\"\n" \
        "    icon_img :\n" \
        "      \"images/status/icons/inven_ico_snake_poison.png\"\n" \
        "    icon_siz :\n" \
        "      - 79\n" \
        "      - 0\n" \
        "    icon_pos :\n" \
        "      - 0\n" \
        "      - 10.50000000\n" \
        "      - 1\n";
    assert(paw->root().toString() == correct);
}

static void _t_load_green () {
    cout << "load green.obj" << endl;
#if _WINDOWS
    auto paw = _loadPaw("../../paw/green.obj");
#else
    auto paw = _loadPaw("../paw/green.obj");
#endif
    assert(paw != null);

    auto correct =
        "Object :\n" \
        "  cls :\n" \
        "    \"Button\"\n" \
        "  nam :\n" \
        "    \"button\"\n" \
        "  col :\n" \
        "    $button_col :\n" \
        "      - 1\n" \
        "      - 1\n" \
        "      - 1\n" \
        "      - 1\n" \
        "  chl :\n" \
        "    - Object :\n" \
        "        dra :\n" \
        "          false\n" \
        "        nam :\n" \
        "          \"bg_gray\"\n" \
        "        vie :\n" \
        "          NinePatch :\n" \
        "            img :\n" \
        "              \"images/ui/button/gray.png\"\n" \
        "            lrt :\n" \
        "              - 10\n" \
        "              - 10\n" \
        "              - 10\n" \
        "              - 10\n" \
        "            siz :\n" \
        "              - $width :\n" \
        "                  400\n" \
        "              - $height :\n" \
        "                  80\n" \
        "    - Object :\n" \
        "        nam :\n" \
        "          \"bg\"\n" \
        "        vie :\n" \
        "          NinePatch :\n" \
        "            img :\n" \
        "              \"images/ui/button/green.png\"\n" \
        "            lrt :\n" \
        "              - 10\n" \
        "              - 10\n" \
        "              - 10\n" \
        "              - 10\n" \
        "            siz :\n" \
        "              - $width :\n" \
        "                  400\n" \
        "              - $height :\n" \
        "                  80\n" \
        "        bdy :\n" \
        "          BulletBody :\n" \
        "            mas :\n" \
        "              0\n" \
        "        shp :\n" \
        "          - BulletShape :\n" \
        "              cls :\n" \
        "                \"Box\"\n" \
        "              hsz :\n" \
        "                - !divide :\n" \
        "                    - $width :\n" \
        "                        400\n" \
        "                    - 2\n" \
        "                - !divide :\n" \
        "                    - $height :\n" \
        "                        80\n" \
        "                    - 2\n" \
        "                - 0\n" \
        "        pda :\n" \
        "          ActionCall :\n" \
        "            roo :\n" \
        "              \"..\"\n" \
        "            act :\n" \
        "              SendMessage :\n" \
        "                dst :\n" \
        "                  $pda_dst :\n" \
        "                    \"/\"\n" \
        "                nam :\n" \
        "                  $pda_msg :\n" \
        "                    \"onButtonClicked\"\n" \
        "                par :\n" \
        "                  $pda_par :\n" \
        "                    [ ]\n" \
        "    - Object :\n" \
        "        nam :\n" \
        "          \"text\"\n" \
        "        pos :\n" \
        "          $text_pos :\n" \
        "            - 0\n" \
        "            - 0\n" \
        "            - 1\n" \
        "        scl :\n" \
        "          $text_scl :\n" \
        "            - 0.70000000\n" \
        "            - 0.70000000\n" \
        "            - 1\n" \
        "        vie :\n" \
        "          BitmapFontView :\n" \
        "            bmf :\n" \
        "              $font :\n" \
        "                \"bitmap_fonts/nanum_myeongjo_extra_bold.67.bmf\"\n" \
        "            txt :\n" \
        "              $text :\n" \
        "                \"confirm\"\n" \
        "            hor :\n" \
        "              \"middle\"\n" \
        "            ver :\n" \
        "              \"middle\"\n";
    assert(paw->root().toString() == correct);
}

static void _t_load_obj_shell () {
    cout << "load obj.shell" << endl;
#if _WINDOWS
    auto paw = _loadPaw("../../paw/obj.shell");
#else
    auto paw = _loadPaw("../paw/obj.shell");
#endif
    assert(paw != null);

    auto correct =
        "Scene :\n" \
        "  phy :\n" \
        "    BulletPhysics :\n" \
        "      null\n" \
        "  chl :\n" \
        "    - Camera :\n" \
        "        nam :\n" \
        "          \"camera\"\n" \
        "        pos :\n" \
        "          - 0\n" \
        "          - 0\n" \
        "          - 1000\n" \
        "        prj :\n" \
        "          Projection :\n" \
        "            typ :\n" \
        "              \"orthogonal\"\n" \
        "    - \"$this\"\n";
    assert(paw->root().toString() == correct);
}

static void _t_load_dialogue () {
    cout << "load dialogue" << endl;
#if _WINDOWS
    auto paw = _loadPaw("../../paw/dialogue.obj");
#else
    auto paw = _loadPaw("../paw/dialogue.obj");
#endif
    assert(paw != null);
    //cout << paw->root().toString();
}

static void _t_load_chance () {
    cout << "load chance.shell" << endl;
#if _WINDOWS
    auto paw = _loadPaw("../../paw/chance.act");
#else
    auto paw = _loadPaw("../paw/chance.act");
#endif
    assert(paw != null);

    auto correct =
		"Sequence :\n" \
		"  ref :\n" \
		"    dst_pos :\n" \
		"      !pos_from :\n" \
		"        roo :\n" \
		"          \"..\"\n" \
		"        tar :\n" \
		"          \"/camera\"\n" \
		"        pos :\n" \
		"          - 0\n" \
		"          - 0\n" \
		"          - -300\n" \
		"  chl :\n" \
		"    - SaveInitialInfo :\n" \
		"        null\n" \
		"    - SaveInitialInfo :\n" \
		"        tar :\n" \
		"          \"back_top_text\"\n" \
		"    - SetInputProcessable :\n" \
		"        false\n" \
		"    - SetDrawByTreeOrder :\n" \
		"        true\n" \
		"    - SetDrawSortGroup :\n" \
		"        20001\n" \
		"    - RemoveAction :\n" \
		"        nam :\n" \
		"          \"blink_back_top_text\"\n" \
		"    - RemoveAction :\n" \
		"        nam :\n" \
		"          \"blink_front_top_text\"\n" \
		"    - SetDrawable :\n" \
		"        tar :\n" \
		"          \"front_top_text\"\n" \
		"        val :\n" \
		"          false\n" \
		"    - MoveZTo :\n" \
		"        dur :\n" \
		"          0\n" \
		"        val :\n" \
		"          !list_get :\n" \
		"            val :\n" \
		"              \"$dst_pos\"\n" \
		"            idx :\n" \
		"              2\n" \
		"    - SetDrawable :\n" \
		"        val :\n" \
		"          true\n" \
		"    - Parallel :\n" \
		"        - MoveTo :\n" \
		"            dur :\n" \
		"              0.50000000\n" \
		"            val :\n" \
		"              \"$dst_pos\"\n" \
		"            int :\n" \
		"              \"InOutQuad\"\n" \
		"        - RotateTo :\n" \
		"            dur :\n" \
		"              0.50000000\n" \
		"            val :\n" \
		"              - 0\n" \
		"              - 180\n" \
		"              - 0\n" \
		"            int :\n" \
		"              \"InOutQuad\"\n" \
		"        - ScaleTo :\n" \
		"            dur :\n" \
		"              0.50000000\n" \
		"            val :\n" \
		"              - 0.60000000\n" \
		"              - 0.60000000\n" \
		"              - 1\n" \
		"            int :\n" \
		"              \"InOutQuad\"\n" \
		"        - Sequence :\n" \
		"            - Wait :\n" \
		"                0.20000000\n" \
		"            - SendMessage :\n" \
		"                dst :\n" \
		"                  \"/\"\n" \
		"                nam :\n" \
		"                  \"showPopupBG\"\n" \
		"    - AddAction :\n" \
		"        nam :\n" \
		"          \"blink_back_top_text\"\n" \
		"        thr :\n" \
		"          true\n" \
		"    - SetInputProcessable :\n" \
		"        true\n" \
		"    - Pause :\n" \
		"        tar :\n" \
		"          \"/\"\n" \
		"        nam :\n" \
		"          \"openChanceCard\"\n" \
		"    - SetInputProcessable :\n" \
		"        false\n" \
		"    - SetDrawable :\n" \
		"        tar :\n" \
		"          \"back_top_text\"\n" \
		"        val :\n" \
		"          false\n" \
		"    - Parallel :\n" \
		"        - Sequence :\n" \
		"            - RotateTo :\n" \
		"                dur :\n" \
		"                  0.20000000\n" \
		"                val :\n" \
		"                  - 0\n" \
		"                  - 90\n" \
		"                  - 0\n" \
		"                int :\n" \
		"                  \"InQuad\"\n" \
		"            - RotateTo :\n" \
		"                dur :\n" \
		"                  0.20000000\n" \
		"                val :\n" \
		"                  - 0\n" \
		"                  - 0\n" \
		"                  - 0\n" \
		"                int :\n" \
		"                  \"OutQuad\"\n" \
		"        - ScaleTo :\n" \
		"            dur :\n" \
		"              0.40000000\n" \
		"            val :\n" \
		"              - 1\n" \
		"              - 1\n" \
		"              - 1\n" \
		"            int :\n" \
		"              \"InOutQuad\"\n" \
		"    - Wait :\n" \
		"        0.10000000\n" \
		"    - SetInputProcessable :\n" \
		"        true\n" \
		"    - Pause :\n" \
		"        tar :\n" \
		"          \"/\"\n" \
		"        nam :\n" \
		"          \"closeChanceCard\"\n" \
		"    - SetInputProcessable :\n" \
		"        false\n" \
		"    - SendMessage :\n" \
		"        dst :\n" \
		"          \"/\"\n" \
		"        nam :\n" \
		"          \"hidePopupBG\"\n" \
		"    - \"$close_action\"\n" \
		"    - LoadInitialInfo :\n" \
		"        null\n" \
		"    - LoadInitialInfo :\n" \
		"        tar :\n" \
		"          \"back_top_text\"\n" \
		"    - SetInputProcessable :\n" \
		"        $input_processable_on_end :\n" \
		"          true\n" \
		"    - SendMessage :\n" \
		"        dst :\n" \
		"          \"/\"\n" \
		"        nam :\n" \
		"          \"endOfChanceCard\"\n";
    assert(paw->root().toString() == correct);
}

static void _t_load_settings () {
    cout << "load settings.app" << endl;
#if _WINDOWS
    auto paw = _loadPaw("../../paw/settings.app");
#else
    auto paw = _loadPaw("../paw/settings.app");
#endif
    assert(paw != null);

    auto correct =
		"App :\n" \
		"  nam :\n" \
		"    \"test_proj\"\n" \
		"  prj :\n" \
		"    \"TestProject\"\n" \
		"  wid :\n" \
		"    432\n" \
		"  hei :\n" \
		"    768\n" \
		"  szf :\n" \
		"    android :\n" \
		"      - 1024\n" \
		"      - 1920\n";
    assert(paw->root().toString() == correct);
}

static void _t_load_open_icon_card_close_part () {
    cout << "load open_icon_card_close_part.obj" << endl;
#if _WINDOWS
    auto paw = _loadPaw("../../paw/open_icon_card_close_part.act");
#else
    auto paw = _loadPaw("../paw/open_icon_card_close_part.act");
#endif
    assert(paw != null);

    auto correct =
		"Sequence :\n" \
		"  - SendMessage :\n" \
		"      nam :\n" \
		"        \"hidePopupBG\"\n" \
		"      dst :\n" \
		"        \"/\"\n" \
		"  - Parallel :\n" \
		"      - MoveXTo :\n" \
		"          dur :\n" \
		"            0.50000000\n" \
		"          val :\n" \
		"            !list_get :\n" \
		"              val :\n" \
		"                \"$self.pos\"\n" \
		"              idx :\n" \
		"                0\n" \
		"          int :\n" \
		"            \"OutQuad\"\n" \
		"      - MoveYTo :\n" \
		"          dur :\n" \
		"            0.50000000\n" \
		"          val :\n" \
		"            !list_get :\n" \
		"              val :\n" \
		"                \"$self.pos\"\n" \
		"              idx :\n" \
		"                1\n" \
		"          int :\n" \
		"            \"OutQuad\"\n" \
		"      - Sequence :\n" \
		"          - Parallel :\n" \
		"              - Sequence :\n" \
		"                  - ScaleTo :\n" \
		"                      dur :\n" \
		"                        0.50000000\n" \
		"                      val :\n" \
		"                        - 1\n" \
		"                        - 1\n" \
		"                        - 1\n" \
		"                      int :\n" \
		"                        \"OutQuad\"\n" \
		"              - Sequence :\n" \
		"                  - RotateBy :\n" \
		"                      dur :\n" \
		"                        0.12000000\n" \
		"                      val :\n" \
		"                        - 0\n" \
		"                        - -90\n" \
		"                        - 0\n" \
		"                  - SetDrawable :\n" \
		"                      tar :\n" \
		"                        \"card\"\n" \
		"                      val :\n" \
		"                        false\n" \
		"                  - SetDrawable :\n" \
		"                      tar :\n" \
		"                        \"wrapper\"\n" \
		"                      val :\n" \
		"                        true\n" \
		"                  - RotateBy :\n" \
		"                      dur :\n" \
		"                        0.12000000\n" \
		"                      val :\n" \
		"                        - 0\n" \
		"                        - -90\n" \
		"                        - 0\n" \
		"                  - Repeat :\n" \
		"                      cnt :\n" \
		"                        1\n" \
		"                      act :\n" \
		"                        Sequence :\n" \
		"                          - RotateBy :\n" \
		"                              dur :\n" \
		"                                0.12000000\n" \
		"                              val :\n" \
		"                                - 0\n" \
		"                                - -90\n" \
		"                                - 0\n" \
		"                          - RotateBy :\n" \
		"                              dur :\n" \
		"                                0.12000000\n" \
		"                              val :\n" \
		"                                - 0\n" \
		"                                - -90\n" \
		"                                - 0\n";
    assert(paw->root().toString() == correct);
}

static void _t_reference_simple () {
    cout << "_t_reference_simple" << endl;
#if _WINDOWS
    auto settings = _loadPaw("../../paw/settings.app");
#else
    auto settings = _loadPaw("../paw/settings.app");
#endif
    assert(settings != null);
    
#if _WINDOWS
    auto obj_shell = _loadPaw("../../paw/obj.shell");
#else
    auto obj_shell = _loadPaw("../paw/obj.shell");
#endif
    assert(obj_shell != null);

#if _WINDOWS
    auto snake = _loadPaw("../../paw/boss_appear_snake.obj");
#else
    auto snake = _loadPaw("../paw/boss_appear_snake.obj");
#endif
    assert(snake != null);


    PawPrint paw;
    paw.beginMap();
        paw.pushKey("ReferenceTest");
        paw.beginMap();
            paw.pushKey("boss");
            paw.beginMap();
                paw.pushKey("appear");
                paw.beginMap();
                    paw.pushKey("snake");
                    paw.pushReference(snake->root());
                paw.endMap();
            paw.endMap();

            paw.pushKey("settings");
            paw.pushReference(settings->root());

            paw.pushKey("obj");
            paw.beginMap();
                paw.pushKey("shell");
                paw.pushReference(obj_shell->root());
            paw.endMap();
        paw.endMap();
    paw.endMap();

	auto cursor = paw.root();
	assert(cursor.size() == 1);
	assert(cursor["ReferenceTest"].size() == 3);
	assert(cursor["ReferenceTest"]["boss"]["appear"]["snake"].toString() == snake->root().toString());
	assert(cursor["ReferenceTest"]["settings"].toString() == settings->root().toString());
	assert(cursor["ReferenceTest"]["obj"]["shell"].toString() == obj_shell->root().toString());

    auto correct =
		"ReferenceTest :\n" \
		"  boss :\n" \
		"    appear :\n" \
		"      snake :\n" \
		"        @objects/ui/popup/boss_appear.obj :\n" \
		"          ref :\n" \
		"            boss_img :\n" \
		"              \"images/ui/popup/snake_art.png\"\n" \
		"            icon_img :\n" \
		"              \"images/status/icons/inven_ico_snake_poison.png\"\n" \
		"            icon_siz :\n" \
		"              - 79\n" \
		"              - 0\n" \
		"            icon_pos :\n" \
		"              - 0\n" \
		"              - 10.50000000\n" \
		"              - 1\n" \
		"  settings :\n" \
		"    App :\n" \
		"      nam :\n" \
		"        \"test_proj\"\n" \
		"      prj :\n" \
		"        \"TestProject\"\n" \
		"      wid :\n" \
		"        432\n" \
		"      hei :\n" \
		"        768\n" \
		"      szf :\n" \
		"        android :\n" \
		"          - 1024\n" \
		"          - 1920\n" \
		"  obj :\n" \
		"    shell :\n" \
		"      Scene :\n" \
		"        phy :\n" \
		"          BulletPhysics :\n" \
		"            null\n" \
		"        chl :\n" \
		"          - Camera :\n" \
		"              nam :\n" \
		"                \"camera\"\n" \
		"              pos :\n" \
		"                - 0\n" \
		"                - 0\n" \
		"                - 1000\n" \
		"              prj :\n" \
		"                Projection :\n" \
		"                  typ :\n" \
		"                    \"orthogonal\"\n" \
		"          - \"$this\"\n";
    assert(paw.root().toString() == correct);
}

int main () {
    _t_basic();
    _t_loadParsingTree();
	_t_load_map_paws();
	_t_load_boss_appear_snake();
	_t_load_green();
	_t_load_obj_shell();
	_t_load_dialogue();
	_t_load_chance();
	_t_load_settings();
	_t_load_open_icon_card_close_part();
    _t_reference_simple();
    return 0;
}
