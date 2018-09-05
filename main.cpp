
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

        cout << paw->root().toString() << endl;
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
        "# Rule 6 : KV -> KEY colon NODE #new_line \n" \
        "# Rule 7 : KV -> KEY colon #new_line #indent NODE #new_line #dedent \n" \
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
        "# Rule 21 : SEQ_ELEM -> dash #new_line #indent NODE #new_line #dedent \n" \
        "# Rule 22 : SEQUENCE -> SEQ_ELEM SEQUENCE \n" \
        "# Rule 23 : SEQUENCE -> SEQ_ELEM \n" \
        "# Rule 24 : SQUARE_SEQ -> square_open square_close \n" \
        "# Rule 25 : SQUARE_SEQ -> square_open SEQ_BLOCKED square_close \n" \
        "# Rule 26 : SQUARE_SEQ -> square_open #new_line square_close \n" \
        "# Rule 27 : SQUARE_SEQ -> square_open #new_line #indent SEQ_BLOCKED #dedent #new_line square_close \n" \
        "# Rule 28 : SEQ_BLOCKED -> NODE comma SEQ_BLOCKED \n" \
        "# Rule 29 : SEQ_BLOCKED -> NODE comma #new_line SEQ_BLOCKED \n" \
        "# Rule 30 : SEQ_BLOCKED -> NODE \n" \
        "# Rule 31 : NODE -> bool \n" \
        "# Rule 32 : NODE -> int \n" \
        "# Rule 33 : NODE -> double \n" \
        "# Rule 34 : NODE -> string \n" \
        "# Rule 35 : NODE -> MAP \n" \
        "# Rule 36 : NODE -> CURL_MAP \n" \
        "# Rule 37 : NODE -> SEQUENCE \n" \
        "# Rule 38 : NODE -> SQUARE_SEQ \n" \
        "##### Table\n" \
        "    |   $ | #dedent | #indent | #new_line | bool | colon | comma | curly_close | curly_open | dash | double | int | square_close | square_open | string | CURL_MAP |  KEY |  KV | KV_BLOCKED |  MAP | MAP_BLOCKED | NODE |    S | SEQUENCE | SEQ_BLOCKED | SEQ_ELEM | SQUARE_SEQ | \n" \
        "-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n" \
        "  0 |     |         |         |           |   s5 |       |       |             |        s14 |  s13 |     s3 |  s4 |              |          s9 |     s6 |     go15 |  go1 | go2 |            |  go8 |             | go16 | go10 |     go12 |             |     go11 |        go7 | \n" \
        "  1 |     |         |         |           |      |   s17 |       |             |            |      |        |     |              |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        "  2 | r10 |     r10 |         |       r10 |  s20 |       |   r10 |         r10 |            |  r10 |    s18 | s19 |          r10 |             |    s21 |          |  go1 | go2 |            | go22 |             |      |      |          |             |          |            | \n" \
        "  3 | r33 |     r33 |         |       r33 |      |    r4 |   r33 |         r33 |            |  r33 |        |     |          r33 |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        "  4 | r32 |     r32 |         |       r32 |      |    r3 |   r32 |         r32 |            |  r32 |        |     |          r32 |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        "  5 | r31 |     r31 |         |       r31 |      |    r2 |   r31 |         r31 |            |  r31 |        |     |          r31 |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        "  6 | r34 |     r34 |         |       r34 |      |    r5 |   r34 |         r34 |            |  r34 |        |     |          r34 |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        "  7 | r38 |     r38 |         |       r38 |      |       |   r38 |         r38 |            |  r38 |        |     |          r38 |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        "  8 | r35 |     r35 |         |       r35 |      |       |   r35 |         r35 |            |  r35 |        |     |          r35 |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        "  9 |     |         |         |       s24 |   s5 |       |       |             |        s14 |  s13 |     s3 |  s4 |          s23 |          s9 |     s6 |     go15 |  go1 | go2 |            |  go8 |             | go26 |      |     go12 |        go25 |     go11 |        go7 | \n" \
        " 10 | acc |         |         |           |      |       |       |             |            |      |        |     |              |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 11 | r23 |     r23 |         |       r23 |      |       |   r23 |         r23 |            |  s13 |        |     |          r23 |             |        |          |      |     |            |      |             |      |      |     go27 |             |     go11 |            | \n" \
        " 12 | r37 |     r37 |         |       r37 |      |       |   r37 |         r37 |            |  r37 |        |     |          r37 |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 13 |     |         |         |       s28 |   s5 |       |       |             |        s14 |  s13 |     s3 |  s4 |              |          s9 |     s6 |     go15 |  go1 | go2 |            |  go8 |             | go29 |      |     go12 |             |     go11 |        go7 | \n" \
        " 14 |     |         |         |       s31 |  s20 |       |       |         s33 |            |      |    s18 | s19 |              |             |    s21 |          | go30 |     |       go34 |      |        go32 |      |      |          |             |          |            | \n" \
        " 15 | r36 |     r36 |         |       r36 |      |       |   r36 |         r36 |            |  r36 |        |     |          r36 |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 16 |  r1 |         |         |           |      |       |       |             |            |      |        |     |              |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 17 |     |         |         |       s35 |   s5 |       |       |             |        s14 |  s13 |     s3 |  s4 |              |          s9 |     s6 |     go15 |  go1 | go2 |            |  go8 |             | go36 |      |     go12 |             |     go11 |        go7 | \n" \
        " 18 |     |         |         |           |      |    r4 |       |             |            |      |        |     |              |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 19 |     |         |         |           |      |    r3 |       |             |            |      |        |     |              |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 20 |     |         |         |           |      |    r2 |       |             |            |      |        |     |              |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 21 |     |         |         |           |      |    r5 |       |             |            |      |        |     |              |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 22 |  r9 |      r9 |         |        r9 |      |       |    r9 |          r9 |            |   r9 |        |     |           r9 |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 23 | r24 |     r24 |         |       r24 |      |       |   r24 |         r24 |            |  r24 |        |     |          r24 |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 24 |     |         |     s38 |           |      |       |       |             |            |      |        |     |          s37 |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 25 |     |         |         |           |      |       |       |             |            |      |        |     |          s39 |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 26 |     |     r30 |         |           |      |       |   s40 |             |            |      |        |     |          r30 |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 27 | r22 |     r22 |         |       r22 |      |       |   r22 |         r22 |            |  r22 |        |     |          r22 |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 28 |     |         |     s41 |           |      |       |       |             |            |      |        |     |              |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 29 | r20 |     r20 |         |       r20 |      |       |   r20 |         r20 |            |  r20 |        |     |          r20 |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 30 |     |         |         |           |      |   s42 |       |             |            |      |        |     |              |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 31 |     |         |     s43 |           |      |       |       |         s44 |            |      |        |     |              |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 32 |     |         |         |           |      |       |       |         s45 |            |      |        |     |              |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 33 | r11 |     r11 |         |       r11 |      |       |   r11 |         r11 |            |  r11 |        |     |          r11 |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 34 |     |     r17 |         |           |      |       |   s46 |         r17 |            |      |        |     |              |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 35 |  r8 |      r8 |     s47 |        r8 |   r8 |       |    r8 |          r8 |            |   r8 |     r8 |  r8 |           r8 |             |     r8 |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 36 |     |         |         |       s48 |      |       |       |             |            |      |        |     |              |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 37 | r26 |     r26 |         |       r26 |      |       |   r26 |         r26 |            |  r26 |        |     |          r26 |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 38 |     |         |         |           |   s5 |       |       |             |        s14 |  s13 |     s3 |  s4 |              |          s9 |     s6 |     go15 |  go1 | go2 |            |  go8 |             | go26 |      |     go12 |        go49 |     go11 |        go7 | \n" \
        " 39 | r25 |     r25 |         |       r25 |      |       |   r25 |         r25 |            |  r25 |        |     |          r25 |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 40 |     |         |         |       s50 |   s5 |       |       |             |        s14 |  s13 |     s3 |  s4 |              |          s9 |     s6 |     go15 |  go1 | go2 |            |  go8 |             | go26 |      |     go12 |        go51 |     go11 |        go7 | \n" \
        " 41 |     |         |         |           |   s5 |       |       |             |        s14 |  s13 |     s3 |  s4 |              |          s9 |     s6 |     go15 |  go1 | go2 |            |  go8 |             | go52 |      |     go12 |             |     go11 |        go7 | \n" \
        " 42 |     |     r16 |         |           |   s5 |       |   r16 |         r16 |        s14 |  s13 |     s3 |  s4 |              |          s9 |     s6 |     go15 |  go1 | go2 |            |  go8 |             | go53 |      |     go12 |             |     go11 |        go7 | \n" \
        " 43 |     |         |         |           |  s20 |       |       |             |            |      |    s18 | s19 |              |             |    s21 |          | go30 |     |       go34 |      |        go54 |      |      |          |             |          |            | \n" \
        " 44 | r12 |     r12 |         |       r12 |      |       |   r12 |         r12 |            |  r12 |        |     |          r12 |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 45 | r13 |     r13 |         |       r13 |      |       |   r13 |         r13 |            |  r13 |        |     |          r13 |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 46 |     |         |         |       s55 |  s20 |       |       |             |            |      |    s18 | s19 |              |             |    s21 |          | go30 |     |       go34 |      |        go56 |      |      |          |             |          |            | \n" \
        " 47 |     |         |         |           |   s5 |       |       |             |        s14 |  s13 |     s3 |  s4 |              |          s9 |     s6 |     go15 |  go1 | go2 |            |  go8 |             | go57 |      |     go12 |             |     go11 |        go7 | \n" \
        " 48 |  r6 |      r6 |         |        r6 |   r6 |       |    r6 |          r6 |            |   r6 |     r6 |  r6 |           r6 |             |     r6 |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 49 |     |     s58 |         |           |      |       |       |             |            |      |        |     |              |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 50 |     |         |         |           |   s5 |       |       |             |        s14 |  s13 |     s3 |  s4 |              |          s9 |     s6 |     go15 |  go1 | go2 |            |  go8 |             | go26 |      |     go12 |        go59 |     go11 |        go7 | \n" \
        " 51 |     |     r28 |         |           |      |       |       |             |            |      |        |     |          r28 |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 52 |     |         |         |       s60 |      |       |       |             |            |      |        |     |              |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 53 |     |     r15 |         |           |      |       |   r15 |         r15 |            |      |        |     |              |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 54 |     |     s61 |         |           |      |       |       |             |            |      |        |     |              |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 55 |     |         |         |           |  s20 |       |       |             |            |      |    s18 | s19 |              |             |    s21 |          | go30 |     |       go34 |      |        go62 |      |      |          |             |          |            | \n" \
        " 56 |     |     r18 |         |           |      |       |       |         r18 |            |      |        |     |              |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 57 |     |         |         |       s63 |      |       |       |             |            |      |        |     |              |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 58 |     |         |         |       s64 |      |       |       |             |            |      |        |     |              |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 59 |     |     r29 |         |           |      |       |       |             |            |      |        |     |          r29 |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 60 |     |     s65 |         |           |      |       |       |             |            |      |        |     |              |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 61 |     |         |         |           |      |       |       |         s66 |            |      |        |     |              |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 62 |     |     r19 |         |           |      |       |       |         r19 |            |      |        |     |              |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 63 |     |     s67 |         |           |      |       |       |             |            |      |        |     |              |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 64 |     |         |         |           |      |       |       |             |            |      |        |     |          s68 |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 65 | r21 |     r21 |         |       r21 |      |       |   r21 |         r21 |            |  r21 |        |     |          r21 |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 66 | r14 |     r14 |         |       r14 |      |       |   r14 |         r14 |            |  r14 |        |     |          r14 |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 67 |  r7 |      r7 |         |        r7 |   r7 |       |    r7 |          r7 |            |   r7 |     r7 |  r7 |           r7 |             |     r7 |          |      |     |            |      |             |      |      |          |             |          |            | \n" \
        " 68 | r27 |     r27 |         |       r27 |      |       |   r27 |         r27 |            |  r27 |        |     |          r27 |             |        |          |      |     |            |      |             |      |      |          |             |          |            | \n";
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
    auto paw = _loadPaw("../../paw/chance.shell");
#else
    auto paw = _loadPaw("../paw/chance.shell");
#endif
    assert(paw != null);

    cout << paw->root().toString() << endl;

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

int main () {
    _t_basic();
    _t_loadParsingTree();
	_t_load_map_paws();
	_t_load_boss_appear_snake();
	_t_load_green();
	_t_load_obj_shell();
	_t_load_dialogue();
	_t_load_chance();
    return 0;
}
